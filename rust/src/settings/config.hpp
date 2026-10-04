#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <menu/framework/color.hpp>
#include <menu/framework/widgets/keybind/keybind.hpp>
#include <glm/vec2.hpp>
#include <string_encryption.hpp>


struct ConfigVarBase {
    std::string category;
    std::string name;

    ConfigVarBase(const std::string& category, const std::string& name);
    ConfigVarBase(const char* category, const char* name);
    virtual ~ConfigVarBase() = default;

    virtual void Save(nlohmann::json& j) const = 0;
    virtual void Load(const nlohmann::json& j) = 0;
};

std::vector<ConfigVarBase*>& GetConfigVars();

template <typename T>
struct ConfigVar : ConfigVarBase {
    T value;

    ConfigVar(const std::string& category, const std::string& name, T defaultValue)
        : ConfigVarBase(category, name), value(defaultValue) {}
    
    ConfigVar(const char* category, const char* name, T defaultValue)
        : ConfigVarBase(category, name), value(defaultValue) {}

    void Save(nlohmann::json& j) const override {
        j[category][name] = value;
    }

    void Load(const nlohmann::json& j) override {
        if (j.contains(category) && j[category].contains(name) && !j[category][name].is_null()) {
            value = j[category][name].get<T>();
        }
    }

    operator T() const { return value; }
    ConfigVar& operator=(const T& val) { value = val; return *this; }

    T* operator&() { return &value; }
    const T* operator&() const { return &value; }
    
    T* operator->() { return &value; }
    const T* operator->() const { return &value; }
};

template <typename T>
struct ConfigRef : ConfigVarBase {
    T* ptr;

    ConfigRef(const std::string& category, const std::string& name, T* ptr)
        : ConfigVarBase(category, name), ptr(ptr) {}

    ConfigRef(const char* category, const char* name, T* ptr)
        : ConfigVarBase(category, name), ptr(ptr) {}

    void Save(nlohmann::json& j) const override {
        if(ptr) j[category][name] = *ptr;
    }

    void Load(const nlohmann::json& j) override {
        if (ptr && j.contains(category) && j[category].contains(name) && !j[category][name].is_null()) {
            *ptr = j[category][name].get<T>();
        }
    }
};

inline void to_json(nlohmann::json& j, const Color& c) {
    j = nlohmann::json{
        {X("r"), c.r}, {X("g"), c.g}, {X("b"), c.b}, {X("a"), c.a}
    };
}

inline void from_json(const nlohmann::json& j, Color& c) {
    if(j.contains(X("r"))) j.at(X("r")).get_to(c.r);
    if(j.contains(X("g"))) j.at(X("g")).get_to(c.g);
    if(j.contains(X("b"))) j.at(X("b")).get_to(c.b);
    if(j.contains(X("a"))) j.at(X("a")).get_to(c.a);
}

inline void to_json(nlohmann::json& j, const Vector2& v) {
    j = nlohmann::json{
        {X("x"), v.x}, {X("y"), v.y}
    };
}

inline void from_json(const nlohmann::json& j, Vector2& v) {
    if(j.contains(X("x"))) j.at(X("x")).get_to(v.x);
    if(j.contains(X("y"))) j.at(X("y")).get_to(v.y);
}

inline void to_json(nlohmann::json& j, const Keybind& k) {
    j = nlohmann::json{
        {X("key"), k.key}, {X("mode"), k.mode}
    };
}

inline void from_json(const nlohmann::json& j, Keybind& k) {
    if(j.contains(X("key"))) j.at(X("key")).get_to(k.key);
    if(j.contains(X("mode"))) j.at(X("mode")).get_to(k.mode);
}

