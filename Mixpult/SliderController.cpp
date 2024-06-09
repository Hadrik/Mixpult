#include "SliderController.h"

#define TRY(x) 	if (!x) {\
									refresh();\
									return x;\
								} else { return true; }

std::set<std::string> SliderController::_used = {};

SliderController::SliderController(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	_app_name = name;

	if (*(name.begin()) == '!') {
		return;
	}
	SliderController::_used.insert(name);
}

bool SliderController::setVolume(float vol) {
	if (getMute()) {
		setMute(false);
	}

	return AudioController::setSessionVolume(_app_name, vol);
}

bool SliderController::setMute(bool mute) {
	return AudioController::setSessionMute(_app_name, mute);
}

bool SliderController::getMute() {
	return AudioController::getSessionMute(_app_name);
}
