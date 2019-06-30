using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace DebugTool.Network
{
	class DebugMessageFactory
	{
		public delegate DebugMessage CreateDebugMessageFunc();

		static public void RegisterMessageType(uint type, CreateDebugMessageFunc func)
		{
			if (m_creators.ContainsKey(type))
				return;	// already registered

			Debug.Assert( func != null );

			m_creators.Add(type, func);
		}

		static public DebugMessage CreateDebugMessage( uint type )
		{
			CreateDebugMessageFunc func;
			if (m_creators.TryGetValue( type, out func ))
			{
				return func();
			}

			return null;
		}

		static private Dictionary<uint, CreateDebugMessageFunc> m_creators = new Dictionary<uint,CreateDebugMessageFunc>();
	}
}
