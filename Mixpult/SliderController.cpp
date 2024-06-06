#include "SliderController.h"

#define TRY(x) 	if (!x) {\
									refresh();\
									return x;\
								} else { return true; }

NameIDMap_t SliderController::_map = {};
std::set<std::string> SliderController::_used = {};

SliderController::SliderController(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	_app_name = name;

	if (*(name.begin()) == '!') {
		_app_ids = {};
		return;
	}

	auto it = _map.find(name);
	if (it == _map.end()) {
		_app_ids = {};
		return;
	}

	_app_ids = it->second;
	SliderController::_used.insert(name);
}

bool SliderController::setVolume(float vol) {
	if (getMute()) {
		setMute(false);
	}

	return AudioController::setSessionVolume(_app_ids, vol);
}

bool SliderController::setMute(bool mute) {
	return AudioController::setSessionMute(_app_ids, mute);
}

bool SliderController::getMute() {
	return AudioController::getSessionMute(_app_ids);
}

void SliderController::useMap(NameIDMap_t map) {
	SliderController::_map = map;
}

void SliderController::refreshMap() {
	SliderController::_map = AudioController::getNameIDMap();
}

bool SliderController::refresh(SliderController& c) {
	refreshMap();
	auto it = _map.find(c._app_name);
	if (it == _map.end()) {
		c._app_ids = {};
		return false;
	}
	c._app_ids = it->second;
	return true;
}
