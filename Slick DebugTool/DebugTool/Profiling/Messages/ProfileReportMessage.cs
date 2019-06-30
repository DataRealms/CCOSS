using System;
using System.Collections.Generic;
using System.Text;
using DebugTool.Network;
using DebugTool.Profiling.Sampling;

namespace DebugTool.Profiling
{
	class ProfileReportMessage : DebugMessage
	{
		ProfileReportMessage()
		{
			m_samples = new List<ProfileSample>();
		}

		public override void Serialize(System.IO.BinaryWriter writer)
		{
			throw new Exception("Serialization of ProfileReportMessage not supported");
		}

		public override void Deserialize(System.IO.BinaryReader reader)
		{
			int version = reader.ReadInt32();
			if (version == 1)
			{
				m_frequency = reader.ReadInt64();
			}


			// Read the string table:
			Dictionary<uint, string> stringtable = new Dictionary<uint, string>();
			int nrstring = reader.ReadInt32();

			for (int i = 0; i < nrstring; ++i)
			{
				uint id = reader.ReadUInt32();
				string name = reader.ReadString();
				stringtable[id] = name;
			}


			// Read the samples:
			int nrsamples = reader.ReadInt32();
			m_samples.Clear();

			for (int i = 0; i < nrsamples; ++i)
			{
				uint nameptr = reader.ReadUInt32();
				string name;

				if (!stringtable.TryGetValue( nameptr, out name ))
					throw new Exception("Unrecognized string found in the ProfileReportMessage samples");

				Int64 time = reader.ReadInt64();
				bool enter = reader.ReadBoolean();
				uint color = reader.ReadUInt32();

				m_samples.Add( new ProfileSample( name, time, enter, color ) );
			}
		}

		private long m_frequency;
		private List<ProfileSample> m_samples;

		public List<ProfileSample> Samples
		{
			get { return m_samples; }
		}
		public long Frequency
		{
			get { return m_frequency; }
		}

		static public uint s_id = 0x50524D45;	// 'PRME'

		public override uint Identifier
		{
			get { return s_id; }
		}

		public static DebugMessage Create()
		{
			return new ProfileReportMessage();
		}
	}
}
