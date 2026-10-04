#pragma once
#include <render/texture/texture.hpp>
#include <string_view>

namespace bundle_icons {

void           init();
texture_t*     for_category_type(std::string_view category, std::string_view type);

texture_t*     for_item_shortname(std::string_view shortname);

bool           ready();

size_t         loaded_count();

}
