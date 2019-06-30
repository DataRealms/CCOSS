using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace DebugTool.Profiling
{
	public partial class TimeView : UserControl
	{
		private readonly int functionCallOffset = 24;
		private readonly int functionCallHeight = 26;
		private readonly int functionCallStride = 28;
		private readonly Font functionCallFont = new Font( "Arial", 9 );
		private readonly Brush functionCallFontColor = new SolidBrush(Color.Black);
		private readonly Pen functionCallDivider = new Pen(Color.FromArgb(40, 0, 0, 0));

		public TimeView()
		{
			InitializeComponent();

			m_yOffset = 0;
			m_maxY = 0;
			m_panning = false;

			this.SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.ResizeRedraw, true);
		}

		private void DrawTimeLine( Graphics g )
		{
			if (m_report != null)
			{
				int left = ConvertTime(m_report.MinimumTime) - 10;
				int right = ConvertTime(m_report.MaximumTime) + 10;

				if (left > 0)
				{
					Rectangle r = new Rectangle(0, 0, left, this.Height);
					g.FillRectangle(Brushes.Gray, r);
				}
				if (right < this.Width)
				{
					Rectangle r = new Rectangle(right, 0, this.Width - right, this.Height);
					g.FillRectangle(Brushes.Gray, r);
				}
			}				
		}

		private bool DrawFunctionBlock( Graphics g, FunctionCall f, int y )
		{
			int left = ConvertTime( f.StartTime );
			int right = ConvertTime( f.EndTime );

			if ((right>0) && (left<this.Width) && (y<this.Height))
			{
				Rectangle r = new Rectangle( left, y, right-left, functionCallHeight );
				if (r.Width >= 2)
				{
					using( Brush b = new SolidBrush( f.Color ))
					{
						g.FillRectangle( b, r );

						g.DrawLine(functionCallDivider, new Point(left, y), new Point(left, y + functionCallHeight - 1));
						g.DrawLine(functionCallDivider, new Point(right-1, y), new Point(right-1, y + functionCallHeight - 1));
					}

					// Draw the text only if there is enough space:
					if (r.Width > 20)
					{
						SizeF size = g.MeasureString( f.Name, functionCallFont );
						if (size.Width < r.Width)
						{
							float xx = ((r.Width - size.Width) * 0.5f) + left;
							float yy = ((r.Height - size.Height) * 0.5f) + y;
							g.DrawString( f.Name, functionCallFont, functionCallFontColor, new PointF(xx, yy) );
						}
						else
						{
							string str = "...";
							size = g.MeasureString( str, functionCallFont );
							if (size.Width < r.Width)
							{
								float xx = ((r.Width - size.Width) * 0.5f) + left;
								float yy = ((r.Height - size.Height) * 0.5f) + y;
								g.DrawString( str, functionCallFont, functionCallFontColor, new PointF(xx, yy) );
							}
						}
					}
				}
				else
				{
					using( Pen p = new Pen( f.Color ) )
					{
						g.DrawLine( p, new Point(left, y), new Point(left, y + functionCallHeight-1) );
					}
				}
				return true;
			}
			return false;
		}
		private void RecDrawFunctionCalls( Graphics g, FunctionCall f, int y )
		{
			m_maxY = Math.Max(m_maxY, y+functionCallStride);
			DrawFunctionBlock( g, f, y );

			foreach( FunctionCall child in f.Children )
			{
				RecDrawFunctionCalls( g, child, y+functionCallStride );
			}
		}
		private void TimeView_Paint(object sender, PaintEventArgs e)
		{
			m_maxY = 0;
			Graphics g = e.Graphics;
			g.Clear( Color.FromArgb(240,240,240) );
			
			DrawTimeLine(g);

			if (m_report!=null)
			{
				foreach( FunctionCall f in m_report.TopFunctions )
				{
					RecDrawFunctionCalls(g, f, functionCallOffset - m_yOffset );
				}
			}
		}

		private void TimeView_MouseWheel(object sender, MouseEventArgs e)
		{
			if (m_report != null)
			{
				double diff = (double)(m_rightTime - m_leftTime);

				long d = (long)((e.Delta * 0.001f) * diff);
				long newleft = m_leftTime + d;
				long newright = m_rightTime - d;

				long diff2 = newright - newleft;
				long totaltime = m_report.MaximumTime - m_report.MinimumTime;
				if (diff2 > totaltime)
				{
					long correction = (diff2 - totaltime) / 2;
					newleft += correction;
					newright -= correction;
				}

				long middle = (newleft + newright) / 2;
				if ((middle >= m_report.MinimumTime) && (middle <= m_report.MaximumTime))
				{
					m_leftTime = newleft;
					m_rightTime = newright;
					Invalidate();
				}
			}
		}
		private void TimeView_MouseDown(object sender, MouseEventArgs e)
		{
			if ((e.Button == MouseButtons.Right) || (e.Button == MouseButtons.Middle))
			{
				m_panning = true;
				m_prevMousePosition = new Point(e.X, e.Y);
				Invalidate();
			}
		}
		private void TimeView_MouseMove(object sender, MouseEventArgs e)
		{
			Point p = new Point(e.X, e.Y);
			if (m_panning)
			{
				int deltax = p.X - m_prevMousePosition.X;
				long timeadd = ConvertScreenDelta(deltax);

				long newleft = m_leftTime - timeadd;
				long newright = m_rightTime - timeadd;

				long middle = (newleft + newright) / 2;
				if (m_report!=null)
				{
					if ((middle >= m_report.MinimumTime) && (middle <= m_report.MaximumTime))
					{
						m_leftTime = newleft;
						m_rightTime = newright;
					}

					int deltay = p.Y - m_prevMousePosition.Y;
					m_yOffset -= deltay;

					if (m_yOffset > m_maxY)
						m_yOffset = m_maxY;
					if (m_yOffset < 0)
						m_yOffset = 0;
				}

				Invalidate();
			}
			else
			{
				FunctionCall f = GetFunctionCallUnderMouse(e.X, e.Y);
				if (f != null)
				{
					double duration = ((double)f.Duration * 1000.0) / (double)m_report.Frequency;
					this.toolTip1.SetToolTip(this, String.Format("{0} - {1:N}", f.ToString(), duration));
				}
				else
				{
					this.toolTip1.SetToolTip(this, "");
				}
			}

			m_prevMousePosition = p;
		}
		private void TimeView_MouseUp(object sender, MouseEventArgs e)
		{
			if ((e.Button == MouseButtons.Right) || (e.Button == MouseButtons.Middle))
			{
				m_panning = false;
				Invalidate();
			}
		}

		private FunctionCall RecGetFunctionCallUnderMouse( FunctionCall f, int y, int mx, int my )
		{
			int left = ConvertTime( f.StartTime );
			int right = ConvertTime( f.EndTime );
			if ((right < mx) || (left > mx))
				return null;

			if (my >= y)
			{
				if (my <= y + functionCallHeight)
				{
					return f;
				}
				else
				{
					foreach( FunctionCall child in f.Children )
					{
						FunctionCall result = RecGetFunctionCallUnderMouse(child, y+functionCallStride, mx, my );
						if (result != null)
							return result;					
					}				
				}
			}

			return null;
		}

		private FunctionCall GetFunctionCallUnderMouse( int mx, int my )
		{
			if (m_report!=null)
			{
				foreach( FunctionCall f in m_report.TopFunctions )
				{
					FunctionCall result = RecGetFunctionCallUnderMouse(f, functionCallOffset - m_yOffset, mx, my );
					if (result != null)
						return result;
				}
			}
			return null;
		}
		
		private ProfileReport m_report;
		public ProfileReport Report
		{
			set 
			{ 
				m_report = value; 
				if (m_report!=null)
				{
					m_leftTime = m_report.MinimumTime;
					m_rightTime = m_report.MaximumTime;
				}
				Invalidate();
			}
		}

		private long m_leftTime;
		private long m_rightTime;
		private int m_yOffset;
		private int m_maxY;
		private bool m_panning;
		private Point m_prevMousePosition;

		private int ConvertTime( long time )
		{
			if (m_rightTime != m_leftTime)
				return (int)(this.Width * (double)(time - m_leftTime) / (double)(m_rightTime - m_leftTime));
			else
				return 0;
		}

		private long ConvertScreenDelta( int delta )
		{
			long timediff = m_rightTime - m_leftTime;
			double timespan = (double)delta / (double)this.Width;
			return (long)(timespan * timediff);
		}
	}
}
