#pragma once

#include "SliderController.h"
#include <vector>
#include <string>

class OtherController : public SliderController {
public:
	OtherController() : SliderController("!other") {};
	OtherController(const std::vector<std::string>& except);

	bool setVolume(float vol);
	bool setMute(bool mute);
	bool getMute();
private:
	bool _mute;
};

