#ifndef DEBUGMESSAGE_H
#define DEBUGMESSAGE_H



#include <DebugTool/DebugMessageFactory.h>
#include <BinaryWriterNormal.h>
class BinaryReaderNormal;



namespace Debug
{

	class DebugMessage
	{
	public:
		virtual void Serialize( BinaryWriterNormal &writer ) = 0;
		virtual void Deserialize( BinaryReaderNormal &reader ) = 0;

		virtual unsigned int GetType() const = 0;
	};



	template< typename T, unsigned int NT >
	class DebugMessageTemplate : public DebugMessage
	{
	public:
		// Automagically register this derived sim command with the factory.
		// Note that this still needs to be defined by derived classes in the .cpp.
		// You can use the REGISTER_SIMCOMMAND macro in the .cpp.

		typedef DebugMessageFactory<T, NT> Factory;
		static Factory s_factory;

		virtual unsigned int GetType() const
		{
			return NT;
		}
	};


	#define REGISTER_DEBUGMESSAGE( C, NT ) template<>  C::Factory	Debug::DebugMessageTemplate<C,NT>::s_factory;
}


#endif
