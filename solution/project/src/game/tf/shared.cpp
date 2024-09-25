#include "shared.hpp"

void bf_write::StartWriting(void *pData, int nBytes, int iStartBit, int nBits)
{
	if (!(nBytes % 4 == 0))
		return;

	if (!(((uintptr_t)pData & 3) == 0))
		return;

	nBytes &= ~3;

	m_pData = (unsigned long *)pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = nBytes << 3;
	}
	else
	{
		if (nBits <= nBytes * 8)
		{
			m_nDataBits = nBits;
		}
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

#define	COORD_INTEGER_BITS 14
#define COORD_FRACTIONAL_BITS 5
#define COORD_DENOMINATOR (1<<(COORD_FRACTIONAL_BITS))
#define COORD_RESOLUTION (1.0/(COORD_DENOMINATOR))

#define COORD_INTEGER_BITS_MP 11
#define COORD_FRACTIONAL_BITS_MP_LOWPRECISION 3
#define COORD_DENOMINATOR_LOWPRECISION (1<<(COORD_FRACTIONAL_BITS_MP_LOWPRECISION))
#define COORD_RESOLUTION_LOWPRECISION (1.0/(COORD_DENOMINATOR_LOWPRECISION))

#define NORMAL_FRACTIONAL_BITS 11
#define NORMAL_DENOMINATOR ( (1<<(NORMAL_FRACTIONAL_BITS)) - 1 )
#define NORMAL_RESOLUTION (1.0/(NORMAL_DENOMINATOR))

#define FAST_BIT_SCAN 1

#include <intrin.h>
#include <math.h>

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

inline unsigned int CountLeadingZeros(unsigned int x)
{
	unsigned long firstBit;
	if (_BitScanReverse(&firstBit, x))
		return 31 - firstBit;
	return 32;
}

inline unsigned int CountTrailingZeros(unsigned int elem)
{
	unsigned long out;
	if (_BitScanForward(&out, elem))
		return out;
	return 32;
}

static BitBufErrorHandler g_BitBufErrorHandler = 0;

inline int BitForBitnum(int bitnum)
{
	return GetBitForBitnum(bitnum);
}

void InternalBitBufErrorHandler(BitBufErrorType errorType, const char *pDebugName)
{
	if (g_BitBufErrorHandler)
		g_BitBufErrorHandler(errorType, pDebugName);
}


void SetBitBufErrorHandler(BitBufErrorHandler fn)
{
	g_BitBufErrorHandler = fn;
}


unsigned long g_LittleBits[32];
unsigned long g_BitWriteMasks[32][33];
unsigned long g_ExtraMasks[33];

class CBitWriteMasksInit
{
public:
	CBitWriteMasksInit()
	{
		for (unsigned int startbit = 0; startbit < 32; startbit++)
		{
			for (unsigned int nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++)
			{
				unsigned int endbit = startbit + nBitsLeft;
				g_BitWriteMasks[startbit][nBitsLeft] = BitForBitnum(startbit) - 1;
				if (endbit < 32)
					g_BitWriteMasks[startbit][nBitsLeft] |= ~(BitForBitnum(endbit) - 1);
			}
		}

		for (unsigned int maskBit = 0; maskBit < 32; maskBit++)
			g_ExtraMasks[maskBit] = BitForBitnum(maskBit) - 1;
		g_ExtraMasks[32] = ~0ul;

		for (unsigned int littleBit = 0; littleBit < 32; littleBit++)
			StoreLittleDWord(&g_LittleBits[littleBit], 0, 1u << littleBit);
	}
};

static CBitWriteMasksInit g_BitWriteMasksInit;

bf_write::bf_write()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; 	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

bf_write::bf_write(const char *pDebugName, void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartWriting(pData, nBytes, 0, nBits);
}

bf_write::bf_write(void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
	StartWriting(pData, nBytes, 0, nBits);
}


void bf_write::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}


void bf_write::SetAssertOnOverflow(bool bAssert)
{
	m_bAssertOnOverflow = bAssert;
}


const char *bf_write::GetDebugName()
{
	return m_pDebugName;
}


void bf_write::SetDebugName(const char *pDebugName)
{
	m_pDebugName = pDebugName;
}


void bf_write::SeekToBit(int bitPos)
{
	m_iCurBit = bitPos;
}


