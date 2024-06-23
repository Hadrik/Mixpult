#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <string_view>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Logger.h"
#include "MessageBoxes.h"

struct Config_t {
  std::vector<std::vector<std::string>> sliders;
  std::string port;
  std::string log_level;
};

class ConfigLoader {
public:
  static Config_t load(std::string path);

private:
  static bool _endsWith(const std::string& main, const std::string& compare);
};

