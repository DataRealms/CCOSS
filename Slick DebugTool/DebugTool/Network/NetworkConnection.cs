using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Threading;
using Sys.IO;

namespace DebugTool.Network
{
	public class NetworkConnection
	{
		public delegate void HandleNetMessage( DebugMessage message );

		public NetworkConnection()
		{
			m_messageHandlers = new Dictionary<uint,HandleNetMessage>();
			m_connected = false;
			m_bigEndian = false;
			m_networkMessages = new Queue<DebugMessage>();
		}

		/// <summary>
		/// Tries to create a network connection to the given IP address
		/// </summary>
		/// <param name="ip">ip to connect to</param>
		/// <param name="port">port to connect to</param>
		public void Connect( string ip, int port )
		{
			// Connect to the ip and port
			m_client = new TcpClient();
			m_client.Connect( IPAddress.Parse( ip ), port );

			// Read the welcome message, this will tell if we're connected to a little or big endian application:
			NetworkStream stream = m_client.GetStream();

			BinaryReader reader = new BinaryReader( stream );
			byte[] connectmsg = reader.ReadBytes(4);

			uint id = BitConverter.ToUInt32(connectmsg, 0);
			if (id == 0x44454247)	// 'DEBG'
			{
				m_reader = new BinaryReader( stream );
				m_bigEndian = false;
			}
			else
			if (id == 0x47424544)	// 'GBED' --> Reverse of 'DEBG', so must be different endian-ness
			{
				m_reader = new BinaryReaderEndianSwap( stream );
				m_bigEndian = true;
			}

			m_connected = true;

			m_connectedTo = ip;

			if (m_bigEndian)
			{
				m_writer = new BinaryWriterEndianSwap(stream);
			}
			else
			{
				m_writer = new BinaryWriter(stream);
			}

			// Start the network message thread:
			m_exitNetworkThread = false;
			m_networkMessages = new Queue<DebugMessage>();
			m_networkThread = new Thread( new ThreadStart( NetworkMessageThread ) );
			m_networkThread.Name = "Network Thread";
			m_networkThread.Start();
		}
		public void Disconnect()
		{
			m_connected = false;
			m_connectedTo = "Disconnected";

			m_exitNetworkThread = true;
			m_networkThread.Join();

			if (m_client.Connected)
			{
				m_client.Close();
			}

			m_reader = null;
			m_writer = null;
		}

		// This function has to be called on a regular basis:
		public void DispatchMessages()
		{
			// Copy all messages to a local queue so that we don't stall the network while processing the messages:
			Queue<DebugMessage> queueToHandle;

			lock( m_thisLock )
			{
				queueToHandle = new Queue<DebugMessage>(m_networkMessages);
				m_networkMessages.Clear();
			}

			// Now handle the queue:
			while (queueToHandle.Count > 0)
			{
				HandleNetworkMessage( queueToHandle.Dequeue());
			}
		}

		public void AddMessageHandler( uint messageID, HandleNetMessage functionToCall )
		{
			if (m_messageHandlers.ContainsKey(messageID))
			{
				m_messageHandlers[messageID] += functionToCall;
			}
			else
			{
				m_messageHandlers.Add(messageID, functionToCall);
			}
		}

		public void SendMessage(DebugMessage msg)
		{
			if (m_writer != null)
			{
				using (MemoryStream s = new MemoryStream())
				{
					BinaryWriter writer = new BinaryWriter(s);

					msg.Serialize(writer);

					int length = (int)s.Length;

					m_writer.Write(msg.Identifier);
					m_writer.Write(length);
				}

				// Write it again, but now write it to the network stream
				msg.Serialize(m_writer);
			}
		}

		public bool Connected
		{
			get { return m_connected; }
		}
		public string ConnectedTo
		{
			get { return m_connectedTo; }
		}

		#region Internal Code
		private void NetworkMessageThread()
		{
			// Update the state:
			while(!m_exitNetworkThread)
			{

				// Read messages until the connection runs out of data
				while (m_client.Available > 8)
				{
					// Read the network message:
					try
					{
						uint type = m_reader.ReadUInt32();
						int size = m_reader.ReadInt32();

						List<byte> bytesRead = new List<byte>();
						while (bytesRead.Count < size)
						{
							if (m_client.Available > 0)
							{
								byte[] readbytes = m_reader.ReadBytes(Math.Min(m_client.Available, size - bytesRead.Count));
								bytesRead.AddRange(readbytes);
							}
							else
							{
								Thread.Sleep(0);	// go asleep for a bit, waiting for the data to come in.
							}
						}
						
						byte[] bytes = bytesRead.ToArray();
						
						DebugMessage msg = DebugMessageFactory.CreateDebugMessage(type);	// If we can't create this message yet, discard it
						if (msg!=null)
						{
							using (Stream s = new MemoryStream(bytes))
							{
								BinaryReader reader;
								if (m_bigEndian)
									reader = new BinaryReaderEndianSwap(s);
								else
									reader = new BinaryReader(s);

								msg.Deserialize(reader);
							}

							// Add the network message to the queue of messages:
							lock( m_thisLock )
							{
								m_networkMessages.Enqueue( msg );
							}						
						}
					}
					catch
					{
						// On any error, we disconnect:
						Disconnect();
					}
				}

				// Go to sleep for a bit:
				Thread.Sleep( 10 );
			}
		}

		private void HandleNetworkMessage(DebugMessage msg)
		{
			// look in our dictionary to see if we have any people who might be interested in this message:
			if (m_messageHandlers.ContainsKey( msg.Identifier ))
			{
				m_messageHandlers[msg.Identifier]( msg );				
			}

			// If not, the message simply disappears:
		}		
		#endregion

		#region Variables
		private bool m_connected;
		private TcpClient m_client;
		private bool m_bigEndian;
		private BinaryWriter m_writer;
		private BinaryReader m_reader;
		private volatile bool m_exitNetworkThread;
		private string m_connectedTo;

		private Thread m_networkThread;
		private Queue<DebugMessage> m_networkMessages;

		private Dictionary<uint, HandleNetMessage> m_messageHandlers;

		private Object m_thisLock = new Object();
		#endregion
	}
}
