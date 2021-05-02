#include "Entity.h"
#include "RTETools.h"
#include "PresetMan.h"
#include "ConsoleMan.h"
#include "DataModule.h"

namespace RTE {

	Entity::ClassInfo Entity::m_sClass("Entity");
	Entity::ClassInfo * Entity::ClassInfo::s_ClassHead = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Entity::Clear() {
		m_PresetName = "None";
		m_IsOriginalPreset = false;
		m_DefinedInModule = -1;
		m_PresetDescription.clear();
		m_Groups.clear();
		m_LastGroupSearch.clear();
		m_LastGroupResult = false;
		m_RandomWeight = 100;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::Create() {
		m_Groups.push_back("All"); // Special "All" group that includes.. all
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::Create(const Entity &reference) {
		m_PresetName = reference.m_PresetName;
		// Note how m_IsOriginalPreset is NOT assigned, automatically indicating that the copy is not an original Preset!
		m_DefinedInModule = reference.m_DefinedInModule;
		m_PresetDescription = reference.m_PresetDescription;

		for (const std::string &group : reference.m_Groups) {
			m_Groups.push_back(group);
		}
		m_RandomWeight = reference.m_RandomWeight;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "CopyOf") {
			std::string refName = reader.ReadPropValue();
			std::string className = GetClassName();
			const Entity *preset = g_PresetMan.GetEntityPreset(className, refName, reader.GetReadModuleID());
			if (preset) {
				preset->Clone(this);
			} else {
				if (className == "AtomGroup" || className == "Attachable" || className == "AEmitter") {
					reader.ReportError("The PresetName to be copied was not found in data modules.");
				}
				// If we couldn't find the preset to copy from, read it as an original but report the problem in the console
				g_ConsoleMan.PrintString("ERROR: Couldn't find the preset '" + refName + "' accessed in " + reader.GetCurrentFilePath() + " at line " + reader.GetCurrentFileLine());
				// Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
				int slashPos = refName.find_first_of('/');
				m_PresetName = (slashPos != std::string::npos) ? refName.substr(slashPos + 1) : refName;
				// Mark this so that the derived class knows it should be added to the PresetMan when it's done reading all properties.
				m_IsOriginalPreset = true;
				// Indicate where this was read from
				m_DefinedInModule = reader.GetReadModuleID();
			}
		} else if (propName == "PresetName" || propName == "InstanceName") {
			SetPresetName(reader.ReadPropValue());
			// Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
			int slashPos = m_PresetName.find_first_of('/');
			if (slashPos != std::string::npos) { m_PresetName = m_PresetName.substr(slashPos + 1); }
			// Mark this so that the derived class knows it should be added to the PresetMan when it's done reading all properties.
			m_IsOriginalPreset = true;
			// Indicate where this was read from
			m_DefinedInModule = reader.GetReadModuleID();
		} else if (propName == "Description") {
			std::string descriptionValue = reader.ReadPropValue();
			if (descriptionValue == "MultiLineText") {
				m_PresetDescription.clear();
				while (reader.NextProperty() && reader.ReadPropName() == "AddLine") {
					m_PresetDescription += reader.ReadPropValue() + "\n\n";
				}
				if (!m_PresetDescription.empty()) {
					m_PresetDescription.resize(m_PresetDescription.size() - 2);
				}
			} else {
				m_PresetDescription = descriptionValue;
			}
		} else if (propName == "RandomWeight") {
			reader >> m_RandomWeight;
			m_RandomWeight = Limit(m_RandomWeight, 100, 0);
		} else if (propName == "AddToGroup") {
			std::string newGroup;
			reader >> newGroup;
			AddToGroup(newGroup);
			// Do this in AddToGroup instead?
			g_PresetMan.RegisterGroup(newGroup, reader.GetReadModuleID());
		} else {
			// Search for a property name match failed!
			// TODO: write this out to some log file
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::Save(Writer &writer) const {
		Serializable::Save(writer);

		// Is an original preset definition
		if (m_IsOriginalPreset) {
			writer.NewPropertyWithValue("PresetName", m_PresetName);
		// Only write out a copy reference if there is one
		} else if (!m_PresetName.empty() && m_PresetName != "None") {
			writer.NewPropertyWithValue("CopyOf", GetModuleAndPresetName());
		}
		if (!m_PresetDescription.empty()) { writer.NewPropertyWithValue("Description", m_PresetDescription); }

		// TODO: Make proper save system that knows not to save redundant data!
		/*
		for (list<string>::const_iterator itr = m_Groups.begin(); itr != m_Groups.end(); ++itr) {
			writer.NewPropertyWithValue("AddToGroup", *itr);
		}
		*/
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::SavePresetCopy(Writer &writer) const {
		// Can only save out copies with this
		if (m_IsOriginalPreset) {
			RTEAbort("Tried to save out a pure Preset Copy Reference from an original Preset!");
			return -1;
		}
		writer.ObjectStart(GetClassName());
		writer.NewPropertyWithValue("CopyOf", GetModuleAndPresetName());
		writer.ObjectEnd();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Entity::GetModuleAndPresetName() const {
		if (m_DefinedInModule < 0) {
			return GetPresetName();
		}
		const DataModule *dataModule = g_PresetMan.GetDataModule(m_DefinedInModule);

		if (!dataModule) {
			return GetPresetName();
		}
		return dataModule->GetFileName() + "/" + GetPresetName();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Entity::MigrateToModule(int whichModule) {
		if (m_DefinedInModule == whichModule) {
			return false;
		}
		m_IsOriginalPreset = true; // This now a unique snowflake
		m_DefinedInModule = whichModule;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Entity::IsInGroup(const std::string &whichGroup) {
		// Do quick check against last search try to see if we can answer without searching again
		if (!whichGroup.empty() && m_LastGroupSearch == whichGroup) {
			return m_LastGroupResult;
		}
		// Searched for Any or All yields ALL
		if (whichGroup == "Any" || whichGroup == "All") {
			return true;
		}
		// Searched for None returns false
		if (whichGroup == "None") {
			return false;
		}
		for (std::list<std::string>::const_iterator itr = m_Groups.begin(); itr != m_Groups.end(); ++itr) {
			if (whichGroup == *itr) {
				// Save the search result for quicker response next time
				m_LastGroupSearch = whichGroup;
				return m_LastGroupResult = true;
			}
		}
		// Save the search result for quicker response next time
		m_LastGroupSearch = whichGroup;
		return m_LastGroupResult = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Reader & operator>>(Reader &reader, Entity &operand) {
		// Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
		std::string objectFilePath = reader.GetCurrentFilePath();
		// Read the Entity from the file and try to add it to PresetMan
		operand.Create(reader);
		g_PresetMan.AddEntityPreset(&operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);

		return reader;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Reader & operator>>(Reader &reader, Entity *operand) {
		if (operand) {
			// Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
			std::string objectFilePath = reader.GetCurrentFilePath();
			// Read the Entity from the file and try to add it to PresetMan
			operand->Create(reader);
			g_PresetMan.AddEntityPreset(operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);
		} else {
			reader.ReportError("Tried to read an .ini file into a null Entity pointer!");
		}
		return reader;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity::ClassInfo::ClassInfo(const std::string &name, ClassInfo *parentInfo, MemoryAllocate allocFunc, MemoryDeallocate deallocFunc, Entity * (*newFunc)(), int allocBlockCount) :
		m_Name(name),
		m_ParentInfo(parentInfo),
		m_Allocate(allocFunc),
		m_Deallocate(deallocFunc),
		m_NewInstance(newFunc),
		m_NextClass(s_ClassHead) {
			s_ClassHead = this;

			m_AllocatedPool.clear();
			m_PoolAllocBlockCount = (allocBlockCount > 0) ? allocBlockCount : 10;
		}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::list<std::string> Entity::ClassInfo::GetClassNames() {
		std::list<std::string> retList;
		for (const ClassInfo *itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			retList.push_back(itr->GetName());
		}
		return retList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity::ClassInfo * Entity::ClassInfo::GetClass(const std::string &name) {
		if (name.empty() || name == "None") {
			return 0;
		}
		for (const ClassInfo *itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			if (itr->GetName() == name) {
				return itr;
			}
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Entity::ClassInfo::FillAllPools(int fillAmount) {
		for (ClassInfo *itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			if (itr->IsConcrete()) { itr->FillPool(fillAmount); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Entity::ClassInfo::FillPool(int fillAmount) {
		// Default to the set block allocation size if fillAmount is 0
		if (fillAmount <= 0) { fillAmount = m_PoolAllocBlockCount; }

		// If concrete class, fill up the pool with pre-allocated memory blocks the size of the type
		if (m_Allocate && fillAmount > 0) {
			for (int i = 0; i < fillAmount; ++i) {
				m_AllocatedPool.push_back(m_Allocate());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void * Entity::ClassInfo::GetPoolMemory() {
		RTEAssert(IsConcrete(), "Trying to get pool memory of an abstract Entity class!");

		// If the pool is empty, then fill it up again with as many instances as we are set to
		if (m_AllocatedPool.empty()) { FillPool((m_PoolAllocBlockCount > 0) ? m_PoolAllocBlockCount : 10); }

		// Get the instance in the top of the pool and pop it off
		void *foundMemory = m_AllocatedPool.back();
		m_AllocatedPool.pop_back();

		RTEAssert(foundMemory, "Could not find an available instance in the pool, even after increasing its size!");

		// Keep track of the number of instances passed out
		m_InstancesInUse++;

		return foundMemory;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Entity::ClassInfo::ReturnPoolMemory(void *returnedMemory) {
		if (!returnedMemory) {
			return 0;
		}
		m_AllocatedPool.push_back(returnedMemory);

		// Keep track of the number of instances passed in
		m_InstancesInUse--;

		return m_InstancesInUse;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Entity::ClassInfo::DumpPoolMemoryInfo(const Writer &fileWriter) {
		for (const ClassInfo *itr = s_ClassHead; itr != nullptr; itr = itr->m_NextClass) {
			if (itr->IsConcrete()) { fileWriter.NewLineString(itr->GetName() + ": " + std::to_string(itr->m_InstancesInUse), false); }
		}
	}
}