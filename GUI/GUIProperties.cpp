#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIProperties::GUIProperties(const std::string &Name) {
	m_Name = Name;
	m_VariableList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIProperties::GUIProperties() {
	m_Name = "";
	m_VariableList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIProperties::~GUIProperties() {
	Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::Clear() {
	// Free the list
	std::vector <PropVariable *>::iterator it;

	for (it = m_VariableList.begin(); it != m_VariableList.end(); it++) {
		PropVariable *p = *it;

		// Free the property
		delete p;
	}
	m_VariableList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::AddVariable(const std::string &Variable, const std::string &Value) {
	// If this property already exists, just update it
	std::string Val;
	if (GetValue(Variable, &Val)) {
		SetValue(Variable, Value);
		return;
	}

	PropVariable *Prop = new PropVariable;

	Prop->m_Name = Variable;
	Prop->m_Value = Value;

	m_VariableList.push_back(Prop);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::AddVariable(const std::string &Variable, char *Value) {
	std::string Val = Value;

	AddVariable(Variable, Val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::AddVariable(const std::string &Variable, int Value) {
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%i", Value);
	std::string Val(buf);
	AddVariable(Variable, Val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::AddVariable(const std::string &Variable, bool Value) {
	std::string Val = Value ? "True" : "False";
	AddVariable(Variable, Val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::SetValue(const std::string &Variable, const std::string &Value) {
	// Find the property
	std::vector <PropVariable *>::iterator it;

	for (it = m_VariableList.begin(); it != m_VariableList.end(); it++) {
		PropVariable *p = *it;

		// Matching name?
		if (stricmp(p->m_Name.c_str(), Variable.c_str()) == 0) {
			p->m_Value = Value;
			return true;
		}
	}

	// Not set
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::SetValue(const std::string &Variable, int Value) {
	char buf[64];
	std::snprintf(buf, sizeof(buf), "%i", Value);

	return SetValue(Variable, buf);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::Update(GUIProperties *Props, bool Add) {
	assert(Props);

	std::vector <PropVariable *>::iterator it1;

	for (it1 = Props->m_VariableList.begin(); it1 != Props->m_VariableList.end(); it1++) {
		const PropVariable *Src = *it1;

		// Set the variable
		if (!SetValue(Src->m_Name, Src->m_Value) && Add) { AddVariable(Src->m_Name, Src->m_Value); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::GetValue(const std::string &Variable, std::string *Value) {
	// Find the property
	std::vector <PropVariable *>::iterator it;

	for (it = m_VariableList.begin(); it != m_VariableList.end(); it++) {
		const PropVariable *p = *it;

		// Matching name?
		if (stricmp(p->m_Name.c_str(), Variable.c_str()) == 0) {
			*Value = p->m_Value;
			return true;
		}
	}

	// Not found
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIProperties::GetValue(const std::string &Variable, std::string *Array, int MaxArraySize) {
	assert(Array);

	std::string Value;

	// Get the string
	if (!GetValue(Variable, &Value)) {
		return 0;
	}
	// Create a c string version of the value for tokenizing
	char *str = new char[Value.length() + 1];
	if (!str) {
		return 0;
	}
	// Tokenize the string
	strcpy(str, Value.c_str());
	char *tok = strtok(str, ",");
	int count = 0;

	while (tok && count < MaxArraySize) {
		Array[count++] = GUIUtil::TrimString(tok);
		tok = strtok(0, ",");
	}

	delete[] str;

	// Return the number of elements read
	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIProperties::GetValue(const std::string &Variable, int *Array, int MaxArraySize) {
	assert(Array);

	std::string Value;

	// Get the string
	if (!GetValue(Variable, &Value)) {
		return 0;
	}
	// Create a c string version of the value for tokenizing
	char *str = new char[Value.length() + 1];
	if (!str) {
		return 0;
	}
	// Tokenize the string
	strcpy(str, Value.c_str());
	char *tok = strtok(str, ",");
	int count = 0;

	while (tok && count < MaxArraySize) {
		Array[count++] = atoi(tok);
		tok = strtok(0, ",");
	}

	delete[] str;

	// Return the number of elements read
	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::GetValue(const std::string &Variable, int *Value) {
	assert(Value);

	std::string val;

	// Get the string
	if (!GetValue(Variable, &val)) {
		return false;
	}
	*Value = atoi(val.c_str());

	// Found the value
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::GetValue(const std::string &Variable, unsigned long *Value) {
	assert(Value);

	std::string val;

	// Get the string
	if (!GetValue(Variable, &val)) {
		return false;
	}
	*Value = atol(val.c_str());

	// Found the value
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::GetValue(const std::string &Variable, bool *Value) {
	assert(Value);

	std::string val;

	// Get the string
	if (!GetValue(Variable, &val)) {
		return false;
	}
	// Default is false
	*Value = false;

	// Convert the string into a boolean
	if (stricmp(val.c_str(), "true") == 0) { *Value = true; }

	// Found the value
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIProperties::GetName() const {
	return m_Name;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIProperties::ToString() {
	std::string OutString = "";

	// Go through each value
	std::vector <PropVariable *>::iterator it;
	for (it = m_VariableList.begin(); it != m_VariableList.end(); it++) {
		const PropVariable *V = *it;

		OutString += V->m_Name;
		OutString.append(" = ");
		OutString += V->m_Value;
		OutString.append("\n");
	}
	return OutString;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIProperties::GetCount() const {
	return m_VariableList.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::GetVariable(int Index, std::string *Name, std::string *Value) {
	// Check for a bad index
	if (Index < 0 || Index >= m_VariableList.size()) {
		return false;
	}

	const PropVariable *P = (PropVariable *)m_VariableList.at(Index);
	if (Name) { *Name = P->m_Name; }
	if (Value) { *Value = P->m_Value; }

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIProperties::SetVariable(int Index, const std::string &Name, const std::string &Value) {
	// Check for a bad index
	if (Index < 0 || Index >= m_VariableList.size()) {
		return false;
	}
	PropVariable *P = m_VariableList.at(Index);
	P->m_Name = Name;
	P->m_Value = Value;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIProperties::Sort(bool Ascending) {
	// Simple bubble sort

	for (int i = 0; i < m_VariableList.size(); i++) {

		for (int j = 0; j < m_VariableList.size() - 1 - i; j++) {

			PropVariable *V = m_VariableList.at(j);
			PropVariable *V2 = m_VariableList.at(j + 1);

			if ((V->m_Name.compare(V2->m_Name) > 0 && Ascending) || (V->m_Name.compare(V2->m_Name) < 0 && !Ascending)) {
				// Swap em
				PropVariable temp = *V;
				*V = *V2;
				*V2 = temp;
			}
		}
	}
}
