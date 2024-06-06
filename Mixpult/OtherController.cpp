#include "OtherController.h"

bool OtherController::setVolume(float vol) {
  if (_mute) {
    setMute(false);
  }
  bool err = false;
  for (auto& line : SliderController::_map) {
    if (SliderController::_used.find(line.first) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (!AudioController::setSessionVolume(line.second, vol)) err = true;;
  }
  return err;
}

bool OtherController::setMute(bool mute) {
  bool err = false;
  for (auto& line : SliderController::_map) {
    if (SliderController::_used.find(line.first) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (!AudioController::setSessionMute(line.second, mute)) err = true;
  }

  // Did anything get (un)muted? Sets _mute
  getMute();
  return err;
}

bool OtherController::getMute() {
  bool m_any = false;
  for (auto& line : SliderController::_map) {
    if (SliderController::_used.find(line.first) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (AudioController::getSessionMute(line.second)) m_any = true;
  }
  _mute = m_any;
  return m_any;
}
