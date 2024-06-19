#include "ConfigLoader.h"

using json = nlohmann::json;

Config_t ConfigLoader::load(std::string path) {
retry:
  std::ifstream f(path);
  if (!f) {
    MESSAGE_ERR_RETRY("Cannot find configuration file!", retry);
  }

  json j;
  try {
    j = json::parse(f);
  } catch (...) {
    MESSAGE_ERR_RETRY("Bad configuration file formatting!", retry);
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
