#include "BasicSerial.h"

BasicSerial::BasicSerial() {
  _serial_handle = INVALID_HANDLE_VALUE;
}

BasicSerial::~BasicSerial() {
  if (_serial_handle == INVALID_HANDLE_VALUE) {
    return;
  }
  CloseHandle(_serial_handle);
}

bool BasicSerial::open(std::string port) {
retry:
  LOG(INFO) << "Opening serial connection";
  const std::string start = "\\\\.\\";
  if (port.rfind(start, 0) != 0) {
    port.insert(0, start);
  }

  _serial_handle = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

  if (_serial_handle == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      LOG(ERR) << "Serial port not found";
      MESSAGE_ERR_RETRY("Serial port not found!", retry);
    }
    LOG(ERR) << "Cannot open serial port";
    MESSAGE_ERR_RETRY("Cannot open serial port!", retry);
  }

  DCB dcb_params;
  dcb_params.DCBlength = sizeof(dcb_params);

  if (!GetCommState(_serial_handle, &dcb_params)) {
    LOG(ERR) << "Cannot load DCB params";
    MESSAGE_ERR_RETRY("Serial error!", retry);
  }

  dcb_params.BaudRate = 9600;
  // Arduino: SERIAL_8N1
  dcb_params.ByteSize = 8;
  dcb_params.Parity = 0;
  dcb_params.StopBits = 1;

  if (!GetCommState(_serial_handle, &dcb_params)) {
    LOG(ERR) << "Cannot set DCB params";
    MESSAGE_ERR_RETRY("Error setting serial parameters!", retry);
  }

  LOG(INFO) << "Serial port opened";
  return true;
}

bool BasicSerial::write(std::string data) {
  DWORD bytes_written;
  if (!WriteFile(_serial_handle, data.c_str(), strlen(data.c_str()), &bytes_written, NULL)) {
    LOG(ERR) << "Cannot write to serial port";
    std::cerr << "Error writing to serial port!" << std::endl;
    return false;
  }
  return true;
}

void BasicSerial::registerCommandCallback(std::string command, std::function<void(std::string)> cb) {
  _callbacks.insert({ command, std::vector({cb}) });
}

std::string BasicSerial::getLastCommand() {
  return _last_command;
}

// VERY hacky and slow, dont look :3
bool BasicSerial::update() {
  _timeouts.ReadTotalTimeoutConstant = 50;
  DWORD bytes_read;

  COMSTAT stats;
  ClearCommError(_serial_handle, NULL, &stats);
  SetCommTimeouts(_serial_handle, &_timeouts);
  const DWORD inQ = stats.cbInQue;

  // Read everything
  char* r = (char *)calloc(inQ + 1, sizeof(char));
  if (!ReadFile(_serial_handle, r, inQ, &bytes_read, NULL)) {
    free(r);
    return false;
  }
  if (bytes_read == 0) {
    free(r);
    return false;
  }

  if (r) {
    std::string str(r);
    str.insert(0, _unfinished); // Prepend last unfinished command
    _unfinished = "";
    // split to commands (delimiter "\r\n")
    std::string command;
    std::stringstream ss(str);
    while (std::getline(ss, command, '\n')) {
      if (command.length() < 1) {
        continue;
      }
      if (*(command.end() - 1) != '\r') {
        _unfinished = command;
        return false;
      }

      command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());

      auto it = _callbacks.find(command);
      if (it == _callbacks.end()) {
        // Command doesnt have registered callback
        _last_command = command;
      } else {
        const auto& vec = it->second;
        for (const auto& fn : vec) {
          fn(command);
        }
      }
    }

    free(r);
  }
  return true;
}

int BasicSerial::avaliable() {
  COMSTAT comm_status;
  ClearCommError(_serial_handle, NULL, &comm_status);
  return comm_status.cbInQue;
}

bool BasicSerial::readChar(char* c) {
  _timeouts.ReadTotalTimeoutConstant = 50; // ms
  if (!SetCommTimeouts(_serial_handle, &_timeouts)) {
    return false;
  }

  DWORD bytes_read = 0;
  if (!ReadFile(_serial_handle, c, 1, &bytes_read, NULL)) {
    return false;
  }

  if (bytes_read == 0) {
    std::cout << "Serial timeout" << std::endl;
    return false; // timeout
  }

  return true;
}
