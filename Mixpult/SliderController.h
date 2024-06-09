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

protected:
	std::string _app_name;

	static std::set<std::string> _used;
};

