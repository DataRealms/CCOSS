#pragma once



namespace Debug
{
	class DebugMessage;

	/////////////////////////////////////////////////////////////////////////////////
	// Base factory 

	class DebugMessageFactoryBase
	{
	public:
		virtual ~DebugMessageFactoryBase() {}

		virtual DebugMessage* Create() = 0;
		virtual DebugMessage* Clone( const DebugMessage* ) = 0;
	};
	
	/////////////////////////////////////////////////////////////////////////////////
	// The actual class which handles the factory registration and creation of NetObjects.
	class DebugMessageCreator
	{
	public:
		static DebugMessageCreator& Instance();

	public:
		void RegisterFactory( DebugMessageFactoryBase* pFactory, unsigned int type )
		{
			// If you hit this assert, then this factory was registered more than once for the same
			// 'type'.  Copy-paste error?
			SlickAssert( m_factoryMap[type] == NULL );

			m_factoryMap[type] = pFactory;
		}

		DebugMessage* Create(unsigned int type )
		{
			// If you hit this assert, then your class factory was not registered.  Make sure it
			// has a static factory member which is both declared and defined in some .cpp.
			SlickAssert( m_factoryMap[type] != NULL );
	
			return m_factoryMap[type]->Create();
		}

		DebugMessage* Clone(unsigned int type, const DebugMessage* pCmd )
		{
			return m_factoryMap[type]->Clone( pCmd );
		}

		typedef std::map<unsigned int, DebugMessageFactoryBase*> FactoryMap;
		FactoryMap m_factoryMap;
	};


	/////////////////////////////////////////////////////////////////////////////////
	// Parametrized factory for a specific SimCommand.

	template< typename T, int NT >
	class DebugMessageFactory : public DebugMessageFactoryBase
	{
	public:
		DebugMessageFactory()
		{
			DebugMessageCreator::Instance().RegisterFactory( this, NT );
		}

		virtual DebugMessage* Create()
		{
			return new T();
		}

		virtual DebugMessage* Clone( const DebugMessage* pCmd )
		{
			return new T(static_cast<const T&>(*pCmd));
		}
	};




}

