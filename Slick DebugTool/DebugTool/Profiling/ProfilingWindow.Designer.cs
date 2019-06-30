namespace DebugTool.Profiling
{
	partial class ProfilingWindow
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ProfilingWindow));
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonSingleSample = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.toolStripButtonSeries = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonClear = new System.Windows.Forms.ToolStripButton();
			this.profileReportView1 = new DebugTool.Profiling.ProfileReportView();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.listViewReports = new System.Windows.Forms.ListView();
			this.columnHeaderName = new System.Windows.Forms.ColumnHeader();
			this.columnHeaderDuration = new System.Windows.Forms.ColumnHeader();
			this.toolStrip1.SuspendLayout();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonSingleSample,
            this.toolStripSeparator1,
            this.toolStripButtonSeries,
            this.toolStripButtonClear});
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(842, 25);
			this.toolStrip1.TabIndex = 1;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButtonSingleSample
			// 
			this.toolStripButtonSingleSample.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonSingleSample.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSingleSample.Image")));
			this.toolStripButtonSingleSample.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonSingleSample.Name = "toolStripButtonSingleSample";
			this.toolStripButtonSingleSample.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonSingleSample.Text = "Profile one frame";
			this.toolStripButtonSingleSample.Click += new System.EventHandler(this.toolStripButtonSingleSample_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
			// 
			// toolStripButtonSeries
			// 
			this.toolStripButtonSeries.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonSeries.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSeries.Image")));
			this.toolStripButtonSeries.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonSeries.Name = "toolStripButtonSeries";
			this.toolStripButtonSeries.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonSeries.Text = "toolStripButton1";
			this.toolStripButtonSeries.ToolTipText = "Record a series of profilereports";
			this.toolStripButtonSeries.Click += new System.EventHandler(this.toolStripButtonSeries_Click);
			// 
			// toolStripButtonClear
			// 
			this.toolStripButtonClear.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.toolStripButtonClear.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonClear.Image")));
			this.toolStripButtonClear.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonClear.Name = "toolStripButtonClear";
			this.toolStripButtonClear.Size = new System.Drawing.Size(23, 22);
			this.toolStripButtonClear.Text = "Clear all reports";
			this.toolStripButtonClear.Click += new System.EventHandler(this.toolStripButtonClear_Click);
			// 
			// profileReportView1
			// 
			this.profileReportView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.profileReportView1.Location = new System.Drawing.Point(0, 0);
			this.profileReportView1.Name = "profileReportView1";
			this.profileReportView1.Size = new System.Drawing.Size(670, 498);
			this.profileReportView1.TabIndex = 0;
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.splitContainer1.Location = new System.Drawing.Point(0, 25);
			this.splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.listViewReports);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.profileReportView1);
			this.splitContainer1.Size = new System.Drawing.Size(842, 498);
			this.splitContainer1.SplitterDistance = 168;
			this.splitContainer1.TabIndex = 2;
			// 
			// listViewReports
			// 
			this.listViewReports.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderName,
            this.columnHeaderDuration});
			this.listViewReports.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listViewReports.HideSelection = false;
			this.listViewReports.Location = new System.Drawing.Point(0, 0);
			this.listViewReports.MultiSelect = false;
			this.listViewReports.Name = "listViewReports";
			this.listViewReports.Size = new System.Drawing.Size(168, 498);
			this.listViewReports.TabIndex = 0;
			this.listViewReports.UseCompatibleStateImageBehavior = false;
			this.listViewReports.View = System.Windows.Forms.View.Details;
			this.listViewReports.SelectedIndexChanged += new System.EventHandler(this.listViewReports_SelectedIndexChanged);
			// 
			// columnHeaderName
			// 
			this.columnHeaderName.Text = "Name";
			this.columnHeaderName.Width = 83;
			// 
			// columnHeaderDuration
			// 
			this.columnHeaderDuration.Text = "Duration (ms)";
			this.columnHeaderDuration.Width = 80;
			// 
			// ProfilingWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(842, 523);
			this.Controls.Add(this.splitContainer1);
			this.Controls.Add(this.toolStrip1);
			this.Name = "ProfilingWindow";
			this.ShowIcon = false;
			this.Text = "Profiling";
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripButton toolStripButtonSingleSample;
		private ProfileReportView profileReportView1;
		private System.Windows.Forms.ToolStripButton toolStripButtonSeries;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.ListView listViewReports;
		private System.Windows.Forms.ColumnHeader columnHeaderName;
		private System.Windows.Forms.ColumnHeader columnHeaderDuration;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripButton toolStripButtonClear;
	}
}