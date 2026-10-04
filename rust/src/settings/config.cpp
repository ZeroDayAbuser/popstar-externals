#include <pawjob_umbrella.hpp>
#include <settings/config.hpp>

std::vector<ConfigVarBase*>& GetConfigVars() {
    static std::vector<ConfigVarBase*> vars;
    return vars;
}

ConfigVarBase::ConfigVarBase(const std::string& category, const std::string& name)
    : category(category), name(name) {
    GetConfigVars().push_back(this);
}

ConfigVarBase::ConfigVarBase(const char* category, const char* name)
    : category(category), name(name) {
    GetConfigVars().push_back(this);
}
