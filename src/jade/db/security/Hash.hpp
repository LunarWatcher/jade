#pragma once

#include <optional>
#include <string>
#include <iomanip>
#include <sstream>

namespace jade::Hash {

struct HashResult {
    std::string hash;
    /**
     * Returns the salt used. Note that this only needs to be checked and stored if
     * the salt supplied to hash() is std::nullopt (which will be the case for new users,
     * or if a salt cycling is done for whatever reason).
     * If hash() is supplied a salt, that salt and the salt returned will always be the
     * same.
     */
    std::string salt;

    /**
     * Describes the hash version. This is used to deal with password upgrades
     * If versions mismatch with the version stored in the database, hashing upgrade
     * methods should be applied (as long as validVersion = true).
     *
     */
    int version;

    /**
     * Whether or not the version is valid, and a candidate for a hash upgrade.
     *
     * Note that this only applies to legacy hashing methods. For example, if the current version
     * is 6, but version 1 was requested, and version 1 used a hashing algorithm that has been
     * broken since its initial introduction, this is false and the login should be fully discarded.
     * However, if version 7, which is not and has never been implemented, hash() will throw instead.
     *
     * In other words, this is an indicator for whether or not the version can:
     *  1. Be used at all
     *  2. Be used to verify the user at some point in time, provided other criteria are met
     *
     * Note that the detail of version upgrades are not detailed here. There may be other factors
     * against an upgrade, but that's an implementation detail of the calling functions.
     *
     * Additionally, note that if validVersion == false, hash == salt == "" and version == 0.
     * This will additionally never be false when automatically determining the version. If the
     * latest hash algorithm is bad, that should be accompanied with an algorithm upgrade.
     * This being false when the version passed to hash() is std::nullopt indicates a severe
     * programmer error.
     */
    bool validVersion = true;
};


/**
 * Interface function for hashing. The hashing algorithm used is undefined 
 *
 * @param password      The password to hash
 * @param salt          The salt for the hash. If std::nullopt, a hash is generated.
 * @param version       Which hash revision to use. This is used for compatibility and upgrade purposes
 */
extern HashResult hash(
    const std::string& password,
    std::optional<std::string> salt,
    std::optional<int> version = std::nullopt
);

namespace _detail {

extern std::string pbkdf2_sha512(const std::string& pass, const std::string& salt, int permutations);

extern std::string generateSalt(size_t len = 24);

template <int Length>
inline std::string byteDecode(unsigned char digest[Length]) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    // TODO: This feels unsafe
    for (size_t i = 0; i < Length; ++i) {
        auto c = digest[i];
        ss << std::setw(2) << (int) c;
    }

    return ss.str();
}

}

}
