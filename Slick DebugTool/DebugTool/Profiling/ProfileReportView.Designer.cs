namespace DebugTool.Profiling
{
	partial class ProfileReportView
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.timeView1 = new DebugTool.Profiling.TimeView();
			this.SuspendLayout();
			// 
			// timeView1
			// 
			this.timeView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.timeView1.Location = new System.Drawing.Point(0, 0);
			this.timeView1.Name = "timeView1";
			this.timeView1.Size = new System.Drawing.Size(627, 394);
			this.timeView1.TabIndex = 0;
			// 
			// ProfileReportView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.timeView1);
			this.Name = "ProfileReportView";
			this.Size = new System.Drawing.Size(627, 394);
			this.ResumeLayout(false);

		}

		#endregion

		private TimeView timeView1;
	}
}
