using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using DebugTool.Network;
using DebugTool.Profiling.Sampling;

namespace DebugTool.Profiling
{
	public partial class ProfilingWindow : Form
	{
		public ProfilingWindow( NetworkConnection connection )
		{
			InitializeComponent();

			m_connection = connection;

			DebugMessageFactory.RegisterMessageType(ProfileReportMessage.s_id, ProfileReportMessage.Create);
			m_connection.AddMessageHandler(ProfileReportMessage.s_id, HandleProfileMessage);
			
			m_reports = new List<ProfileReport>();
		}

		public void HandleProfileMessage( DebugMessage msg )
		{
			ProfileReportMessage m = msg as ProfileReportMessage;
			if (m != null)
			{
				ProfileReport report = ProfileReport.GenerateFromSamples(m.Frequency, m.Samples);
				if (report != null)
				{
					m_reports.Add(report);
					UpdateListView();
				}
			}
		}

		private void UpdateListView()
		{
			this.listViewReports.BeginUpdate();
			object selection = (listViewReports.SelectedItems.Count > 0) ? listViewReports.SelectedItems[0].Tag : null;
			this.listViewReports.Items.Clear();

			int count = 0;
			foreach (ProfileReport report in m_reports)
			{
				ListViewItem lvi = new ListViewItem( String.Format("Report{0}", count) );
				lvi.Tag = report;
				lvi.Selected = (report == selection);

				System.Windows.Forms.ListViewItem.ListViewSubItem child = new ListViewItem.ListViewSubItem(lvi, String.Format("{0:N0}", (int)((double)(report.MaximumTime-report.MinimumTime) * 1000.0 / (double)report.Frequency)));
				lvi.SubItems.Add(child);

				this.listViewReports.Items.Add( lvi );
				count++;
			}

			this.listViewReports.EndUpdate();
		}

		List<ProfileReport> m_reports;
		NetworkConnection m_connection;
		
		private void toolStripButtonSingleSample_Click(object sender, EventArgs e)
		{
			m_connection.SendMessage(new ProfileControlMessage(false, 1));			
		}

		private void toolStripButtonSeries_Click(object sender, EventArgs e)
		{
			m_connection.SendMessage(new ProfileControlMessage(false, 200));
		}

		private void listViewReports_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (this.listViewReports.SelectedItems.Count > 0)
			{
				this.profileReportView1.Report = (ProfileReport)this.listViewReports.SelectedItems[0].Tag;
			}
			else
			{
				this.profileReportView1.Report = null;
			}
		}

		private void toolStripButtonClear_Click(object sender, EventArgs e)
		{
			this.listViewReports.BeginUpdate();
			this.listViewReports.Items.Clear();
			this.listViewReports.EndUpdate();

			this.profileReportView1.Report = null;

			m_reports.Clear();
		}
	}
}