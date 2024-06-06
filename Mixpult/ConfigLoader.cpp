#include "ConfigLoader.h"

using json = nlohmann::json;

Config_t ConfigLoader::load(std::string path) {
  std::ifstream f(path);
  if (!f) {
    MessageBoxA(nullptr, "Cannot find configuration file!", "Mixpult", MB_OK | MB_ICONERROR);
    throw;
  }
  json j;
  try {
    j = json::parse(f);
  } catch (...) {
    MessageBoxA(nullptr, "Bad configuration file formatting!", "Mixpult", MB_OK | MB_ICONERROR);
    throw;
  }

  Config_t c;
  try {
    j.at("port").get_to(c.port);
  } catch (...) {
    MessageBoxA(nullptr, "Configuration file bad \"port\" option!", "Mixpult", MB_OK | MB_ICONERROR);
    throw;
  }

  try {
    auto& arr = j.at("sliders");
    for (auto& elem : arr) {
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
    MessageBoxA(nullptr, "Configuration file bad \"sliders\" option!", "Mixpult", MB_OK | MB_ICONERROR);
    throw;
  }

  return c;
}
