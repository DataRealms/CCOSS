using System;
using System.Collections.Generic;
using System.Text;

namespace DebugTool.Profiling.Sampling
{
	public struct ProfileSample
	{
		public ProfileSample( string name, long time, bool enter, uint color )
		{
			m_name = name;
			m_time = time;
			m_enter = enter;
			m_color = color;
		}

		private string	m_name;
		private long	m_time;
		private bool	m_enter;
		private uint	m_color;

		public string Name
		{
			get { return m_name; }
		}
		public long Time
		{
			get { return m_time; }
		}
		public bool Enter
		{
			get { return m_enter; }
		}
		public uint Color
		{
			get { return m_color; }
		}
	}
}