void bf_write::WriteSBitLong(int data, int numbits)
{
	int nValue = data;
	int nPreserveBits = (0x7FFFFFFF >> (32 - numbits));
	int nSignExtension = (nValue >> 31) & ~nPreserveBits;
	nValue &= nPreserveBits;
	nValue |= nSignExtension;

	WriteUBitLong(nValue, numbits, false);
}

void bf_write::WriteVarInt32(uint32_t data)
{
	if ((m_iCurBit & 7) == 0 && (m_iCurBit + bitbuf::kMaxVarint32Bytes * 8) <= m_nDataBits)
	{
		uint8_t *target = ((uint8_t *)m_pData) + (m_iCurBit >> 3);

		target[0] = static_cast<uint8_t>(data | 0x80);
		if (data >= (1 << 7))
		{
			target[1] = static_cast<uint8_t>((data >> 7) | 0x80);
			if (data >= (1 << 14))
			{
				target[2] = static_cast<uint8_t>((data >> 14) | 0x80);
				if (data >= (1 << 21))
				{
					target[3] = static_cast<uint8_t>((data >> 21) | 0x80);
					if (data >= (1 << 28))
					{
						target[4] = static_cast<uint8_t>(data >> 28);
						m_iCurBit += 5 * 8;
						return;
					}
					else
					{
						target[3] &= 0x7F;
						m_iCurBit += 4 * 8;
						return;
					}
				}
				else
				{
					target[2] &= 0x7F;
					m_iCurBit += 3 * 8;
					return;
				}
			}
			else
			{
				target[1] &= 0x7F;
				m_iCurBit += 2 * 8;
				return;
			}
		}
		else
		{
			target[0] &= 0x7F;
			m_iCurBit += 1 * 8;
			return;
		}
	}
	else {
		while (data > 0x7F)
		{
			WriteUBitLong((data & 0x7F) | 0x80, 8);
			data >>= 7;
		}
		WriteUBitLong(data & 0x7F, 8);
	}
}

void bf_write::WriteVarInt64(uint64_t data)
{
	if ((m_iCurBit & 7) == 0 && (m_iCurBit + bitbuf::kMaxVarintBytes * 8) <= m_nDataBits)
	{
		uint8_t *target = ((uint8_t *)m_pData) + (m_iCurBit >> 3);

		uint32_t part0 = static_cast<uint32_t>(data);
		uint32_t part1 = static_cast<uint32_t>(data >> 28);
		uint32_t part2 = static_cast<uint32_t>(data >> 56);

		int size;

		if (part2 == 0)
		{
			if (part1 == 0)
			{
				if (part0 < (1 << 14))
				{
					if (part0 < (1 << 7))
					{
						size = 1; goto size1;
					}
					else
					{
						size = 2; goto size2;
					}
				}
				else
				{
					if (part0 < (1 << 21))
					{
						size = 3; goto size3;
					}
					else
					{
						size = 4; goto size4;
					}
				}
			}
			else
			{
				if (part1 < (1 << 14))
				{
					if (part1 < (1 << 7))
					{
						size = 5; goto size5;
					}
					else
					{
						size = 6; goto size6;
					}
				}
				else
				{
					if (part1 < (1 << 21))
					{
						size = 7; goto size7;
					}
					else
					{
						size = 8; goto size8;
					}
				}
			}
		}
		else
		{
			if (part2 < (1 << 7))
			{
				size = 9; goto size9;
			}
			else
			{
				size = 10; goto size10;
			}
		}

	size10: target[9] = static_cast<uint8_t>((part2 >> 7) | 0x80);
	size9: target[8] = static_cast<uint8_t>((part2) | 0x80);
	size8: target[7] = static_cast<uint8_t>((part1 >> 21) | 0x80);
	size7: target[6] = static_cast<uint8_t>((part1 >> 14) | 0x80);
	size6: target[5] = static_cast<uint8_t>((part1 >> 7) | 0x80);
	size5: target[4] = static_cast<uint8_t>((part1) | 0x80);
	size4: target[3] = static_cast<uint8_t>((part0 >> 21) | 0x80);
	size3: target[2] = static_cast<uint8_t>((part0 >> 14) | 0x80);
	size2: target[1] = static_cast<uint8_t>((part0 >> 7) | 0x80);
	size1: target[0] = static_cast<uint8_t>((part0) | 0x80);

		target[size - 1] &= 0x7F;
		m_iCurBit += size * 8;
	}
	else {
		while (data > 0x7F)
		{
			WriteUBitLong((data & 0x7F) | 0x80, 8);
			data >>= 7;
		}
		WriteUBitLong(data & 0x7F, 8);
	}
}

