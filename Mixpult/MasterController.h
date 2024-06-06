#pragma once

#include "SliderController.h"

class MasterController : public SliderController {
public:
	MasterController() : SliderController("!master") {};

	bool setVolume(float vol);
	bool setMute(bool mute);
	bool getMute();
};

