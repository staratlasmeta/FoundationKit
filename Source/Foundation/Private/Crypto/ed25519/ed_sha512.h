#ifndef SHA512_H
#define SHA512_H

#include <stddef.h>

#include "fixedint.h"

/* state */
typedef struct sha512_context_ {
    uint64_t  length, state[8];
    size_t curlen;
    unsigned char buf[128];
} sha512_context;


int ed_sha512_init(sha512_context * md);
int ed_sha512_final(sha512_context * md, unsigned char *out);
int ed_sha512_update(sha512_context * md, const unsigned char *in, size_t inlen);
int ed_sha512(const unsigned char *message, size_t message_len, unsigned char *out);

#endif
