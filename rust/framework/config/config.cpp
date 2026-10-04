#include "config.hpp"
#include "../app/app.hpp"
#include "../controls/checkbox/checkbox.hpp"
#include "../controls/color_picker/color_picker.hpp"
#include "../controls/dropdown/dropdown.hpp"
#include "../controls/listbox/listbox.hpp"
#include "../controls/multi_dropdown/multi_dropdown.hpp"
#include "../controls/slider/slider.hpp"
#include "../controls/text_input/text_input.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <windows.h>

namespace fs = std::filesystem;
using nlohmann::json;

namespace config {

    static std::string exe_dir() {
        char buf[MAX_PATH]{};
        ::GetModuleFileNameA(nullptr, buf, MAX_PATH);
        return fs::path(buf).parent_path().string();
    }

    std::string directory() {
        fs::path d = fs::path(exe_dir()) / "configs";
        std::error_code ec;
        fs::create_directories(d, ec);
        return d.string();
    }

    static fs::path path_for(const std::string& name) {
        return fs::path(directory()) / (name + ".json");
    }

    // why: walk the widget tree depth-first, calling `visit` with the
    // "/"-joined path for each named node. Unnamed (decorative) nodes
    // contribute nothing but their children still get walked. Subtrees
    // with should_save=false are skipped entirely so transient overlay
    // state never ends up on disk.
    static void walk(gui::Object* node, std::string path,
                     const std::function<void(gui::Object*, const std::string&)>& visit)
    {
        if (!node || !node->should_save) return;
        if (!node->m_name.empty()) {
            if (!path.empty()) path += '/';
            path += node->m_name;
            visit(node, path);
        }
        node->for_each_logical_child([&](gui::Object* child) {
            walk(child, path, visit);
        });
    }

    static void serialize(gui::Object* node, json& j, const std::string& key) {
        if (auto* c = dynamic_cast<gui::Checkbox*>(node))      j[key] = c->value;
        else if (auto* d = dynamic_cast<gui::Dropdown*>(node)) j[key] = d->value;
        else if (auto* m = dynamic_cast<gui::MultiDropdown*>(node)) j[key] = m->value;
        else if (auto* s = dynamic_cast<gui::Slider<int>*>(node))   j[key] = s->value;
        else if (auto* s = dynamic_cast<gui::Slider<float>*>(node)) j[key] = s->value;
        else if (auto* cp = dynamic_cast<gui::ColorPicker*>(node)) {
            j[key] = { {"r", cp->value.r}, {"g", cp->value.g},
                       {"b", cp->value.b}, {"a", cp->value.a} };
        }
        else if (auto* t = dynamic_cast<gui::TextInput*>(node)) j[key] = t->storage;
    }

    static void deserialize(gui::Object* node, const json& j, const std::string& key) {
        if (!j.contains(key)) return;
        const auto& v = j.at(key);
        try {
            if (auto* c = dynamic_cast<gui::Checkbox*>(node)) {
                if (v.is_boolean()) c->value = v.get<bool>();
            }
            else if (auto* d = dynamic_cast<gui::Dropdown*>(node)) {
                if (v.is_number_integer()) {
                    int val = v.get<int>();
                    const int n = static_cast<int>(d->options.size());
                    if (n > 0) {
                        if (val < 0) val = 0;
                        if (val >= n) val = n - 1;
                    }
                    d->value = val;
                }
            }
            else if (auto* m = dynamic_cast<gui::MultiDropdown*>(node)) {
                if (v.is_number_integer()) m->value = v.get<int>();
            }
            else if (auto* s = dynamic_cast<gui::Slider<int>*>(node)) {
                if (v.is_number()) s->value = v.get<int>();
            }
            else if (auto* s = dynamic_cast<gui::Slider<float>*>(node)) {
                if (v.is_number()) s->value = v.get<float>();
            }
            else if (auto* cp = dynamic_cast<gui::ColorPicker*>(node)) {
                if (v.is_object()) {
                    if (v.contains("r")) cp->value.r = v["r"].get<std::uint8_t>();
                    if (v.contains("g")) cp->value.g = v["g"].get<std::uint8_t>();
                    if (v.contains("b")) cp->value.b = v["b"].get<std::uint8_t>();
                    if (v.contains("a")) cp->value.a = v["a"].get<std::uint8_t>();
                }
            }
            else if (auto* t = dynamic_cast<gui::TextInput*>(node)) {
                if (v.is_string()) t->storage = v.get<std::string>();
            }
        } catch (...) {
            // type mismatch or out-of-range — keep current value
        }
    }

    void save_by_name(const std::string& name) {
        if (name.empty()) return;
        std::lock_guard lock(app::gui_mutex);

        // why: read-modify-write so unrelated existing keys survive (e.g.
        // user switched feature branches between save/load and half the
        // JSON is from the other branch's widget tree).
        json j;
        {
            std::ifstream in(path_for(name));
            if (in.good()) {
                try { in >> j; } catch (...) { j = json::object(); }
            }
            if (!j.is_object()) j = json::object();
        }

        for (auto& w : app::windows) {
            walk(w.get(), "", [&](gui::Object* node, const std::string& p) {
                serialize(node, j, p);
            });
        }

        std::ofstream out(path_for(name));
        if (!out.good()) return;
        out << j.dump(2);
    }