void bf_write::WriteSignedVarInt32(int32_t data)
{
	WriteVarInt32(bitbuf::ZigZagEncode32(data));
}

void bf_write::WriteSignedVarInt64(int64_t data)
{
	WriteVarInt64(bitbuf::ZigZagEncode64(data));
}

int	bf_write::ByteSizeVarInt32(uint32_t data)
{
	int size = 1;
	while (data > 0x7F)
	{
		size++;
		data >>= 7;
	}
	return size;
}

int	bf_write::ByteSizeVarInt64(uint64_t data)
{
	int size = 1;
	while (data > 0x7F)
	{
		size++;
		data >>= 7;
	}
	return size;
}

void bf_write::WriteBitLong(unsigned int data, int numbits, bool bSigned)
{
	if (bSigned)
		WriteSBitLong((int)data, numbits);
	else
		WriteUBitLong(data, numbits);
}

bool bf_write::WriteBits(const void *pInData, int nBits)
{
	unsigned char *pOut = (unsigned char *)pInData;
	int nBitsLeft = nBits;

	if ((m_iCurBit + nBits) > m_nDataBits)
	{
		SetOverflowFlag();
		return false;
	}

	while (((uintptr_t)pOut & 3) != 0 && nBitsLeft >= 8)
	{

		WriteUBitLong(*pOut, 8, false);
		++pOut;
		nBitsLeft -= 8;
	}

	if ((nBitsLeft >= 32) && (m_iCurBit & 7) == 0)
	{
		int numbytes = nBitsLeft >> 3;
		int numbits = numbytes << 3;

		memcpy((char *)m_pData + (m_iCurBit >> 3), pOut, numbytes);
		pOut += numbytes;
		nBitsLeft -= numbits;
		m_iCurBit += numbits;
	}

	if (nBitsLeft >= 32)
	{
		unsigned long iBitsRight = (m_iCurBit & 31);
		unsigned long iBitsLeft = 32 - iBitsRight;
		unsigned long bitMaskLeft = g_BitWriteMasks[iBitsRight][32];
		unsigned long bitMaskRight = g_BitWriteMasks[0][iBitsRight];

		unsigned long *pData = &m_pData[m_iCurBit >> 5];

		while (nBitsLeft >= 32)
		{
			unsigned long curData = *(unsigned long *)pOut;
			pOut += sizeof(unsigned long);

			*pData &= bitMaskLeft;
			*pData |= curData << iBitsRight;

			pData++;

			if (iBitsLeft < 32)
			{
				curData >>= iBitsLeft;
				*pData &= bitMaskRight;
				*pData |= curData;
			}

			nBitsLeft -= 32;
			m_iCurBit += 32;
		}
	}


	while (nBitsLeft >= 8)
	{
		WriteUBitLong(*pOut, 8, false);
		++pOut;
		nBitsLeft -= 8;
	}

	if (nBitsLeft)
	{
		WriteUBitLong(*pOut, nBitsLeft, false);
	}

	return !IsOverflowed();
}


bool bf_write::WriteBitsFromBuffer(bf_read *pIn, int nBits)
{
	while (nBits > 32)
	{
		WriteUBitLong(pIn->ReadUBitLong(32), 32);
		nBits -= 32;
	}

	WriteUBitLong(pIn->ReadUBitLong(nBits), nBits);
	return !IsOverflowed() && !pIn->IsOverflowed();
}


void bf_write::WriteBitAngle(float fAngle, int numbits)
{
	int d;
	unsigned int mask;
	unsigned int shift;

	shift = BitForBitnum(numbits);
	mask = shift - 1;

	d = (int)((fAngle / 360.0) * shift);
	d &= mask;

	WriteUBitLong((unsigned int)d, numbits);
}

