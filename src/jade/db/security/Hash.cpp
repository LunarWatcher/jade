#include "Hash.hpp"
#include "jade/security/Random.hpp"

#include <openssl/sha.h>
#include <optional>
#include <openssl/evp.h>

namespace jade {

Hash::HashResult Hash::hash(
    const std::string& password,
    std::optional<std::string> salt,
    std::optional<int> version
) {
    if (!salt) {
        salt = generateToken<34>();
    }
    // Set to the latest version when adding new versions
    // (And make sure I handled upgrades before and while doing so :) )
    if (!version) {
        version = 1;
    }

    std::string hash;

    if (version == 1) {
        // Note: any changes to the number of permutations counts as a new version
        // I doubt that will be a problem with pbkdf2 in particular, but this applies in general; if the parameters change
        // and the change affects the hash, it's a new version.
        // Note that 1M is massively overkill for the SHA512 variant (approximately 5 times higher than it needs to be),
        // so it's much more future-proof than it needs to be.
        hash = _detail::pbkdf2_sha512(password, *salt, 1'000'000);
    } else {
        throw std::runtime_error("Unknown password revision");
    }

    return HashResult { 
        hash,
        *salt,
        *version,
        true
    };
}

namespace Hash {

std::string _detail::pbkdf2_sha512(const std::string &pass, const std::string &salt, int iterations) {
    unsigned char digest[SHA512_DIGEST_LENGTH];
    PKCS5_PBKDF2_HMAC(
        pass.c_str(),
        (int) pass.size(),
        reinterpret_cast<const unsigned char*>(salt.c_str()),
        (int) salt.size(),
        iterations,
        EVP_sha512(),
        SHA512_DIGEST_LENGTH,
        digest
    );

    return byteDecode<SHA512_DIGEST_LENGTH>(digest);
}

}

}
