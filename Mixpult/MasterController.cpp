#include "MasterController.h"

bool MasterController::setVolume(float vol) {
  if (getMute()) {
    setMute(false);
  }
  return AudioController::setMasterVolume(vol);
}

bool MasterController::setMute(bool mute) {
  return AudioController::setMasterMute(mute);
}

bool MasterController::getMute() {
  return AudioController::getMasterMute();
}
