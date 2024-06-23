#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

enum logLevel {
  INFO,
  WARN,
  ERR,
  NONE
};

class LOG {
public:
  LOG(logLevel l = INFO, bool headers = true);
  ~LOG();

  static void setPath(const std::string& path);
  static std::string getPath();
  static void setMinLevel(logLevel level);
  static void setMinLevel(std::string level);

  static std::string getDate();
  static std::string getTime();
  static std::string getTimeFmt(const std::string& format);

  template<class C>
  LOG& operator<<(const C& msg) {
    if (_level >= LOG::_min_level) {
      _out << msg;
    }
    return *this;
  };

private:
  static std::string _path;
  static logLevel _min_level;
  std::ofstream _out;
  logLevel _level;
  inline std::string _label(logLevel l);
};
