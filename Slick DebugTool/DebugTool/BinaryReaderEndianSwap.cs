using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Sys.IO
{
	public class BinaryReaderEndianSwap : BinaryReader
	{
		public BinaryReaderEndianSwap(Stream output)
			: base(output)
		{
		}

		public BinaryReaderEndianSwap(Stream output, Encoding encoding)
			: base(output, encoding)
		{
		}

		// Swaps the endian bytes for the given block, assuming it's 
		public static byte[] EndianSwap(byte[] input)
		{
			byte [] result = new byte[input.Length];

			for (int i = 0; i < input.Length; ++i)
			{
				result[i] = input[input.Length - 1 - i];
			}

			return result;
		}

		public override decimal ReadDecimal()
		{
			throw new Exception("Not supported");
		}

		public override double ReadDouble()
		{
			return BitConverter.ToDouble( EndianSwap( ReadBytes(8) ), 0);
		}

		public override float ReadSingle()
		{
			return BitConverter.ToSingle(EndianSwap(ReadBytes(4)), 0);
		}

		public override int ReadInt32()
		{
			return BitConverter.ToInt32(EndianSwap(ReadBytes(4)), 0);
		}

		public override long ReadInt64()
		{
			return BitConverter.ToInt64(EndianSwap(ReadBytes(8)), 0);
		}

		public override short ReadInt16()
		{
			return BitConverter.ToInt16(EndianSwap(ReadBytes(2)), 0);
		}

		private new int Read7BitEncodedInt()
		{
			int ret = 0;
			for (int shift = 0; shift < 35; shift += 7)
			{
				int b = ReadByte();
				if (b == -1)
				{
					throw new EndOfStreamException();
				}
				ret = ret | ((b & 0x7f) << shift);
				if ((b & 0x80) == 0)
				{
					return ret;
				}
			}
			throw new IOException("Invalid 7-bit encoded integer in stream.");
		}

		public override string ReadString()
		{
			char[] chars = ReadChars(Read7BitEncodedInt());
			return new string(chars);
		}

		public override uint ReadUInt32()
		{
			return BitConverter.ToUInt32(EndianSwap(ReadBytes(4)), 0);
		}

		public override ulong ReadUInt64()
		{
			return BitConverter.ToUInt64(EndianSwap(ReadBytes(8)), 0);
		}

		public override ushort ReadUInt16()
		{
			return BitConverter.ToUInt16(EndianSwap(ReadBytes(2)), 0);
		}
	}
}
