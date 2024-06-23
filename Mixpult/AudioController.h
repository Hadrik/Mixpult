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
#include <queue>
#include "Logger.h"
#include "MessageBoxes.h"
#include "SessionNotificationReciever.h"
#include "SessionEventReciever.h"
#using <System.dll>

#include <iostream>

class AudioController {
public:
	static void init();
	static void exit();
	static void refresh();

	// Names
	static int getSessionCount();
	static std::set<std::string> getAllSessions();
	static std::string getPIDName(DWORD pid);

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

	static void logSessions();

private:
	static HRESULT _onNewSession(IAudioSessionControl* ac);
	static HRESULT _onStateChange(unsigned long id, AudioSessionState state);
	static HRESULT _onDisconnect(unsigned long id, AudioSessionDisconnectReason reason);
	static bool _createNewSession(CComPtr<IAudioSessionControl> asc);
	static bool _releaseSession(unsigned long id);

	struct Controls {
		bool valid;
		CComPtr<IAudioSessionControl> audioSessionControl;
		CComPtr<IAudioSessionControl2> audioSessionControl2;
		CComPtr<ISimpleAudioVolume> simpleAudioVolume;
		SessionEventReciever* eventReciever;
		unsigned long sessionID;
	};
	static std::map<std::string, std::vector<Controls>> _sessions;

	static SessionNotificationReciever _notif_reciever;
	static CComPtr<IAudioEndpointVolume> _pEpVol;
	static CComPtr<IAudioSessionEnumerator> _pAudioSessionEnumerator;
	static CComPtr<IAudioSessionManager2> _pAudioSessionManager2;
	static HANDLE _hSerial;
};

class AudioID {
public:
	static unsigned long get();
	static void release(unsigned long);

private:
	AudioID() = delete;
	~AudioID() = delete;

	static unsigned long _at;
	static std::queue<unsigned long> _unused;
};