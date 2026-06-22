#pragma once
#include <string>
#include <unordered_map>

class ConfigManager {
   public:
    ConfigManager();

    static std::string get_string(const std::string& key);
    static bool get_bool(const std::string& key);
    static int get_int(const std::string& key);
    static float get_float(const std::string& key);

   private:
    static std::unordered_map<std::string, std::string> load_config_file(const std::string& filename);
    static std::unordered_map<std::string, std::string> load_config();
    static const std::unordered_map<std::string, std::string> config;
};
