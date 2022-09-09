#include "ed25519.h"
#include "ed_sha512.h"
#include "ge.h"
#include "sc.h"

void ed25519_sign(unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *private_key) {
    sha512_context hash;
    unsigned char az[64];
    unsigned char hram[64];
    unsigned char r[64];
    ge_p3 R;

    ed_sha512_init(&hash);
    ed_sha512_update(&hash, private_key, 32);
    ed_sha512_final(&hash, az);
    az[0] &= 248;
    az[31] &= 127;
    az[31] |= 64;
    
    ed_sha512_init(&hash);
    ed_sha512_update(&hash, az + 32, 32);
    ed_sha512_update(&hash, message, message_len);
    ed_sha512_final(&hash, r);

    sc_reduce(r);
    ge_scalarmult_base(&R, r);
    ge_p3_tobytes(signature, &R);
    
    ed_sha512_init(&hash);
    ed_sha512_update(&hash, signature, 32);
    ed_sha512_update(&hash, private_key + 32, 32);
    ed_sha512_update(&hash, message, message_len);
    ed_sha512_final(&hash, hram);

    sc_reduce(hram);
    sc_muladd(signature + 32, hram, az, r);
}
