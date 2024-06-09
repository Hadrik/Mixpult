#include "OtherController.h"

bool OtherController::setVolume(float vol) {
  if (_mute) {
    setMute(false);
  }

  auto names = AudioController::getAllSessions();
  bool err = false;

  for (auto& name : names) {
    if (SliderController::_used.find(name) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (!AudioController::setSessionVolume(name, vol)) err = true;
  }
  return err;
}

bool OtherController::setMute(bool mute) {
  auto names = AudioController::getAllSessions();
  bool err = false;
  for (auto& name : names) {
    if (SliderController::_used.find(name) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (!AudioController::setSessionMute(name, mute)) err = true;
  }

  // Did anything get (un)muted? Sets _mute
  getMute();
  return err;
}

bool OtherController::getMute() {
  auto names = AudioController::getAllSessions();
  bool m_any = false;
  for (auto& name : names) {
    if (SliderController::_used.find(name) != SliderController::_used.end()) {
      continue; // This element is used
    }

    if (AudioController::getSessionMute(name)) m_any = true;
  }
  _mute = m_any;
  return m_any;
}
