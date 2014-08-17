/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 **/

#include "md5.h"
#include <cstring>

/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z)          ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)          ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)          (((x) ^ (y)) ^ (z))
#define H2(x, y, z)         ((x) ^ ((y) ^ (z)))
#define I(x, y, z)          ((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
    (a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
    (*(uint32_t *)&ptr[(n) * 4])
#define GET(n) \
    SET(n)
#else
#define SET(n) \
    (m_block[(n)] = \
    (uint32_t)ptr[(n) * 4] | \
    ((uint32_t)ptr[(n) * 4 + 1] << 8) | \
    ((uint32_t)ptr[(n) * 4 + 2] << 16) | \
    ((uint32_t)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
    (m_block[(n)])
#endif

Md5::Md5()
: m_lo(0)
, m_hi(0)
, m_a(0x67452301)
, m_b(0xefcdab89)
, m_c(0x98badcfe)
, m_d(0x10325476)
{
}

void Md5::update(const uint8_t* data, uint64_t size)
{
    uint32_t saved_lo = m_lo;
    if ((m_lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
        m_hi++;
    m_hi += size >> 29;

    uint64_t used = saved_lo & 0x3f;

    if (used)
    {
        uint64_t available = 64 - used;

        if (size < available)
        {
            memcpy(&m_buffer[used], data, size);
            return;
        }

        memcpy(&m_buffer[used], data, available);
        data = (const uint8_t*) data + available;
        size -= available;
        body(m_buffer, 64);
    }

    if (size >= 64)
    {
        data = body(data, size & ~(uint64_t)0x3f);
        size &= 0x3f;
    }

    memcpy(m_buffer, data, size);
}

void Md5::finalize(uint8_t result[16])
{
    uint64_t used = m_lo & 0x3f;
    m_buffer[used++] = 0x80;
    uint64_t available = 64 - used;

    if (available < 8)
    {
        memset(&m_buffer[used], 0, available);
        body(m_buffer, 64);
        used = 0;
        available = 64;
    }

    memset(&m_buffer[used], 0, available - 8);

    m_lo <<= 3;
    m_buffer[56] = m_lo;
    m_buffer[57] = m_lo >> 8;
    m_buffer[58] = m_lo >> 16;
    m_buffer[59] = m_lo >> 24;
    m_buffer[60] = m_hi;
    m_buffer[61] = m_hi >> 8;
    m_buffer[62] = m_hi >> 16;
    m_buffer[63] = m_hi >> 24;

    body(m_buffer, 64);

    result[0] = m_a;
    result[1] = m_a >> 8;
    result[2] = m_a >> 16;
    result[3] = m_a >> 24;
    result[4] = m_b;
    result[5] = m_b >> 8;
    result[6] = m_b >> 16;
    result[7] = m_b >> 24;
    result[8] = m_c;
    result[9] = m_c >> 8;
    result[10] = m_c >> 16;
    result[11] = m_c >> 24;
    result[12] = m_d;
    result[13] = m_d >> 8;
    result[14] = m_d >> 16;
    result[15] = m_d >> 24;
}

std::string Md5::toString(const uint8_t data[16])
{
    char md5string[33];
    for(int i = 0; i < 16; ++i)
    {
        sprintf(&md5string[i*2], "%02x", data[i]);
    }

    return md5string;
}

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There are no alignment requirements.
 */
const uint8_t* Md5::body(const uint8_t* data, uint64_t size)
{
    const uint8_t* ptr = data;
    uint32_t saved_a, saved_b, saved_c, saved_d;

    uint32_t a = m_a;
    uint32_t b = m_b;
    uint32_t c = m_c;
    uint32_t d = m_d;

    do 
    {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

/* Round 1 */
        STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
        STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
        STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
        STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
        STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
        STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
        STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
        STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
        STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
        STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
        STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
        STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
        STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
        STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
        STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
        STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

/* Round 2 */
        STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
        STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
        STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
        STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
        STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
        STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
        STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
        STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
        STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
        STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
        STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
        STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
        STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
        STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
        STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
        STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

/* Round 3 */
        STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
        STEP(H2, d, a, b, c, GET(8), 0x8771f681, 11)
        STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
        STEP(H2, b, c, d, a, GET(14), 0xfde5380c, 23)
        STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
        STEP(H2, d, a, b, c, GET(4), 0x4bdecfa9, 11)
        STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
        STEP(H2, b, c, d, a, GET(10), 0xbebfbc70, 23)
        STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
        STEP(H2, d, a, b, c, GET(0), 0xeaa127fa, 11)
        STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
        STEP(H2, b, c, d, a, GET(6), 0x04881d05, 23)
        STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
        STEP(H2, d, a, b, c, GET(12), 0xe6db99e5, 11)
        STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
        STEP(H2, b, c, d, a, GET(2), 0xc4ac5665, 23)

/* Round 4 */
        STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
        STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
        STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
        STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
        STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
        STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
        STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
        STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
        STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
        STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
        STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
        STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
        STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
        STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
        STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
        STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while (size -= 64);

    m_a = a;
    m_b = b;
    m_c = c;
    m_d = d;

    return ptr;
}

