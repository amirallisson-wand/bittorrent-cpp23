#include "bittorrent/utils/crypto.hpp"
#include <openssl/sha.h>

namespace bittorrent::utils {

core::SHA1Hash sha1(std::string_view data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);

    core::SHA1Hash result;
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        result[i] = static_cast<std::byte>(hash[i]);
    }

    return result;
}

}  // namespace bittorrent::utils
