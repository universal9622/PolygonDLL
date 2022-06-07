#include "pch.h"

#include "Patches.h"
#include "Util.h"
#include "Hooks/Crypt.h"

Crypt::Crypt()
{
    if (!CryptAcquireContext(&context, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        if (::GetLastError() == NTE_BAD_KEYSET)
        {
            if (!CryptAcquireContext(&context, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET))
            {
                throw std::runtime_error("Error during CryptAcquireContext 2\n");
            }
        }
        else
        {
            throw std::runtime_error("Error during CryptAcquireContext\n");
        }
    }

    try
    {
#ifdef _DEBUG
        std::vector<BYTE> publicKey = Util::base64Decode(Util::publicKey);
#else
        std::vector<BYTE> publicKey = Util::publicKey;
#endif

        BYTE* blob = new BYTE[publicKey.size()];
        std::copy(publicKey.begin(), publicKey.end(), blob);

        if (!CryptImportKey(context, blob, publicKey.size(), 0, 0, &key))
        {
            throw std::runtime_error("");
        }
    }
    catch (...)
    {
#ifdef _DEBUG
        throw std::runtime_error("Failed to import public key");
#else
        throw std::runtime_error("Error during CryptImportKey");
#endif
    }
}

Crypt::~Crypt()
{
    CryptDestroyKey(key);
    CryptReleaseContext(context, 0);
}

bool Crypt::verifySignatureBase64(std::string message, std::string signatureBase64, ALG_ID algorithm = CALG_SHA_256)
{
    // Check for a reasonable signature length before verifying
    if (signatureBase64.length() > 4096)
    {
        return false;
    }

    HCRYPTHASH hash;

    if (!CryptCreateHash(context, algorithm, NULL, 0, &hash))
    {
        throw std::runtime_error("");
    }

    try
    {
        if (!CryptHashData(hash, (BYTE*)message.c_str(), message.size(), 0))
        {
            return false;
        }

        std::vector<BYTE> signature = Util::base64Decode(signatureBase64);

        /*
            The native cryptography API uses little-endian byte order
            while OpenSSL uses big-endian byte order.

            If you are verifying a signature generated by using a OpenSSL API
            (or similar), you must swap the order of signature bytes before
            calling the CryptVerifySignature function to verify the signature.
        */

        std::reverse(signature.begin(), signature.end());

        BYTE* signatureData = new BYTE[signature.size()];
        std::copy(signature.begin(), signature.end(), signatureData);

        if (!CryptVerifySignature(hash, signatureData, signature.size(), key, NULL, 0))
        {
            return false;
        }
    }
    catch (...)
    {
        ::CryptDestroyHash(hash);
        return false;
    }

    ::CryptDestroyHash(hash);

    return true;
}

Crypt__verifySignatureBase64_t Crypt__verifySignatureBase64 = (Crypt__verifySignatureBase64_t)ADDRESS_CRYPT__VERIFYSIGNATUREBASE64;

// Crypt::verifySignatureBase64(std::string message, std::string signatureBase64)
void __fastcall Crypt__verifySignatureBase64_hook(HCRYPTPROV* _this, void*, int a2, BYTE* pbData, int a4, int a5, int a6, DWORD dwDataLen, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15)
{
    /*
        Ideally, we would be able to just use the function signature as-is.
        However, it causes inexplicable crashes. Thus, we must reconstruct
        the strings by hand given the manual parameters.
    */

    std::string message;
    std::string signatureBase64;

    // Get message
    const BYTE* v18 = pbData;
    if ((unsigned int)a8 < 0x10)
    {
        v18 = (const BYTE*)&pbData;
    }

    message = std::string(reinterpret_cast<const char*>(pbData), dwDataLen);

    // Get signatureBase64
    int* v21 = (int*)a10;
    if ((unsigned int)a15 < 0x10)
    {
        v21 = &a10;
    }

    signatureBase64 = std::string(reinterpret_cast<const char*>(v21), a14);

    // Verify signature
    if (!Crypt().verifySignatureBase64(message, signatureBase64, CALG_SHA_256))
    {
        // backwards compatibility for sha1 signatures
        if (!Crypt().verifySignatureBase64(message, signatureBase64, CALG_SHA1))
        {
            throw std::runtime_error("");
        }
    }
}