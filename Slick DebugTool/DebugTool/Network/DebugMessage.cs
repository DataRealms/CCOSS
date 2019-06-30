using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace DebugTool.Network
{
	public abstract class DebugMessage
	{
		public abstract void Serialize( BinaryWriter writer );
		public abstract void Deserialize( BinaryReader reader );

		public abstract uint Identifier
		{
			get;
		}
	}
}
