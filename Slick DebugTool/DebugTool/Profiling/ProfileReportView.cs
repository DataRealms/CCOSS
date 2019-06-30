using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace DebugTool.Profiling
{
	public partial class ProfileReportView : UserControl
	{
		public ProfileReportView()
		{
			InitializeComponent();
		}

		public ProfileReport Report
		{
			set { this.timeView1.Report = value; }
		}
	}
}
