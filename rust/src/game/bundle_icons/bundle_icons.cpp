#include "bundle_icons.hpp"
#include <sdk/rust/unity_bundle/unity_bundle.hpp>

#include <atomic>
#include <memory>
#include <print>
#include <utils/debug.hpp>
#include <string>
#include <unordered_map>

namespace bundle_icons {

namespace {

const std::unordered_map<std::string, std::string>& mapping() {
    static const std::unordered_map<std::string, std::string> m = {
        { "Ores & Collectibles|Stone",     "stonenode"   },
        { "Ores & Collectibles|Sulfur",    "sulfurnode"  },
        { "Ores & Collectibles|Metal",     "metalnode"   },
        { "Ores & Collectibles|Hemp",      "hemp"        },
        { "Ores & Collectibles|Wood",      "woodcollectable" },
        { "Ores & Collectibles|Blueberry", "blueberry"   },
        { "Ores & Collectibles|Redberry",  "redberry"    },
        { "Ores & Collectibles|Greenberry","greenberry"  },
        { "Ores & Collectibles|Whiteberry","whiteberry"  },
        { "Ores & Collectibles|Mushroom",  "mushroom"    },
        { "Ores & Collectibles|Potato",    "potato"      },

        { "NPCs & Animals|Bear",        "bear"      },
        { "NPCs & Animals|Polar Bear",  "polarbear" },
        { "NPCs & Animals|Wolf",        "hog"       },
        { "NPCs & Animals|Stag",        "deer"      },
        { "NPCs & Animals|Boar",        "hog"       },
        { "NPCs & Animals|Chicken",     "chicken"   },
        { "NPCs & Animals|Horse",       "horse"     },

        { "Crates & Barrels|Elite crate",       "elitecrate"   },
        { "Crates & Barrels|Military crate",    "militarycrate" },
        { "Crates & Barrels|Locked crate",      "lockedcrate"  },
        { "Crates & Barrels|Air drop",          "airdrop"      },
        { "Crates & Barrels|Loot barrel",       "browncrate"   },
        { "Crates & Barrels|Oil barrel",        "browncrate"   },
        { "Crates & Barrels|Tool crate",        "toolcrate"    },
        { "Crates & Barrels|Small crate",       "browncrate"   },
        { "Crates & Barrels|Food crate",        "browncrate"   },
        { "Crates & Barrels|Small food crate",  "browncrate"   },
        { "Crates & Barrels|Health crate",      "medicalsyringe"},
        { "Crates & Barrels|Vehicle parts",     "browncrate"   },
        { "Crates & Barrels|Crate",             "browncrate"   },

        { "Vehicles|Minicopter",       "minicopter" },
        { "Vehicles|Scrap heli",       "scrapheli"  },
        { "Vehicles|Attack heli",      "scrapheli"  },
        { "Vehicles|RHIB",             "rhib"       },
        { "Vehicles|Rowboat",          "rowboat"    },
        { "Vehicles|Tugboat",          "rhib"       },
        { "Vehicles|Kyak",             "rowboat"    },
        { "Vehicles|Hot air balloon",  "hotairballoon" },

        { "Traps & Turrets|Auto turret",  "autoturret"  },
        { "Traps & Turrets|Shotgun trap", "shotguntrap" },
        { "Traps & Turrets|Flame turret", "shotguntrap" },

        { "Static & Monuments|SAM site",   "samsite"      },
        { "Static & Monuments|Cargo ship", "cargoship"    },
        { "Static & Monuments|Bradley",    "bradleyvehicle" },
    };
    return m;
}

const std::unordered_map<std::string, std::string>& shortname_mapping() {
    static const std::unordered_map<std::string, std::string> m = {
        { "rifle.ak",            "ak47"            },
        { "rifle.lr300",         "lr300"           },
        { "rifle.semiauto",      "sar"             },
        { "rifle.bolt",          "boltactionrifle" },
        { "rifle.m39",           "m39"             },
        { "rifle.l96",           "boltactionrifle" },
        { "rifle.m16a2",         "ak47"            },

        { "smg.mp5",             "mp5"             },
        { "smg.thompson",        "thompson"        },
        { "smg.2",               "customsmg"       },

        { "shotgun.pump",        "pumpshotgun"     },
        { "shotgun.double",      "doublebarrel"    },
        { "shotgun.spas12",      "spas12"          },
        { "shotgun.waterpipe",   "doublebarrel"    },

        { "pistol.m92",          "m92"             },
        { "pistol.python",       "python"          },
        { "pistol.semiauto",     "p250"            },
        { "pistol.revolver",     "revolver"        },
        { "pistol.eoka",         "eoka"            },

        { "lmg.m249",            "m249"            },

        { "bow.hunting",         "huntingbow"      },
        { "bow.compound",        "compoundbow"     },
        { "crossbow",            "crossbow"        },
    };
    return m;
}

std::unordered_map<std::string, std::unique_ptr<texture_t>>& cache() {
    static std::unordered_map<std::string, std::unique_ptr<texture_t>> c;
    return c;
}

std::atomic<bool> g_ready{ false };

}

void init() {
    auto& c = cache();
    int uploaded = 0;

    for (const auto& [_key, bundle_name] : mapping()) {
        if (c.count(bundle_name)) continue;
        auto* dec = unity_bundle::store::get(bundle_name);
        if (!dec) continue;
        auto tex = std::make_unique<texture_t>();
        if (!tex->load_from_rgba(dec->rgba.data(), dec->width, dec->height)) continue;
        c.emplace(bundle_name, std::move(tex));
        ++uploaded;
    }

    for (const auto& [_short, bundle_name] : shortname_mapping()) {
        if (c.count(bundle_name)) continue;
        auto* dec = unity_bundle::store::get(bundle_name);
        if (!dec) continue;
        auto tex = std::make_unique<texture_t>();
        if (!tex->load_from_rgba(dec->rgba.data(), dec->width, dec->height)) continue;
        c.emplace(bundle_name, std::move(tex));
        ++uploaded;
    }

    g_ready.store(true, std::memory_order_release);
    DBG("[bundle_icons] init: uploaded {} textures "
                 "({} category mappings, {} per-weapon mappings)",
                 uploaded, mapping().size(), shortname_mapping().size());
}

texture_t* for_category_type(std::string_view category, std::string_view type) {
    if (!g_ready.load(std::memory_order_acquire)) return nullptr;
    std::string key;
    key.reserve(category.size() + 1 + type.size());
    key.append(category);
    key.push_back('|');
    key.append(type);
    auto m_it = mapping().find(key);
    if (m_it == mapping().end()) return nullptr;
    auto c_it = cache().find(m_it->second);
    if (c_it == cache().end()) return nullptr;
    return c_it->second.get();
}

texture_t* for_item_shortname(std::string_view shortname) {
    if (!g_ready.load(std::memory_order_acquire)) return nullptr;
    if (shortname.empty()) return nullptr;
    auto m_it = shortname_mapping().find(std::string(shortname));
    if (m_it == shortname_mapping().end()) return nullptr;
    auto c_it = cache().find(m_it->second);
    if (c_it == cache().end()) return nullptr;
    return c_it->second.get();
}

bool   ready()        { return g_ready.load(std::memory_order_acquire); }
size_t loaded_count() { return cache().size(); }

}
