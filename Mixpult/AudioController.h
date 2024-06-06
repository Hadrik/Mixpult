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
#using <System.dll>

#include <iostream>

using SessionID_t = int;
using NameIDMap_t = std::map<std::string, std::vector<SessionID_t>>;

class AudioController {
public:
	static void init();
	static void exit();

	// Names
	static int getSessionCount();
	static std::string getSessionName(SessionID_t id);
	static std::string getSessionTitle(SessionID_t id);
	static DWORD getSessionPID(SessionID_t id);
	static std::string getPIDName(DWORD pid);
	static std::string getPIDTitle(DWORD pid);
	static NameIDMap_t getNameIDMap();

	// Mute
	static bool setSessionMute(SessionID_t id, bool mute);
	static bool setSessionMute(std::vector<SessionID_t> ids, bool mute);
	static bool setMasterMute(bool mute);
	static bool getSessionMute(SessionID_t id);
	static bool getSessionMute(std::vector<SessionID_t> ids);
	static bool getMasterMute();

	// Volume
	static bool setSessionVolume(SessionID_t id, float vol);
	static bool setSessionVolume(std::vector<SessionID_t> ids, float vol);
	static bool setMasterVolume(float vol);
	static float getSessionVolume(SessionID_t id);
	static float getSessionVolume(std::vector<SessionID_t> ids);
	static float getMasterVolume();

private:
	static CComPtr<IAudioEndpointVolume> _pEpVol;
	static CComPtr<IAudioSessionEnumerator> _pAudioSessionEnumerator;
	static HANDLE _hSerial;
};

#endif