#include "ConfigLoader.h"

using json = nlohmann::json;

Config_t ConfigLoader::load(std::string path) {
retry:
  std::ifstream f(path);
  if (!f) {
    std::string err = "Cannot find configuration file!\n";
    err.append(path);
    MESSAGE_ERR_RETRY(err.c_str(), retry);
  }

  json j;
  if (ConfigLoader::_endsWith(path, "json")) {
    // regular json
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
    try {
      j = json::parse(f, nullptr, true, true);
    } catch (json::parse_error& e) {
      std::string err = "Bad configuration file formatting!\n\n";
      err.append(e.what());
      MESSAGE_ERR_RETRY(err.c_str(), retry);
    }
  } else {
    MESSAGE_ERR_RETRY("Bad configuration file extension!\nAllowed only json or jsonc", retry);
  }

  

  Config_t c;
  try {
    j.at("port").get_to(c.port);
  } catch (...) {
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
    MESSAGE_ERR_RETRY("Configuration file bad \"sliders\" option!", retry);
  }

  return c;
}

bool ConfigLoader::_endsWith(const std::string& main, const std::string& compare) {
  if (main.size() >= compare.size() &&
    main.compare(main.size() - compare.size(), compare.size(), compare) == 0)
    return true;
  else
    return false;
}
