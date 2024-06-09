#include "BasicSerial.h"
#include "AudioController.h"
#include "SliderController.h"
#include "MasterController.h"
#include "OtherController.h"
#include "ConfigLoader.h"
#include <iostream>
#include <sstream>
#include <string>

std::vector<std::vector<SliderController*>> sliders;
BasicSerial s;

void data(std::string in) {
  // Split string to vector
  std::vector<std::string> d;
  std::stringstream ss(in);
  std::string item;
  while (std::getline(ss, item, '|')) {
    d.push_back(item);
  }

  // Assing volume to each controller
  for (size_t i = 0; i < std::min<size_t>(sliders.size(), d.size()); i++) {
    for (auto& slider : sliders[i]) {
      if (d[i] == "m") {
        slider->setMute(true);
      } else {
        int val;
        try {
          val = std::stoi(d[i]);
        } catch (...) {
          return;
        }
        slider->setVolume(val / 1024.0f);
      }
    }
  }
}

void respond(std::string) {
  s.write("resp");
}

bool setup(std::string config_path) {
  // Read config
  Config_t cfg = ConfigLoader::load(config_path);

  // Get all sessions
  auto names = AudioController::getAllSessions();
  std::cout << "Discovered streams:\n";
  for (const auto& name : names) {
    std::cout << name;
    std::cout << "\n";
  }

  // Set up controllers
  for (auto& slider : cfg.sliders) {
    std::vector<SliderController*> vsc;
    for (auto& app : slider) {
      if (app.rfind('!', 0) == 0) {
        if (app == "!master") {
          vsc.push_back(new MasterController());
        } else if (app == "!other") {
          vsc.push_back(new OtherController());
        }
      } else {
        vsc.push_back(new SliderController(app));
      }
    }
    sliders.push_back(vsc);
  }

  // Configure serial connection
  if (!s.open(cfg.port)) return false;
  s.registerCommandCallback("ping", respond);

  return true;
}

bool loop() {
  s.update();
  data(s.getLastCommand());
  return true;
}

int main(int argc, char const* argv[]) {
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
  if (!setup((argc == 2) ? std::string(argv[1]) : "./")) {
    std::cin.get();
    return 1;
  }
  while (loop()) {}
  std::cin.get();
  return 2;
}
