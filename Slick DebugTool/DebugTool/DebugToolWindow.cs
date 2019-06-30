using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using DebugTool.Network;
using DebugTool.Profiling;

namespace DebugTool
{
	public partial class DebugToolWindow : Form
	{
		public DebugToolWindow()
		{
			InitializeComponent();

			toolStripComboBoxConnectTo.Items.Add("127.0.0.1");
			toolStripComboBoxConnectTo.Items.Add("192.168.1.124");
			toolStripComboBoxConnectTo.SelectedItem = "192.168.1.124";

			m_networkConnection = new NetworkConnection();
		}

		private NetworkConnection m_networkConnection;

		private void timerNetworkUpdate_Tick(object sender, EventArgs e)
		{
			// Dispatch network messages as they come in:
			m_networkConnection.DispatchMessages();

			// Set the status bar text so it displays who we're connected to:
			toolStripComboBoxConnectTo.Enabled = !m_networkConnection.Connected;
			toolStripButtonConnect.Enabled = !m_networkConnection.Connected;
			toolStripButtonDisconnect.Enabled = m_networkConnection.Connected;


			if (m_networkConnection.Connected)
			{
				toolStripStatusLabel.Text = String.Format("Connected to: {0}", m_networkConnection.ConnectedTo);
			}
			else
			{
				toolStripStatusLabel.Text = String.Format("Disconnected");
			}
		}

		private void Connect( string ip )
		{
			Disconnect();

			this.Cursor = Cursors.WaitCursor;
			try
			{
				m_networkConnection.Connect(ip, 23481);
			}
			catch( Exception ex )
			{
				MessageBox.Show( ex.Message, "Error connecting" );
			}
			this.Cursor = Cursors.Default;
		}

		private bool Disconnect()
		{
			bool result = true;

			this.Cursor = Cursors.WaitCursor;
			if (m_networkConnection.Connected)
			{
				if (MessageBox.Show("You are still connected to an application. Are you sure you wish to quit?","Are you sure?", MessageBoxButtons.YesNo) == DialogResult.Yes)
				{
					m_networkConnection.Disconnect();
					result = true;
				}
				else
				{
					result = false;
				}
			}
			this.Cursor = Cursors.Default;

			return result;
		}

		private void toolStripButtonConnect_Click(object sender, EventArgs e)
		{
			Connect(toolStripComboBoxConnectTo.Text);
		}

		private void toolStripButtonDisconnect_Click(object sender, EventArgs e)
		{
			Disconnect();
		}

		private void DebugTool_FormClosing(object sender, FormClosingEventArgs e)
		{
			e.Cancel = !Disconnect();
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			this.Close();
		}


		private void toolStripButtonProfiling_Click(object sender, EventArgs e)
		{
			ProfilingWindow outp = new ProfilingWindow(m_networkConnection);
			outp.MdiParent = this;
			outp.Show();
		}

	}
}