void bf_write::WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision)
{
	int		signbit = (f <= -(bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION));
	int		intval = (int)fabsf(f);
	int		fractval = bLowPrecision ?
		(abs((int)(f * COORD_DENOMINATOR_LOWPRECISION)) & (COORD_DENOMINATOR_LOWPRECISION - 1)) :
		(abs((int)(f * COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1));

	bool    bInBounds = intval < (1 << COORD_INTEGER_BITS_MP);

	unsigned int bits, numbits;

	if (bIntegral)
	{
		if (intval)
		{
			--intval;
			bits = intval * 8 + signbit * 4 + 2 + bInBounds;
			numbits = 3 + (bInBounds ? COORD_INTEGER_BITS_MP : COORD_INTEGER_BITS);
		}
		else
		{
			bits = bInBounds;
			numbits = 2;
		}
	}
	else
	{
		if (intval)
		{
			--intval;
			bits = intval * 8 + signbit * 4 + 2 + bInBounds;
			bits += bInBounds ? (fractval << (3 + COORD_INTEGER_BITS_MP)) : (fractval << (3 + COORD_INTEGER_BITS));
			numbits = 3 + (bInBounds ? COORD_INTEGER_BITS_MP : COORD_INTEGER_BITS)
				+ (bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
		}
		else
		{
			bits = fractval * 8 + signbit * 4 + 0 + bInBounds;
			numbits = 3 + (bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
		}
	}

	WriteUBitLong(bits, numbits);
}

void bf_write::WriteBitCoord(const float f)
{
	int		signbit = (f <= -COORD_RESOLUTION);
	int		intval = (int)fabsf(f);
	int		fractval = abs((int)(f * COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1);


	WriteOneBit(intval);
	WriteOneBit(fractval);

	if (intval || fractval)
	{
		WriteOneBit(signbit);

		if (intval)
		{
			intval--;
			WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
		}

		if (fractval)
		{
			WriteUBitLong((unsigned int)fractval, COORD_FRACTIONAL_BITS);
		}
	}
}

void bf_write::WriteBitVec3Coord(const Vector &fa)
{
	int		xflag, yflag, zflag;

	xflag = (fa[0] >= COORD_RESOLUTION) || (fa[0] <= -COORD_RESOLUTION);
	yflag = (fa[1] >= COORD_RESOLUTION) || (fa[1] <= -COORD_RESOLUTION);
	zflag = (fa[2] >= COORD_RESOLUTION) || (fa[2] <= -COORD_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);
	WriteOneBit(zflag);

	if (xflag)
		WriteBitCoord(fa[0]);
	if (yflag)
		WriteBitCoord(fa[1]);
	if (zflag)
		WriteBitCoord(fa[2]);
}

void bf_write::WriteBitNormal(float f)
{
	int	signbit = (f <= -NORMAL_RESOLUTION);

	unsigned int fractval = abs((int)(f * NORMAL_DENOMINATOR));

	if (fractval > NORMAL_DENOMINATOR)
		fractval = NORMAL_DENOMINATOR;

	WriteOneBit(signbit);

	WriteUBitLong(fractval, NORMAL_FRACTIONAL_BITS);
}

void bf_write::WriteBitVec3Normal(const Vector &fa)
{
	int		xflag, yflag;

	xflag = (fa[0] >= NORMAL_RESOLUTION) || (fa[0] <= -NORMAL_RESOLUTION);
	yflag = (fa[1] >= NORMAL_RESOLUTION) || (fa[1] <= -NORMAL_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);

	if (xflag)
		WriteBitNormal(fa[0]);
	if (yflag)
		WriteBitNormal(fa[1]);

	int	signbit = (fa[2] <= -NORMAL_RESOLUTION);
	WriteOneBit(signbit);
}

void bf_write::WriteBitAngles(const Vector &fa)
{
	Vector tmp(fa.x, fa.y, fa.z);
	WriteBitVec3Coord(tmp);
}

void bf_write::WriteChar(int val)
{
	WriteSBitLong(val, sizeof(char) << 3);
}

void bf_write::WriteByte(int val)
{
	WriteUBitLong(val, sizeof(unsigned char) << 3);
}

void bf_write::WriteShort(int val)
{
	WriteSBitLong(val, sizeof(short) << 3);
}

void bf_write::WriteWord(int val)
{
	WriteUBitLong(val, sizeof(unsigned short) << 3);
}

void bf_write::WriteLong(long val)
{
	WriteSBitLong(val, sizeof(long) << 3);
}

void bf_write::WriteLongLong(int64_t val)
{
	unsigned int *pLongs = (unsigned int *)&val;

	const short endianIndex = 0x0100;
	char *idx = (char *)&endianIndex;
	WriteUBitLong(pLongs[*idx++], sizeof(long) << 3);
	WriteUBitLong(pLongs[*idx], sizeof(long) << 3);
}

void bf_write::WriteFloat(float val)
{
	LittleFloat(&val, &val);

	WriteBits(&val, sizeof(val) << 3);
}

bool bf_write::WriteBytes(const void *pBuf, int nBytes)
{
	return WriteBits(pBuf, nBytes << 3);
}

bool bf_write::WriteString(const char *pStr)
{
	if (pStr)
	{
		do
		{
			WriteChar(*pStr);
			++pStr;
		} while (*(pStr - 1) != 0);
	}
	else
	{
		WriteChar(0);
	}

	return !IsOverflowed();
}

bf_read::bf_read()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; 	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

bf_read::bf_read(const void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	StartReading(pData, nBytes, 0, nBits);
}

bf_read::bf_read(const char *pDebugName, const void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartReading(pData, nBytes, 0, nBits);
}

void bf_read::StartReading(const void *pData, int nBytes, int iStartBit, int nBits)
{
	m_pData = (unsigned long *)pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = m_nDataBytes << 3;
	}
	else
	{
		m_nDataBits = nBits;
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

void bf_read::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}

void bf_read::SetAssertOnOverflow(bool bAssert)
{
	m_bAssertOnOverflow = bAssert;
}

void bf_read::SetDebugName(const char *pName)
{
	m_pDebugName = pName;
}

void bf_read::SetOverflowFlag()
{
	if (m_bAssertOnOverflow)
	{
	}
	m_bOverflow = true;
}

unsigned int bf_read::CheckReadUBitLong(int numbits)
{
	int i, nBitValue;
	unsigned int r = 0;

	for (i = 0; i < numbits; i++)
	{
		nBitValue = ReadOneBitNoCheck();
		r |= nBitValue << i;
	}
	m_iCurBit -= numbits;

	return r;
}

void bf_read::ReadBits(void *pOutData, int nBits)
{
	unsigned char *pOut = (unsigned char *)pOutData;
	int nBitsLeft = nBits;

	while (((size_t)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		*pOut = (unsigned char)ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	while (nBitsLeft >= 32)
	{
		*((unsigned long *)pOut) = ReadUBitLong(32);
		pOut += sizeof(unsigned long);
		nBitsLeft -= 32;
	}

	while (nBitsLeft >= 8)
	{
		*pOut = ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	if (nBitsLeft)
	{
		*pOut = ReadUBitLong(nBitsLeft);
	}

}

int bf_read::ReadBitsClamped_ptr(void *pOutData, size_t outSizeBytes, size_t nBits)
{
	size_t outSizeBits = outSizeBytes * 8;
	size_t readSizeBits = nBits;
	int skippedBits = 0;
	if (readSizeBits > outSizeBits)
	{
		readSizeBits = outSizeBits;
		skippedBits = (int)(nBits - outSizeBits);
	}

	ReadBits(pOutData, static_cast<int>(readSizeBits));
	SeekRelative(skippedBits);

	return (int)readSizeBits;
}

float bf_read::ReadBitAngle(int numbits)
{
	float fReturn;
	int i;
	float shift;

	shift = (float)(BitForBitnum(numbits));

	i = ReadUBitLong(numbits);
	fReturn = (float)i * (360.0 / shift);

	return fReturn;
}

unsigned int bf_read::PeekUBitLong(int numbits)
{
	unsigned int r;
	int i, nBitValue;

	bf_read savebf;

	savebf = *this;
	r = 0;
	for (i = 0; i < numbits; i++)
	{
		nBitValue = ReadOneBit();

		if (nBitValue)
		{
			r |= BitForBitnum(i);
		}
	}

	*this = savebf;

	return r;
}

unsigned int bf_read::ReadUBitLongNoInline(int numbits)
{
	return ReadUBitLong(numbits);
}

unsigned int bf_read::ReadUBitVarInternal(int encodingType)
{
	m_iCurBit -= 4;
	int bits = 4 + encodingType * 4 + (((2 - encodingType) >> 31) & 16);
	return ReadUBitLong(bits);
}

int bf_read::ReadSBitLong(int numbits)
{
	unsigned int r = ReadUBitLong(numbits);
	unsigned int s = 1 << (numbits - 1);
	if (r >= s)
	{
		r = r - s - s;
	}
	return r;
}

uint32_t bf_read::ReadVarInt32()
{
	uint32_t result = 0;
	int count = 0;
	uint32_t b;

	do
	{
		if (count == bitbuf::kMaxVarint32Bytes)
		{
			return result;
		}
		b = ReadUBitLong(8);
		result |= (b & 0x7F) << (7 * count);
		++count;
	} while (b & 0x80);

	return result;
}

uint64_t bf_read::ReadVarInt64()
{
	uint64_t result = 0;
	int count = 0;
	uint64_t b;

	do
	{
		if (count == bitbuf::kMaxVarintBytes)
		{
			return result;
		}
		b = ReadUBitLong(8);
		result |= static_cast<uint64_t>(b & 0x7F) << (7 * count);
		++count;
	} while (b & 0x80);

	return result;
}

int32_t bf_read::ReadSignedVarInt32()
{
	uint32_t value = ReadVarInt32();
	return bitbuf::ZigZagDecode32(value);
}

int64_t bf_read::ReadSignedVarInt64()
{
	uint32_t value = ReadVarInt64();
	return bitbuf::ZigZagDecode64(value);
}

unsigned int bf_read::ReadBitLong(int numbits, bool bSigned)
{
	if (bSigned)
		return (unsigned int)ReadSBitLong(numbits);
	else
		return ReadUBitLong(numbits);
}


float bf_read::ReadBitCoord(void)
{
	int		intval = 0, fractval = 0, signbit = 0;
	float	value = 0.0;


	intval = ReadOneBit();
	fractval = ReadOneBit();

	if (intval || fractval)
	{
		signbit = ReadOneBit();

		if (intval)
		{
			intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
		}

		if (fractval)
		{
			fractval = ReadUBitLong(COORD_FRACTIONAL_BITS);
		}

		value = intval + ((float)fractval * COORD_RESOLUTION);

		if (signbit)
			value = -value;
	}

	return value;
}

float bf_read::ReadBitCoordMP(bool bIntegral, bool bLowPrecision)
{
	int flags = ReadUBitLong(3 - bIntegral);
	enum { INBOUNDS = 1, INTVAL = 2, SIGN = 4 };

	if (bIntegral)
	{
		if (flags & INTVAL)
		{
			unsigned int bits = ReadUBitLong((flags & INBOUNDS) ? COORD_INTEGER_BITS_MP + 1 : COORD_INTEGER_BITS + 1);
			int intval = (bits >> 1) + 1;
			return (bits & 1) ? -intval : intval;
		}
		return 0.f;
	}

	static const float mul_table[4] =
	{
		1.f / (1 << COORD_FRACTIONAL_BITS),
		-1.f / (1 << COORD_FRACTIONAL_BITS),
		1.f / (1 << COORD_FRACTIONAL_BITS_MP_LOWPRECISION),
		-1.f / (1 << COORD_FRACTIONAL_BITS_MP_LOWPRECISION)
	};
	float multiply = *(float *)((uintptr_t)&mul_table[0] + (flags & 4) + bLowPrecision * 8);

	static const unsigned char numbits_table[8] =
	{
		COORD_FRACTIONAL_BITS,
		COORD_FRACTIONAL_BITS,
		COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS,
		COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS_MP,
		COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
		COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
		COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS,
		COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS_MP
	};
	unsigned int bits = ReadUBitLong(numbits_table[(flags & (INBOUNDS | INTVAL)) + bLowPrecision * 4]);

	if (flags & INTVAL)
	{

		unsigned int fracbitsMP = bits >> COORD_INTEGER_BITS_MP;
		unsigned int fracbits = bits >> COORD_INTEGER_BITS;

		unsigned int intmaskMP = ((1 << COORD_INTEGER_BITS_MP) - 1);
		unsigned int intmask = ((1 << COORD_INTEGER_BITS) - 1);

		unsigned int selectNotMP = (flags & INBOUNDS) - 1;

		fracbits -= fracbitsMP;
		fracbits &= selectNotMP;
		fracbits += fracbitsMP;

		intmask -= intmaskMP;
		intmask &= selectNotMP;
		intmask += intmaskMP;

		unsigned int intpart = (bits & intmask) + 1;
		unsigned int intbitsLow = intpart << COORD_FRACTIONAL_BITS_MP_LOWPRECISION;
		unsigned int intbits = intpart << COORD_FRACTIONAL_BITS;
		unsigned int selectNotLow = (unsigned int)bLowPrecision - 1;

		intbits -= intbitsLow;
		intbits &= selectNotLow;
		intbits += intbitsLow;

		bits = fracbits | intbits;
	}

	return (int)bits * multiply;
}

unsigned int bf_read::ReadBitCoordBits(void)
{
	unsigned int flags = ReadUBitLong(2);
	if (flags == 0)
		return 0;

	static const int numbits_table[3] =
	{
		COORD_INTEGER_BITS + 1,
		COORD_FRACTIONAL_BITS + 1,
		COORD_INTEGER_BITS + COORD_FRACTIONAL_BITS + 1
	};
	return ReadUBitLong(numbits_table[flags - 1]) * 4 + flags;
}

unsigned int bf_read::ReadBitCoordMPBits(bool bIntegral, bool bLowPrecision)
{
	unsigned int flags = ReadUBitLong(2);
	enum { INBOUNDS = 1, INTVAL = 2 };
	int numbits = 0;

	if (bIntegral)
	{
		if (flags & INTVAL)
		{
			numbits = (flags & INBOUNDS) ? (1 + COORD_INTEGER_BITS_MP) : (1 + COORD_INTEGER_BITS);
		}
		else
		{
			return flags;
		}
	}
	else
	{
		static const unsigned char numbits_table[8] =
		{
			1 + COORD_FRACTIONAL_BITS,
			1 + COORD_FRACTIONAL_BITS,
			1 + COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS,
			1 + COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS_MP,
			1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
			1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
			1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS,
			1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS_MP
		};
		numbits = numbits_table[flags + bLowPrecision * 4];
	}

	return flags + ReadUBitLong(numbits) * 4;
}

void bf_read::ReadBitVec3Coord(Vector &fa)
{
	int		xflag, yflag, zflag;

	fa = Vector(0, 0, 0);

	xflag = ReadOneBit();
	yflag = ReadOneBit();
	zflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitCoord();
	if (yflag)
		fa[1] = ReadBitCoord();
	if (zflag)
		fa[2] = ReadBitCoord();
}

float bf_read::ReadBitNormal(void)
{
	int	signbit = ReadOneBit();

	unsigned int fractval = ReadUBitLong(NORMAL_FRACTIONAL_BITS);

	float value = (float)fractval * NORMAL_RESOLUTION;

	if (signbit)
		value = -value;

	return value;
}

void bf_read::ReadBitVec3Normal(Vector &fa)
{
	int xflag = ReadOneBit();
	int yflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitNormal();
	else
		fa[0] = 0.0f;

	if (yflag)
		fa[1] = ReadBitNormal();
	else
		fa[1] = 0.0f;

	int znegative = ReadOneBit();

	float fafafbfb = fa[0] * fa[0] + fa[1] * fa[1];
	if (fafafbfb < 1.0f)
		fa[2] = sqrt(1.0f - fafafbfb);
	else
		fa[2] = 0.0f;

	if (znegative)
		fa[2] = -fa[2];
}

void bf_read::ReadBitAngles(Vector &fa)
{
	Vector tmp;
	ReadBitVec3Coord(tmp);
}

int64_t bf_read::ReadLongLong()
{
	int64_t retval;
	unsigned int *pLongs = (unsigned int *)&retval;

	const short endianIndex = 0x0100;
	char *idx = (char *)&endianIndex;
	pLongs[*idx++] = ReadUBitLong(sizeof(long) << 3);
	pLongs[*idx] = ReadUBitLong(sizeof(long) << 3);

	return retval;
}

float bf_read::ReadFloat()
{
	float ret;
	ReadBits(&ret, 32);

	LittleFloat(&ret, &ret);
	return ret;
}

bool bf_read::ReadBytes(void *pOut, int nBytes)
{
	ReadBits(pOut, nBytes << 3);
	return !IsOverflowed();
}

bool bf_read::ReadString(char *pStr, int maxLen, bool bLine, int *pOutNumChars)
{
	bool bTooSmall = false;
	int iChar = 0;
	while (1)
	{
		char val = ReadChar();
		if (val == 0)
			break;
		else if (bLine && val == '\n')
			break;

		if (iChar < (maxLen - 1))
		{
			pStr[iChar] = val;
			++iChar;
		}
		else
		{
			bTooSmall = true;
		}
	}

	pStr[iChar] = 0;

	if (pOutNumChars)
		*pOutNumChars = iChar;

	return !IsOverflowed() && !bTooSmall;
}


char *bf_read::ReadAndAllocateString(bool *pOverflow)
{
	char str[2048];

	int nChars;
	bool bOverflow = !ReadString(str, sizeof(str), false, &nChars);
	if (pOverflow)
		*pOverflow = bOverflow;

	char *pRet = new char[nChars + 1];
	for (int i = 0; i <= nChars; i++)
		pRet[i] = str[i];

	return pRet;
}

void bf_read::ExciseBits(int startbit, int bitstoremove)
{
	int endbit = startbit + bitstoremove;
	int remaining_to_end = m_nDataBits - endbit;

	bf_write temp;
	temp.StartWriting((void *)m_pData, m_nDataBits << 3, startbit);

	Seek(endbit);

	for (int i = 0; i < remaining_to_end; i++)
	{
		temp.WriteOneBit(ReadOneBit());
	}

	Seek(startbit);

	m_nDataBits -= bitstoremove;
	m_nDataBytes = m_nDataBits >> 3;
}

int bf_read::CompareBitsAt(int offset, bf_read *other, int otherOffset, int numbits)
{
	extern unsigned long g_ExtraMasks[33];

	if (numbits == 0)
		return 0;

	int overflow1 = offset + numbits > m_nDataBits;
	int overflow2 = otherOffset + numbits > other->m_nDataBits;

	int x = overflow1 | overflow2;
	if (x != 0)
		return x;

	unsigned int iStartBit1 = offset & 31u;
	unsigned int iStartBit2 = otherOffset & 31u;
	unsigned long *pData1 = (unsigned long *)m_pData + (offset >> 5);
	unsigned long *pData2 = (unsigned long *)other->m_pData + (otherOffset >> 5);
	unsigned long *pData1End = pData1 + ((offset + numbits - 1) >> 5);
	unsigned long *pData2End = pData2 + ((otherOffset + numbits - 1) >> 5);

	while (numbits > 32)
	{
		x = LoadLittleDWord((unsigned long *)pData1, 0) >> iStartBit1;
		x ^= LoadLittleDWord((unsigned long *)pData1, 1) << (32 - iStartBit1);
		x ^= LoadLittleDWord((unsigned long *)pData2, 0) >> iStartBit2;
		x ^= LoadLittleDWord((unsigned long *)pData2, 1) << (32 - iStartBit2);
		if (x != 0)
		{
			return x;
		}
		++pData1;
		++pData2;
		numbits -= 32;
	}

	x = LoadLittleDWord((unsigned long *)pData1, 0) >> iStartBit1;
	x ^= LoadLittleDWord((unsigned long *)pData1End, 0) << (32 - iStartBit1);
	x ^= LoadLittleDWord((unsigned long *)pData2, 0) >> iStartBit2;
	x ^= LoadLittleDWord((unsigned long *)pData2End, 0) << (32 - iStartBit2);
	return (x & g_ExtraMasks[numbits]);
}

CEntitySphereQuery::CEntitySphereQuery(const Vector &center, float radius, int flagMask, int partitionMask)
{
	s::CEntitySphereQuery_CEntitySphereQuery.call<void>(this, center, radius, flagMask, partitionMask);
}

C_BaseEntity *CEntitySphereQuery::GetCurrentEntity()
{
	if (m_listIndex < m_listCount) {
		return m_pList[m_listIndex];
	}

	return NULL;
}