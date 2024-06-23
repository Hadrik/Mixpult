#include "ConfigLoader.h"

using json = nlohmann::json;

Config_t ConfigLoader::load(std::string path) {
retry:
  LOG(INFO) << "Loading config";
  std::ifstream f(path);
  if (!f) {
    LOG(ERR) << "Config file not found (" << path << ")";
    std::string err = "Cannot find configuration file!\n";
    err.append(path);
    MESSAGE_ERR_RETRY(err.c_str(), retry);
  }

  json j;
  if (ConfigLoader::_endsWith(path, "json")) {
    // regular json
    LOG(INFO) << "Using JSON";
    try {
      j = json::parse(f);
    } catch (json::parse_error& e) {
      std::string err = "Bad configuration file formatting!\n";
      if (e.id == 101) err.append("If you're trying to use comments change file extension from json to jsonc\n");
      err.append("\n");
      err.append(e.what());
      MESSAGE_ERR_RETRY(err.c_str(), retry);
    }
  } else if (ConfigLoader::_endsWith(path, "jsonc")) {
    // json with comments
    LOG(INFO) << "Using JSONC";
    try {
      j = json::parse(f, nullptr, true, true);
    } catch (json::parse_error& e) {
      std::string err = "Bad configuration file formatting!\n\n";
      err.append(e.what());
      MESSAGE_ERR_RETRY(err.c_str(), retry);
    }
  } else {
    LOG(ERR) << "Config file isnt JSON or JSONC (" << path << ")";
    MESSAGE_ERR_RETRY("Bad configuration file extension!\nAllowed only json or jsonc", retry);
  }



  Config_t c;
  try {
    j.at("log_level").get_to(c.log_level);
  } catch (...) {
    LOG(WARN) << "Cannot find \"log_level\" in config file, using default (INFO)";
    c.log_level = "INFO";
  }

  try {
    j.at("port").get_to(c.port);
  } catch (...) {
    LOG(ERR) << "Config file doesnt contain \"port\"";
    MESSAGE_ERR_RETRY("Configuration file bad \"port\" option!", retry);
  }

  try {
    const auto& arr = j.at("sliders");
    for (const auto& elem : arr) {
      if (elem.is_string()) {
        std::string str;
        elem.get_to(str);
        c.sliders.push_back(std::vector<std::string>({ str }));
      }
      else if (elem.is_array()) {
        std::vector<std::string> vec;
        elem.get_to(vec);
        c.sliders.push_back(vec);
      }
      else {
        throw;
      }
    }
  }
  catch (...) {
    LOG(ERR) << "Config file doesnt contain\"sliders\"";
    MESSAGE_ERR_RETRY("Configuration file bad \"sliders\" option!", retry);
  }

  LOG(INFO) << "Config loaded";
  return c;
}

bool ConfigLoader::_endsWith(const std::string& main, const std::string& compare) {
  if (main.size() >= compare.size() &&
    main.compare(main.size() - compare.size(), compare.size(), compare) == 0)
    return true;
  else
    return false;
}
