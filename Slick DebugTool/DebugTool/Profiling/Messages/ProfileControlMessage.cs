using System;
using System.Collections.Generic;
using System.Text;
using DebugTool.Network;
using DebugTool.Profiling.Sampling;

namespace DebugTool.Profiling
{
	class ProfileControlMessage : DebugMessage
	{
		ProfileControlMessage()
		{
		}

		public ProfileControlMessage(bool paused, int nrFramesToProfile)
		{
			m_paused = paused;
			m_numberOfFramesToProfile = nrFramesToProfile;
		}

		public override void Serialize(System.IO.BinaryWriter writer)
		{
			writer.Write(m_paused);
			writer.Write(m_numberOfFramesToProfile);
		}

		public override void Deserialize(System.IO.BinaryReader reader)
		{
			throw new Exception("Deserialization of ProfileControlMessage not supported");
		}

		private bool m_paused;
		private int	m_numberOfFramesToProfile;


		static public uint s_id = 0x50434D45;	// 'PCME'

		public override uint Identifier
		{
			get { return s_id; }
		}

		public static DebugMessage Create()
		{
			return new ProfileControlMessage();
		}
	}
}
