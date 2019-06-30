using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Sys.IO
{
	public class BinaryWriterEndianSwap : BinaryWriter
	{
		public BinaryWriterEndianSwap(Stream output)
			: base(output)
		{
		}

		public BinaryWriterEndianSwap(Stream output, Encoding encoding)
			: base(output, encoding)
		{
		}

		public override void Write(decimal value)
		{
			throw new Exception("Not supported");
		}

		public override void Write(double value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap( BitConverter.GetBytes( value ) ) );
		}

		public override void Write(float value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap( BitConverter.GetBytes( value ) ) );
		}

		public override void Write(int value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}

		public override void Write(long value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}

		public override void Write(short value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}

		private new void Write7BitEncodedInt(int value)
		{
			while (value >= 128)
			{
				Write( (byte)((value & 0x7f) | 0x80 ) );
				value = value >> 7;
			}
			Write((byte)value);
		}

		public override void Write(string value)
		{
			Write7BitEncodedInt(value.Length);
			Write( value.ToCharArray() );
		}

		public override void Write(uint value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}

		public override void Write(ulong value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}

		public override void Write(ushort value)
		{
			Write(BinaryReaderEndianSwap.EndianSwap(BitConverter.GetBytes(value)));
		}
	}
}
