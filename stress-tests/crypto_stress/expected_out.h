
#ifndef TINY_DATA
#define TINY_DATA   1
#endif // TINY_DATA


#if TINY_DATA

#if TEST_AES

// input : zero
// Obs: CTR e CBC são iguais para o 1o bloco de 128 bits com input 0;
uint32_t aes_expect[2][3][4] = {
    // CTR Mode
    {
        {0xd126ed98, 0x34ddbfe6, 0x5acd80d2, 0x63192786},   // 256 bit key
        {0x85122e99, 0x1fa902ad, 0x68b1b310, 0xe9e3d7f9},   // 192 bit key
        {0xb7a3d5cd, 0xe587dd06, 0x01bbbbf8, 0xbefcbb71},   // 128 bit key
    },
    // CBC Mode
    {
        {0x0f0c6287, 0x556d5f6a, 0x80dfaafd, 0x218301ab},   // 256 bit key
        {0xd05a569b, 0x20c9bc9b, 0x2ab0593f, 0x67a80751},   // 192 bit key
        {0x0f2d94c8, 0x59515c6c, 0xf8fc4aa5, 0xa3a56556},   // 128 bit key
    }
};

#endif // TEST_AES

#if TEST_SHA256

// input : "Hello" (0x48656c6c6f)
uint32_t sha256_expect[8] = {
    0x185f8db3,
    0x2271fe25,
    0xf561a6fc,
    0x938b2e26,
    0x4306ec30,
    0x4eda5180,
    0x07d17648,
    0x26381969,
};

#endif // TEST_SHA256

#if TEST_SHA3

// input : "Hello" (0x48656c6c6f)
uint32_t sha3_expect[16] = {
    0x0b8a44ac,
    0x991e2b26,
    0x3e8623cf,
    0xbeefc1cf,
    0xfe8c1c0d,
    0xe57b3e2b,
    0xf1673b4f,
    0x35e660e8,
    0x9abd18af,
    0xb7ac93cf,
    0x215eba36,
    0xdd1af676,
    0x98d6c9ca,
    0x3fdaaf73,
    0x4ffc4bd5,
    0xa8e34627,
};

#endif // TEST_SHA3

#else   // TINY_DATA

#endif  // TINY_DATA
