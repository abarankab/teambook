/*
 * libtamias/utilities.cpp — some useful functions
 * notes: that's bad to use short name in internal functions. It may cause problems with linking.
 */

#include "../../include/wtf/utilities.h"

using tamias::chartype;
using tamias::inttype64;
using tamias::sizetype;
using tamias::uinttype32;
using tamias::uinttype64;
using tamias::ByteArray;
using tamias::String;
using tamias::Vector;

Vector <String> tamias::wtf::utilities::splitString( const String &source, chartype splitter )
{
  Vector <String> res;
  String t = "";
  for (sizetype i = 0; i < source.length(); i++)
    if (source[i] == splitter)
    {
      if (t != "")
        res.pushBack(t);
      t = "";
    }
    else
      t += source[i];
  if (t != "")
    res.pushBack(t);
  return res;
}

Vector <String> tamias::wtf::utilities::splitString( const String &source, const String &splitter )
{
  Vector <String> res;
  String t = "";
  uinttype64 hash1 = 0, hash2 = 0, hashp = 1;
  for (sizetype i = 0; i < splitter.length(); i++)
    hash1 = 301703 * hash1 + splitter[i], hashp *= 301703;
  for (sizetype i = 0; i < splitter.length() && i < source.length(); i++)
    hash2 = 301703 * hash2 + source[i];
  for (sizetype i = 0; i < source.length(); i++)
  {
    bool flag = hash1 == hash2 && i + splitter.length() <= source.length();
    for (sizetype j = 0; flag && j < splitter.length(); j++)
      if (source[j + i] != splitter[j])
        flag = false;
    if (flag)
    {
      if (t != "")
        res.pushBack(t);
      t = "";
      for (sizetype j = 1; j < splitter.length(); j++, i++)
        if (i + splitter.length() < source.length())
          hash2 = hash2 * 301703 - source[i] * hashp + source[i + splitter.length()];
    }
    else
      t += source[i];
    if (i + splitter.length() < source.length())
      hash2 = hash2 * 301703 - source[i] * hashp + source[i + splitter.length()];
  }
  if (t != "")
    res.pushBack(t);
  return res;
}

String tamias::wtf::utilities::randomString( sizetype length )
{
  String res = "";
  for (sizetype i = 0; i < length; i++)
  {
    int x = rand() % 62;
    if (x < 10)
      res += '0' + x;
    else if (x < 36)
      res += 'A' + x - 10;
    else
      res += 'a' + x - 36;
  }
  return res;
}

typedef struct {
  uinttype32 state[4];                             /* state (ABCD) */
  uinttype64 count;     /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                       /* input buffer */
} MD5_CTX;

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform( uinttype32[4], const unsigned char[64] );

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
  Rotation is separate from addition to prevent recomputation. */
#define FF(a, b, c, d, x, s, ac)\
{ \
  (a) += F ((b), (c), (d)) + (x) + (uinttype32)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) \
{ \
  (a) += G ((b), (c), (d)) + (x) + (uinttype32)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) \
{ \
  (a) += H ((b), (c), (d)) + (x) + (uinttype32)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) \
{ \
  (a) += I ((b), (c), (d)) + (x) + (uinttype32)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
}

/* MD5 initialization. Begins an MD5 operation, writing a new context. */
void MD5Init( MD5_CTX *context )
{
  context->count = 0;
  /* Load magic initialization constants. */
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context. */
void MD5Update( MD5_CTX *context, const unsigned char *input, unsigned int inputLen )
{
  unsigned int i, index, partLen;
  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count >> 3) & 0x3F);
  /* Update number of bits */
  context->count += inputLen << 3;

  partLen = 64 - index;

  /* Transform as many times as possible.*/
  if (inputLen >= partLen)
  {
    memcpy(&context->buffer[index], input, partLen);
    MD5Transform (context->state, context->buffer);
    for (i = partLen; i + 63 < inputLen; i += 64)
      MD5Transform (context->state, &input[i]);
    index = 0;
  }
  else
    i = 0;

  /* Buffer remaining input */
  memcpy(&context->buffer[index], &input[i], inputLen - i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context. */
void MD5Final ( unsigned char digest[16], MD5_CTX *context )
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  memcpy(bits, &context->count, 8);

  /* Pad out to 56 mod 64.*/
  index = (unsigned int)((context->count >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update(context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update(context, bits, 8);

  /* Store state in digest */
  memcpy(digest, context->state, 16);

  /* Zeroize sensitive information. */
  memset(context, 0, sizeof(*context));
}

/* MD5 basic transformation. Transforms state based on block. From rfc1321. */
static void MD5Transform ( uinttype32 state[4], const unsigned char block[64] )
{
  uinttype32 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  memcpy(x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information. */
  memset(x, 0, sizeof(x));
}

ByteArray tamias::wtf::utilities::md5Sum( const ByteArray &source )
{
  MD5_CTX context;
  MD5Init(&context);
  MD5Update(&context, (unsigned char*)source.cString(), source.size());
  unsigned char digest[16];
  MD5Final(digest, &context);
  ByteArray result;
  static char hex[17] = "0123456789abcdef";
  for (sizetype i = 0; i < 16; i++)
  {
    result += hex[digest[i] >> 4];
    result += hex[digest[i] & 0xf];
  }
  return result;
}

inttype64 tamias::wtf::utilities::stringToInt( const String &source )
{
  if (source.length() == 0)
    return 0;
  inttype64 sign = source[0] == '-' ? -1 : +1;
  inttype64 result = 0;
  for (sizetype i = (source[0] == '-'); i < source.length(); i++)
    result = result * 10 + source[i] - '0';
  return result * sign;
}

bool tamias::wtf::utilities::isValidInt( String const &value )
{
  return testString(value, "0-9"); // TODO: regular expressions?
}

String tamias::wtf::utilities::intToString( inttype64 value, sizetype zeros )
{
  String result = "";
  if (value < 0)
    result += '-', value = -value;
  inttype64 divide = 1;
  while (value >= divide * 10 || zeros > 1)
    divide *= 10, zeros ? zeros-- : 0;
  while (divide > 0)
    result += '0' + value / divide % 10, divide /= 10;
  return result;
}

bool tamias::wtf::utilities::isAlphaNum( const String &source )
{
  for (sizetype i = 0; i < source.length(); i++)
    if (!((source[i] >= 'A' && source[i] <= 'Z') || (source[i] >= 'a' && source[i] <= 'z') ||
          (source[i] >= '0' && source[i] <= '9') || source[i] == '_'))
      return false;
  return true;
}

bool tamias::wtf::utilities::testString( const String &source, const String &pattern )
{ // TODO: optimize this using dynamic interval tree ^_~
  for (sizetype i = 0; i < source.length(); i++)
  {
    bool good = false;
    for (sizetype j = 0; j < pattern.length(); j++)
      if (pattern[j] == '\\')
      {
        j++;
        if (j < pattern.length() && source[i] == pattern[j])
          good = true, j = pattern.length();
      }
      else
      if (pattern[j] != '-' && pattern[j] == source[i])
        good = true, j = pattern.length();
      else if (pattern[j] == '-')
      {
        good = true;
        if (j > 0 && source[i] < pattern[j - 1])
          good = false;
        if (j < pattern.length() - 1 && source[i] > pattern[j + 1])
          good = false;
        if (good)
          j = pattern.length();
      }
    if (!good)
      return false;
  }
  return true;
}

