#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#pragma once

#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <msclr/marshal_cppstd.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
//#include "SessionNotificationReciever.h"
#using <System.dll>

#include <iostream>

class AudioController {
public:
	static void init();
	static void exit();

	// Names
	static int getSessionCount();
	static std::string getPIDName(DWORD pid);
	static std::string getPIDTitle(DWORD pid);
	static std::set<std::string> getAllSessions();

	// Mute
	static bool setSessionMute(std::string name, bool mute);
	static bool setSessionMute(std::vector<std::string> names, bool mute);
	static bool setMasterMute(bool mute);
	static bool getSessionMute(std::string name);
	static bool getSessionMute(std::vector<std::string> names);
	static bool getMasterMute();

	// Volume
	static bool setSessionVolume(std::string name, float vol);
	static bool setSessionVolume(std::vector<std::string> names, float vol);
	static bool setMasterVolume(float vol);
	static float getSessionVolume(std::string name);
	static float getSessionVolume(std::vector<std::string> names);
	static float getMasterVolume();

private:
  static bool _getSessionControlByName(std::string name, std::vector<CComPtr<IAudioSessionControl>> &ascs);

	//static SessionNotificationReciever _notif_reciever;
	static CComPtr<IAudioEndpointVolume> _pEpVol;
	static CComPtr<IAudioSessionEnumerator> _pAudioSessionEnumerator;
	static HANDLE _hSerial;
};

#endif