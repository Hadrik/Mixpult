#ifndef BASICSERIAL_H
#define BASICSERIAL_H

#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

class BasicSerial {
public:
  BasicSerial();
  ~BasicSerial();

  bool open(std::string port);
  bool write(std::string data);
  void registerCommandCallback(std::string command, std::function<void(std::string)> cb);
  std::string getLastCommand();
  bool update();
  int avaliable();

private:
  HANDLE _serial_handle;
  COMMTIMEOUTS _timeouts;
  std::map<std::string, std::vector< std::function<void(std::string)>>> _callbacks;
  std::string _last_command;
  std::string _unfinished;

  bool readChar(char*);
};

#endif