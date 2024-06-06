#pragma once

#include "SliderController.h"

class OtherController : public SliderController {
public:
	OtherController() : SliderController("!other") {};

	bool setVolume(float vol);
	bool setMute(bool mute);
	bool getMute();
private:
	bool _mute;
};

