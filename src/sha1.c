/** @file
 *  SHA-1 hash.
 */

/*
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

-----------------
Modified 7/98
By James H. Brown <jbrown@burgoyne.com>
Still 100% Public Domain

Corrected a problem which generated improper hash values on 16 bit machines
Routine SHA1Update changed from
    void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int
len)
to
    void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned
long len)

The 'len' parameter was declared an int which works fine on 32 bit machines.
However, on 16 bit machines an int is too small for the shifts being done
against
it.  This caused the hash function to generate incorrect values if len was
greater than 8191 (8K - 1) due to the 'len << 3' on line 3 of SHA1Update().

Since the file IO in main() reads 16K at a time, any file 8K or larger would
be guaranteed to generate the wrong hash (e.g. Test Vector #3, a million
"a"s).

I also changed the declaration of variables i & j in SHA1Update to
unsigned long from unsigned int for the same reason.

These changes should make no difference to any 32 bit implementations since
an
int and a long are the same size in those environments.

--
I also corrected a few compiler warnings generated by Borland C.
1. Added #include <process.h> for exit() prototype
2. Removed unused variable 'j' in SHA1Final
3. Changed exit(0) to return(0) at end of main.

ALL changes I made can be located by searching for comments containing 'JHB'
-----------------
Modified 8/98
By Steve Reid <sreid@sea-to-sky.net>
Still 100% public domain

1- Removed #include <process.h> and used return() instead of exit()
2- Fixed overwriting of finalcount in SHA1Final() (discovered by Chris Hall)
3- Changed email address from steve@edmweb.com to sreid@sea-to-sky.net

-----------------
Modified 4/01
By Saul Kravitz <Saul.Kravitz@celera.com>
Still 100% PD
Modified to run on Compaq Alpha hardware.

-----------------
Modified 07/2002
By Ralph Giles <giles@artofcode.com>
Still 100% public domain
modified for use with stdint types, autoconf
code cleanup, removed attribution comments
switched SHA1Final() argument order for consistency
use SHA1_ prefix for public api
move public api to sha1.h
*/

#include <string.h>

#include "ostypes.h"
#include "sha1.h"

static uint32_t host_to_be(uint32_t i);
static void SHA1_Transform(uint32_t state[5], const uint8_t buffer[64]);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define blk0(i) (block->l[i] = host_to_be(block->l[i]))
#define blk(i)                                                                 \
    (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ \
                                block->l[(i + 2) & 15] ^ block->l[i & 15],     \
                            1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v, w, x, y, z, i)                                     \
    z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);
#define R1(v, w, x, y, z, i)                                    \
    z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);
#define R2(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
    w = rol(w, 30);
#define R3(v, w, x, y, z, i)                                          \
    z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
    w = rol(w, 30);
#define R4(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
    w = rol(w, 30);

static uint32_t host_to_be(uint32_t i)
{
#define le_to_be(i) ((rol((i), 24) & 0xFF00FF00) | (rol((i), 8) & 0x00FF00FF))
#if defined(__BIG_ENDIAN__) ||                                   \
    (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return i;
#elif defined(__LITTLE_ENDIAN__) ||                                 \
    (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return le_to_be(i);
#else /* fallback to run-time check */
    static const union {
        uint32_t u;
        unsigned char c;
    } check = {1};
    return check.c ? le_to_be(i) : i;
#endif
#undef le_to_be
}

static void *(*volatile SHA1_explicit_memset)(void *, int, size_t) = &memset;

static void SHA1_cleanse(void *p, size_t len)
{
    SHA1_explicit_memset(p, 0, len);
}

/* Hash a single 512-bit block. This is the core of the algorithm. */
static void SHA1_Transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t a, b, c, d, e;
    typedef union {
        uint8_t c[64];
        uint32_t l[16];
    } CHAR64LONG16;
    CHAR64LONG16 *block;

    static uint8_t workspace[64];
    block = (CHAR64LONG16 *)workspace;
    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    /* clang-format off */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* clang-format on */

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* Wipe variables */
    SHA1_cleanse(&a, sizeof(a));
    SHA1_cleanse(&b, sizeof(b));
    SHA1_cleanse(&c, sizeof(c));
    SHA1_cleanse(&d, sizeof(d));
    SHA1_cleanse(&e, sizeof(e));
}

/* SHA1Init - Initialize new context */
void crypto_SHA1_Init(SHA1_CTX *context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */
void crypto_SHA1_Update(SHA1_CTX *context, const uint8_t *data, size_t len)
{
    size_t i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += (uint32_t)len << 3) < ((uint32_t)len << 3))
        context->count[1]++;
    context->count[1] += (uint32_t)(len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1_Transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64) {
            SHA1_Transform(context->state, data + i);
        }
        j = 0;
    } else
        i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */
void crypto_SHA1_Final(SHA1_CTX *context, uint8_t *digest)
{
    uint32_t i;
    uint8_t finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >>
                                         ((3 - (i & 3)) * 8)) &
                                        255); /* Endian independent */
    }
    crypto_SHA1_Update(context, (uint8_t *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        crypto_SHA1_Update(context, (uint8_t *)"\0", 1);
    }
    /* Should cause a SHA1_Transform() */
    crypto_SHA1_Update(context, finalcount, 8);
    for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
        digest[i] =
            (uint8_t)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    /* Wipe variables */
    SHA1_cleanse(context->buffer, 64);
    SHA1_cleanse(context->state, 20);
    SHA1_cleanse(context->count, 8);
    SHA1_cleanse(finalcount, 8); /* SWR */

    SHA1_Transform(context->state, context->buffer);
}

void crypto_SHA1(const uint8_t *data, size_t len, uint8_t *digest)
{
    SHA1_CTX ctx;
    crypto_SHA1_Init(&ctx);
    crypto_SHA1_Update(&ctx, data, len);
    crypto_SHA1_Final(&ctx, digest);
}
