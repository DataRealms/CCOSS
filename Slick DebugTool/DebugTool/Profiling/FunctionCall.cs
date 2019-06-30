using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace DebugTool.Profiling
{
	public class FunctionCall
	{
		public FunctionCall(string name, long startTime, uint color)
		{
			m_name = name;
			m_startTime = startTime;
			m_endTime = startTime;
			m_color = Color.FromArgb((int)color);
			m_children = new List<FunctionCall>();
		}

		private string				m_name;
		private long				m_startTime;
		private long				m_endTime;
		private Color				m_color;
		private List<FunctionCall>	m_children;

		public void AddChild( FunctionCall func )
		{
			m_children.Add(func);
		}

		public string Name
		{
			get { return m_name; }
		}
		public long Duration
		{
			get { return m_endTime - m_startTime; }
		}
		public long StartTime
		{
			get { return m_startTime; }
			set { m_startTime = value; }
		}
		public long EndTime
		{
			get { return m_endTime; }
			set { m_endTime = value; }
		}
		public Color Color
		{
			get { return m_color; }
			set { m_color = value; }
		}
		public List<FunctionCall> Children
		{
			get { return m_children; }
		}

		public override string ToString()
		{
			return m_name;
		}
	}
}
