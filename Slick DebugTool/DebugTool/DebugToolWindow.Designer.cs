namespace DebugTool
{
	partial class DebugToolWindow
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DebugToolWindow));
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripComboBoxConnectTo = new System.Windows.Forms.ToolStripComboBox();
			this.toolStripButtonConnect = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonDisconnect = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.toolStripButtonProfiling = new System.Windows.Forms.ToolStripButton();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.timerNetworkUpdate = new System.Windows.Forms.Timer(this.components);
			this.menuStrip1.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.toolStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(984, 24);
			this.menuStrip1.TabIndex = 1;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
			this.fileToolStripMenuItem.Text = "&File";
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
			this.exitToolStripMenuItem.Text = "E&xit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel});
			this.statusStrip1.Location = new System.Drawing.Point(0, 592);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(984, 22);
			this.statusStrip1.TabIndex = 2;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel
			// 
			this.toolStripStatusLabel.Name = "toolStripStatusLabel";
			this.toolStripStatusLabel.Size = new System.Drawing.Size(50, 17);
			this.toolStripStatusLabel.Text = "Ready...";
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripComboBoxConnectTo,
            this.toolStripButtonConnect,
            this.toolStripButtonDisconnect,
            this.toolStripSeparator1,
            this.toolStripButtonProfiling});
			this.toolStrip1.Location = new System.Drawing.Point(0, 24);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(984, 25);
			this.toolStrip1.TabIndex = 3;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripComboBoxConnectTo
			// 
			this.toolStripComboBoxConnectTo.Name = "toolStripComboBoxConnectTo";
			this.toolStripComboBoxConnectTo.Size = new System.Drawing.Size(121, 25);
			// 
			// toolStripButtonConnect
			// 
			this.toolStripButtonConnect.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonConnect.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonConnect.Image")));
			this.toolStripButtonConnect.ImageTransparentColor = System.Drawing.Color.White;
			this.toolStripButtonConnect.Name = "toolStripButtonConnect";
			this.toolStripButtonConnect.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonConnect.Text = "toolStripButton1";
			this.toolStripButtonConnect.ToolTipText = "Connect to an application";
			this.toolStripButtonConnect.Click += new System.EventHandler(this.toolStripButtonConnect_Click);
			// 
			// toolStripButtonDisconnect
			// 
			this.toolStripButtonDisconnect.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonDisconnect.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonDisconnect.Image")));
			this.toolStripButtonDisconnect.ImageTransparentColor = System.Drawing.Color.White;
			this.toolStripButtonDisconnect.Name = "toolStripButtonDisconnect";
			this.toolStripButtonDisconnect.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonDisconnect.Text = "toolStripButton2";
			this.toolStripButtonDisconnect.ToolTipText = "Disconnect from the currently connected application";
			this.toolStripButtonDisconnect.Click += new System.EventHandler(this.toolStripButtonDisconnect_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
			// 
			// toolStripButtonProfiling
			// 
			this.toolStripButtonProfiling.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonProfiling.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonProfiling.Image")));
			this.toolStripButtonProfiling.ImageTransparentColor = System.Drawing.Color.White;
			this.toolStripButtonProfiling.Name = "toolStripButtonProfiling";
			this.toolStripButtonProfiling.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonProfiling.ToolTipText = "Profiling";
			this.toolStripButtonProfiling.Click += new System.EventHandler(this.toolStripButtonProfiling_Click);
			// 
			// timerNetworkUpdate
			// 
			this.timerNetworkUpdate.Enabled = true;
			this.timerNetworkUpdate.Interval = 30;
			this.timerNetworkUpdate.Tick += new System.EventHandler(this.timerNetworkUpdate_Tick);
			// 
			// DebugToolWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(984, 614);
			this.Controls.Add(this.toolStrip1);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.menuStrip1);
			this.IsMdiContainer = true;
			this.MainMenuStrip = this.menuStrip1;
			this.Name = "DebugToolWindow";
			this.Text = "Debug Tool";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DebugTool_FormClosing);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripButton toolStripButtonConnect;
		private System.Windows.Forms.ToolStripButton toolStripButtonDisconnect;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
		private System.Windows.Forms.ToolStripComboBox toolStripComboBoxConnectTo;
		private System.Windows.Forms.Timer timerNetworkUpdate;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
		private System.Windows.Forms.ToolStripButton toolStripButtonProfiling;
	}
}