    // why: one-shot key migration for widgets that have been moved
    // between tabs. Reads each legacy key and copies its value to the
    // new key (only if the new key doesn't already exist, so user-set
    // values on the new path always win). Idempotent — safe to run
    // on every load. Old keys are left in place for backwards-compat.
    static void migrate_legacy_keys(json& j) {
        static const std::pair<const char*, const char*> kMoves[] = {
            // weapon modifications moved from Misc/General -> Aimbot/Weapons
            { "Pawjob/Misc/General/Automatic weapons",                        "Pawjob/Aimbot/Weapons/Automatic weapons" },
            { "Pawjob/Misc/General/Instant bow",                              "Pawjob/Aimbot/Weapons/Instant bow" },
            { "Pawjob/Misc/General/Instant eoka",                             "Pawjob/Aimbot/Weapons/Instant eoka" },
            { "Pawjob/Misc/General/Thick bullet",                             "Pawjob/Aimbot/Weapons/Thick bullet" },
            { "Pawjob/Misc/General/Rapid fire",                               "Pawjob/Aimbot/Weapons/Rapid fire" },
            { "Pawjob/Misc/General/No sway",                                  "Pawjob/Aimbot/Weapons/No sway" },
            { "Pawjob/Misc/General/Hitbox override",                          "Pawjob/Aimbot/Weapons/Hitbox override" },
            { "Pawjob/Misc/General/Hitbox override/Bone",                     "Pawjob/Aimbot/Weapons/Hitbox override/Bone" },
            { "Pawjob/Misc/General/Override weapon spread",                   "Pawjob/Aimbot/Weapons/Override weapon spread" },
            { "Pawjob/Misc/General/Override weapon spread/Options/Amount",    "Pawjob/Aimbot/Weapons/Override weapon spread/Options/Amount" },
            { "Pawjob/Misc/General/Override weapon recoil",                   "Pawjob/Aimbot/Weapons/Override weapon recoil" },
            { "Pawjob/Misc/General/Override weapon recoil/Options/Amount",    "Pawjob/Aimbot/Weapons/Override weapon recoil/Options/Amount" },
            { "Pawjob/Misc/General/Override melee range",                     "Pawjob/Aimbot/Weapons/Override melee range" },
            { "Pawjob/Misc/General/Override melee range/Options/Amount",      "Pawjob/Aimbot/Weapons/Override melee range/Options/Amount" },
            // Bullet tracers moved from Visuals/Local -> Aimbot/Visualization
            { "Pawjob/Visuals/Local/Bullet tracers",         "Pawjob/Aimbot/Visualization/Bullet tracers" },
            { "Pawjob/Visuals/Local/Bullet tracers/Color",   "Pawjob/Aimbot/Visualization/Bullet tracers/Color" },
        };
        for (const auto& [old_key, new_key] : kMoves) {
            if (j.contains(old_key) && !j.contains(new_key)) {
                j[new_key] = j[old_key];
            }
        }
    }

    void load_by_name(const std::string& name) {
        if (name.empty()) return;
        std::ifstream in(path_for(name));
        if (!in.good()) return;
        json j;
        try { in >> j; } catch (...) { return; }
        if (!j.is_object()) return;

        migrate_legacy_keys(j);

        std::lock_guard lock(app::gui_mutex);
        for (auto& w : app::windows) {
            walk(w.get(), "", [&](gui::Object* node, const std::string& p) {
                deserialize(node, j, p);
            });
        }
    }

    std::vector<std::string> list_files() {
        std::vector<std::string> out;
        std::error_code ec;
        for (auto& entry : fs::directory_iterator(directory(), ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            const auto& p = entry.path();
            if (p.extension() != ".json") continue;
            out.push_back(p.stem().string());
        }
        std::sort(out.begin(), out.end());
        return out;
    }

    bool remove_by_name(const std::string& name) {
        std::error_code ec;
        return fs::remove(path_for(name), ec);
    }

    void refresh_file_list(gui::Listbox* list) {
        if (!list) return;
        list->options.clear();
        for (auto& name : list_files()) list->options.push_back(name);
    }

    void init() {
        // Auto-load Default.json on startup if it exists; otherwise
        // create one from the current (default) widget state so the
        // configs folder isn't empty on first launch.
        auto files = list_files();
        if (std::find(files.begin(), files.end(), std::string("Default")) == files.end()) {
            save_by_name("Default");
        } else {
            load_by_name("Default");
        }
    }

    void load() { load_by_name("Default"); }
    void save() { save_by_name("Default"); }
    void reset() { /* no-op for now; widgets keep their c++ defaults */ }
}
