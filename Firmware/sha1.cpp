#include "sha1.h"
#include <avr/pgmspace.h>
#include <string.h>

#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

Sha1::Sha1()
{
 uint8_t buf [] = {
		  0x01,0x23,0x45,0x67, // H0
		  0x89,0xab,0xcd,0xef, // H1
		  0xfe,0xdc,0xba,0x98, // H2
		  0x76,0x54,0x32,0x10, // H3
		  0xf0,0xe1,0xd2,0xc3  // H4
		};
  memcpy(state.b, buf, sizeof(buf));
  byteCount = 0;
  bufferOffset = 0;
}

static uint32_t rol32(uint32_t number, uint8_t bits)
{
  return ((number << bits) | (number >> (32-bits)));
}

void Sha1::hashBlock()
{
  uint32_t a = state.w[0];
  uint32_t b = state.w[1];
  uint32_t c = state.w[2];
  uint32_t d = state.w[3];
  uint32_t e = state.w[4];

  for (uint8_t i = 0; i < 80; ++i)
  {
    uint32_t t;
    if (i >= 16)
    {
      t = buffer.w[(i+13)&15] ^ buffer.w[(i+8)&15] ^ buffer.w[(i+2)&15] ^ buffer.w[i&15];
      buffer.w[i&15] = rol32(t,1);
    }
    if (i < 20)
    {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    }
    else if (i < 40)
    {
      t = (b ^ c ^ d) + SHA1_K20;
    }
    else if (i < 60)
    {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    }
    else
    {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t += rol32(a, 5) + e + buffer.w[i&15];
    e = d;
    d = c;
    c = rol32(b, 30);
    b = a;
    a = t;
  }
  state.w[0] += a;
  state.w[1] += b;
  state.w[2] += c;
  state.w[3] += d;
  state.w[4] += e;
}

void Sha1::addUncounted(uint8_t data)
{
  buffer.b[bufferOffset ^ 3] = data;
  bufferOffset++;
  if (bufferOffset == BlockLength)
  {
    hashBlock();
    bufferOffset = 0;
  }
}

size_t Sha1::write(uint8_t data)
{
  ++byteCount;
  addUncounted(data);

  return sizeof(data);
}

void Sha1::pad()
{
  // Implement SHA-1 padding (fips180-2 §5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  while (bufferOffset != 56) addUncounted(0x00);

  // Append length in the last 8 bytes
  addUncounted(0); // We're only using 32 bit lengths
  addUncounted(0); // But SHA-1 supports 64 bit lengths
  addUncounted(0); // So zero pad the top bits
  addUncounted(byteCount >> 29); // Shifting to multiply by 8
  addUncounted(byteCount >> 21); // as SHA-1 supports bitstreams as well as
  addUncounted(byteCount >> 13); // byte.
  addUncounted(byteCount >> 5);
  addUncounted(byteCount << 3);
}

void Sha1::update(uint8_t const* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; ++i)
    {
        write(data[i]);
    }
}

void Sha1::update(uint8_t data)
{
    write(data);
}

void Sha1::finish(uint8_t* out)
{
  // Pad to complete the last block
  pad();
  
  // Swap byte order back
  for (uint8_t i = 0; i < 5; ++i)
  {
    uint32_t a = state.w[i];
    uint32_t b = a<<24;
    b |= (a<<8) & 0x00ff0000;
    b |= (a>>8) & 0x0000ff00;
    b |= a>>24;
    state.w[i] = b;
  }
  
  // Return pointer to hash (20 characters)
  memcpy(out, state.b, 20);
}
