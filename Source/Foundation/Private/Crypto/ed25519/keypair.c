#include <stdint.h>

#include "ed25519.h"
#include "ed_sha512.h"
#include "ge.h"

void ed25519_sha512(const unsigned char *message, size_t message_len, unsigned char *out)
{
	ed_sha512(message, message_len, out);
}

void ed25519_create_keypair(uint8_t *public_key, uint8_t *private_key, const uint8_t *seed)
{
    ge_p3 A;

	uint8_t hash[64];
    ed_sha512(seed, 32, hash);

	hash[0] &= 248;
    hash[31] &= 63;
    hash[31] |= 64;

    ge_scalarmult_base(&A, hash);
    ge_p3_tobytes(public_key, &A);

    for (int i = 0; i < 32; ++i)
    {
    	private_key[i] = seed[i];
	    private_key[32 + i] = public_key[i];
    }
}
