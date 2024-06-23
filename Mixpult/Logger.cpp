#include "Logger.h"

std::string LOG::_path = ".\\log.txt";
logLevel LOG::_min_level = NONE;

LOG::LOG(logLevel l, bool headers) : _level(l) {
  _out.open(LOG::_path, std::ios_base::app);
  if (headers) {
    _out << LOG::getTime() << " " << _label(l) << ": ";
  }
}

LOG::~LOG() {
  _out << "\n";
  _out.close();
}

void LOG::setPath(const std::string& path) {
  LOG::_path = path;
}

std::string LOG::getPath() {
  return LOG::_path;
}

void LOG::setMinLevel(logLevel level) {
  LOG::_min_level = level;
}

void LOG::setMinLevel(std::string level) {
  std::transform(level.begin(), level.end(), level.begin(), ::toupper);
  if (level == "INFO") setMinLevel(INFO);
  else if (level == "WARNING") setMinLevel(WARN);
  else if (level == "ERROR") setMinLevel(ERR);
  else setMinLevel(NONE);
}

std::string LOG::getDate() {
  return LOG::getTimeFmt("%F");
}

std::string LOG::getTime() {
  return LOG::getTimeFmt("%X");
}

std::string LOG::getTimeFmt(const std::string& format) {
  const auto chnow = std::chrono::system_clock::now();
  const auto now = std::chrono::system_clock::to_time_t(chnow);
  std::stringstream ss;
#pragma warning(disable:4996)
  ss << std::put_time(std::localtime(&now), format.c_str());
#pragma warning(default:4996)
  return ss.str();
}

inline std::string LOG::_label(logLevel l) {
  std::string n;
  switch (l) {
  case INFO: n = "[INFO]    "; break;
  case WARN: n = "[WARNING] "; break;
  case ERR:  n = "[ERROR]   "; break;
  default:   n = "";
  }
  return n;
}
