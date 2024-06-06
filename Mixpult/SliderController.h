#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "AudioController.h"

class SliderController {
public:
	SliderController(std::string name);

	virtual bool setVolume(float vol);
	virtual bool setMute(bool mute);
	virtual bool getMute();

	static void useMap(NameIDMap_t map);
	static void refreshMap();

protected:
	static bool refresh(SliderController& c);

	std::string _app_name;
	std::vector<SessionID_t> _app_ids;

	static NameIDMap_t _map;
	static std::set<std::string> _used;
};

