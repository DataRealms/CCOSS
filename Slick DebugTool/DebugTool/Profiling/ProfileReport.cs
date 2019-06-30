using System;
using System.Collections.Generic;
using System.Text;
using DebugTool.Profiling.Sampling;

namespace DebugTool.Profiling
{
	public class ProfileReport
	{
		private ProfileReport()
		{
			m_topHierarchy = new List<FunctionCall>();
			m_allFunctionCalls = new List<FunctionCall>();
			m_frequency = 0;
		}

		private List<FunctionCall> m_topHierarchy;
		private List<FunctionCall> m_allFunctionCalls;
		private long m_frequency;

		public long Frequency
		{
			get { return m_frequency; }
		}
		public long MinimumTime
		{
			get 
			{
				if (m_topHierarchy.Count > 0)
				{
					return m_topHierarchy[0].StartTime;
				}

				return 0;
			}
		}
		public long MaximumTime
		{
			get 
			{
				if (m_topHierarchy.Count > 0)
				{
					return m_topHierarchy[m_topHierarchy.Count-1].EndTime;
				}

				return 0;
			}
		}
		public List<FunctionCall> TopFunctions
		{
			get { return m_topHierarchy; }
		}

		private static bool RecGenerate( ProfileReport report, List<ProfileSample> samples, ref int i, FunctionCall f)
		{
			int size = samples.Count;
			++i;

			for (;i<size; ++i)
			{
				if (samples[i].Enter)
				{
					FunctionCall fchild = new FunctionCall( samples[i].Name, samples[i].Time, samples[i].Color );

					RecGenerate( report, samples, ref i, fchild );

					f.AddChild( fchild );

					report.m_allFunctionCalls.Add( fchild );
				}
				else
				{
					f.EndTime = samples[i].Time;
					return true;
				}
			}

			return false;
		}
		public static ProfileReport GenerateFromSamples( long frequency, List<ProfileSample> samples )
		{
			ProfileReport report = new ProfileReport();
			report.m_frequency = frequency;

			int size = samples.Count;

			for (int i=0; i<size; ++i)
			{
				if (samples[i].Enter)
				{
					FunctionCall f = new FunctionCall( samples[i].Name, samples[i].Time, samples[i].Color );

					if (!RecGenerate( report, samples, ref i, f ))
					{
						// the function never ended, so make the end time equal to the last end time
						if (f.Children.Count > 0)
						{
							f.EndTime = f.Children[f.Children.Count-1].EndTime;
						}
						else
						{
							f.EndTime = f.StartTime;
						}
					}

					report.m_allFunctionCalls.Add( f );
					report.m_topHierarchy.Add( f );
				}
			}
			return report;
		}
	}
}
