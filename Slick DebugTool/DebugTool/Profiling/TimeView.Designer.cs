namespace DebugTool.Profiling
{
	partial class TimeView
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
			this.components = new System.ComponentModel.Container();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.SuspendLayout();
			// 
			// TimeView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Name = "TimeView";
			this.Size = new System.Drawing.Size(608, 374);
			this.toolTip1.SetToolTip(this, "Lala");
			this.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.TimeView_MouseWheel);
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.TimeView_MouseDown);
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.TimeView_MouseMove);
			this.Paint += new System.Windows.Forms.PaintEventHandler(this.TimeView_Paint);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.TimeView_MouseUp);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ToolTip toolTip1;
	}
}
