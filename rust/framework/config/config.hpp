#pragma once
#include <string>
#include <vector>

namespace gui { class Listbox; }

namespace config {

void init();
void load();
void save();

void save_by_name(const std::string& name);
void load_by_name(const std::string& name);
void refresh_file_list(gui::Listbox* list);
void reset();

std::vector<std::string> list_files();
bool                     remove_by_name(const std::string& name);
std::string              directory();

}
