#pragma once

#include <string_view>
#include "bittorrent/core/types.hpp"

namespace bittorrent::utils {

core::SHA1Hash sha1(std::string_view data);

}  // namespace bittorrent::utils
