#include "config_manager.h"
#include <fstream>

std::unordered_map<std::string, std::string> ConfigManager::load_config_file(const std::string &filename) {
    // Loading config file
    std::ifstream file{filename};
    if (!file.is_open()) {
        return {};
    }

    std::unordered_map<std::string, std::string> config;
    std::string line;

    // Inserting [key, value] pairs in config line by line
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        const size_t separator = line.find('=');
        if (separator == std::string::npos) continue;

        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);

        config[key] = value;
    }

    return config;
}

std::unordered_map<std::string, std::string> ConfigManager::load_config() {
    // Getting "config.default" and "config.local" configs
    std::unordered_map config{load_config_file("config.default")};
    const std::unordered_map local{load_config_file("config.local")};

    // Setting default values to local values (only for existing in "config.default" keys!)
    for (const auto &[key, value] : local) {
        if (config.contains(key)) {
            config[key] = value;
        }
    }

    return config;
}

const std::unordered_map<std::string, std::string> ConfigManager::config{load_config()};

ConfigManager::ConfigManager() = default;

std::string ConfigManager::get_string(const std::string &key) {
    const auto value{config.find(key)};
    if (value != config.end()) {
        return value->second;
    }
    throw std::runtime_error("Config key was not found: " + key);
}

bool ConfigManager::get_bool(const std::string &key) {
    const std::string value = get_string(key);
    return value == "true" || value == "1";
}

int ConfigManager::get_int(const std::string &key) { return std::stoi(get_string(key)); }

float ConfigManager::get_float(const std::string &key) { return std::stof(get_string(key)); }
