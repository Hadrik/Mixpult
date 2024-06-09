#include "AudioController.h"

using namespace System;
using namespace System::Diagnostics;

CComPtr<IAudioEndpointVolume> AudioController::_pEpVol = NULL;
CComPtr<IAudioSessionEnumerator> AudioController::_pAudioSessionEnumerator = NULL;
SessionNotificationReciever AudioController::_notif_reciever(AudioController::_onNewSession);
std::map<std::string, std::vector<AudioController::Controls>> AudioController::_sessions = {};

void AudioController::init() {
	CComPtr<IMMDevice> pDevice;
	CComPtr<IMMDeviceEnumerator> pDeviceEnumerator;
	CComPtr<IAudioSessionManager2> pSessionManager2;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	pDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
	pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager2);
	pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&AudioController::_pEpVol);
	pSessionManager2->GetSessionEnumerator(&AudioController::_pAudioSessionEnumerator);
	pDevice.Release();
	pDeviceEnumerator.Release();
	pSessionManager2.Release();

	int sessionCount = 0;
	AudioController::_pAudioSessionEnumerator->GetCount(&sessionCount);
	for (int i = 0; i < sessionCount; i++) {
		CComPtr<IAudioSessionControl> s;
		AudioController::_pAudioSessionEnumerator->GetSession(i, &s);
		AudioController::_createNewSession(s);
		s.Release();
	}
}

void AudioController::exit() {
	for (auto& [name, vec] : AudioController::_sessions) {
		for (auto& cont : vec) {
			cont.audioSessionControl.Release();
			cont.audioSessionControl2.Release();
			cont.simpleAudioVolume.Release();
		}
	}
	AudioController::_pAudioSessionEnumerator.Release();
	AudioController::_pEpVol.Release();
	CoUninitialize();
}

int AudioController::getSessionCount() {
	int sessionCount = 0;
	AudioController::exit();
	AudioController::init();
	AudioController::_pAudioSessionEnumerator->GetCount(&sessionCount);
	return sessionCount;
}

// Lowercase
std::string AudioController::getPIDName(DWORD pid) {
	String^ procname;

	try {
		Process^ proc = Process::GetProcessById(pid);
		procname = proc->ProcessName;
	}
	catch (Exception^ e) {
		e;
		return NULL;
	}
	std::string name = msclr::interop::marshal_as<std::string>(procname);
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	return name;
}

std::string AudioController::getPIDTitle(DWORD pid) {
	String^ title;

	try {
		Process^ proc = Process::GetProcessById(pid);
		title = proc->MainWindowTitle;
	}
	catch (Exception^ e) {
		e;
		return NULL;
	}
	return msclr::interop::marshal_as<std::string>(title);
}

std::set<std::string> AudioController::getAllSessions() {
	std::set<std::string> names;

	for (const auto& [name, _] : AudioController::_sessions) {
		names.insert(name);
	}
	return names;
}

bool AudioController::setSessionMute(std::string name, bool mute) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	auto it = AudioController::_sessions.find(name);
	if (it == AudioController::_sessions.end()) {
		return false;
	}

	bool res = false;
	for (auto& c : (*it).second) {
		HRESULT r;
		r = c.simpleAudioVolume->SetMute(mute, NULL);
		res |= (r == S_OK);
	}

	return res;
}

bool AudioController::setSessionMute(std::vector<std::string> names, bool mute) {
	bool ok = true;
	for (const auto& name : names) {
		ok &= AudioController::setSessionMute(name, mute);
	}
	return ok;
}

bool AudioController::setMasterMute(bool mute) {
	return S_OK == AudioController::_pEpVol->SetMute(mute, NULL);
}

bool AudioController::getSessionMute(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	auto it = AudioController::_sessions.find(name);
	if (it == AudioController::_sessions.end()) {
		return false;
	}

	bool res = false;
	for (auto& c : (*it).second) {
		BOOL m;
		c.simpleAudioVolume->GetMute(&m);
		res |= (m == TRUE);
	}

	return res;
}

bool AudioController::getSessionMute(std::vector<std::string> names) {
	bool mute = false;
	for (const auto& name : names) {
		mute |= AudioController::getSessionMute(name);
	}
	return mute;
}

bool AudioController::getMasterMute() {
	BOOL mute;
	AudioController::_pEpVol->GetMute(&mute);
	return mute != 0;
}

bool AudioController::setSessionVolume(std::string name, float vol) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	auto it = AudioController::_sessions.find(name);
	if (it == AudioController::_sessions.end()) {
		return false;
	}

	bool res = false;
	for (auto& c : (*it).second) {
		HRESULT r = c.simpleAudioVolume->SetMasterVolume(vol, NULL);
		res |= (r == S_OK);
	}

	return res;
}

bool AudioController::setSessionVolume(std::vector<std::string> names, float vol) {
	bool ok = true;
	for (const auto& name : names) {
		ok &= AudioController::setSessionVolume(name, vol);
	}
	return ok;
}

bool AudioController::setMasterVolume(float vol) {
	return S_OK == AudioController::_pEpVol->SetMasterVolumeLevelScalar(vol, NULL);
}

float AudioController::getSessionVolume(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	auto it = AudioController::_sessions.find(name);
	if (it == AudioController::_sessions.end()) {
		return -1;
	}

	float maxvol = -1;
	for (auto& c : (*it).second) {
		float vol;
		if (FAILED(c.simpleAudioVolume->GetMasterVolume(&vol))) {
			continue;
		}
		maxvol = std::max<float>(vol, maxvol);
	}

	return maxvol;
}

float AudioController::getSessionVolume(std::vector<std::string> names) {
	float highest = 0.0;
	for (const auto& name : names) {
		highest = std::max<float>(AudioController::getSessionVolume(name), highest);
	}
	return highest;
}

float AudioController::getMasterVolume() {
	float vol;
	AudioController::_pEpVol->GetMasterVolumeLevelScalar(&vol);
	return vol;
}

bool AudioController::_createNewSession(CComPtr<IAudioSessionControl> asc) {
	CComPtr<IAudioSessionControl2> as2;
	CComPtr<ISimpleAudioVolume> sav;

	// Set controls
	Controls c;
	c.audioSessionControl = asc;

	if (FAILED(asc->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&as2))) {
		asc.Release();
		return false;
	}
	c.audioSessionControl2 = as2;

	if (FAILED(asc->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&sav))) {
		asc.Release();
		as2.Release();
		return false;
	}
	c.simpleAudioVolume = sav;

	// Get process name
	DWORD pid;
	if (FAILED(as2->GetProcessId(&pid))) {
		asc.Release();
		as2.Release();
		sav.Release();
		return false;
	}
	std::string name = AudioController::getPIDName(pid);

	// Save
	auto it = AudioController::_sessions.find(name);
	if (it == AudioController::_sessions.end()) {
		AudioController::_sessions.emplace(name, std::vector<Controls>{ c });
	} else {
		(*it).second.push_back(c);
	}
	return true;
}

HRESULT AudioController::_onNewSession(IAudioSessionControl* ac) {
	CComPtr<IAudioSessionControl> a = ac;
	bool ok = AudioController::_createNewSession(a);
	a.Release();
	return ok ? S_OK : E_FAIL;
}