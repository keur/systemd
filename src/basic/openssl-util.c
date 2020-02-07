/* SPDX-License-Identifier: LGPL-2.1+ */

#if HAVE_OPENSSL

#include "openssl-util.h"
#include "macro.h"

DEFINE_TRIVIAL_CLEANUP_FUNC(HMAC_CTX*, HMAC_CTX_free);

int hmac(
        const EVP_MD *alg,
        const uint8_t *key,
        int key_len,
        const uint8_t *msg,
        int msg_len,
        uint8_t *md,
        unsigned int *md_len) {

        _cleanup_(HMAC_CTX_freep) HMAC_CTX *ctx = NULL;
        unsigned int len;
        int r;

        assert(alg);
        assert(key);
        assert(md);

        ctx = HMAC_CTX_new();
        if (!ctx)
                /* OpenSSL isn't giving us much help here, but this function
                 * only returns NULL on failed allocations. */
                return -ENOMEM;

        r = HMAC_Init_ex(ctx, key, key_len, alg, NULL);
        if (r == 0)
                return -EIO;

        r = HMAC_Update(ctx, msg, msg_len);
        if (r == 0)
                return -EIO;

        r = HMAC_Final(ctx, md, &len);
        if (r == 0)
                return -EIO;

        if (md_len)
                *md_len = len;

        return 0;
}

DEFINE_TRIVIAL_CLEANUP_FUNC(EVP_MD_CTX*, EVP_MD_CTX_destroy);

int sha(
        const EVP_MD *alg,
        const void *msg,
        size_t msg_len,
        uint8_t *md,
        unsigned int *md_len) {
        _cleanup_(EVP_MD_CTX_destroyp) EVP_MD_CTX *ctx = NULL;
        unsigned int len;
        int r;

        ctx = EVP_MD_CTX_new();
        if (!ctx)
                /* This function just calls OPENSSL_zalloc, so failure
                 * here is almost certainly a failed allocation. */
                return -ENOMEM;

        /* The documentation claims EVP_DigestInit behaves just like
         * EVP_DigestInit_ex if passed NULL, except it also calls
         * EVP_MD_CTX_reset, which deinitializes the context. */
        r = EVP_DigestInit_ex(ctx, alg, NULL);
        if (r == 0)
                return -EIO;

        r = EVP_DigestUpdate(ctx, msg, msg_len);
        if (r == 0)
                return -EIO;

        r = EVP_DigestFinal_ex(ctx, md, &len);
        if (r == 0)
                return -EIO;

        if (md_len)
                *md_len = len;

        return 0;
}

#endif
