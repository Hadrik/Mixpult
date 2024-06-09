#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "MessageBoxes.h"

struct Config_t {
  std::string port;
  std::vector<std::vector<std::string>> sliders;
};

class ConfigLoader {
public:
  static Config_t load(std::string path);
};

