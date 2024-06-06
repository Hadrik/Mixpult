#include "AudioController.h"

using namespace System;
using namespace System::Diagnostics;

CComPtr<IAudioEndpointVolume> AudioController::_pEpVol = NULL;
CComPtr<IAudioSessionEnumerator> AudioController::_pAudioSessionEnumerator = NULL;

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
}

void AudioController::exit() {
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

std::string AudioController::getSessionName(SessionID_t id){
	if (id == -1)
		return "master";
	DWORD pid = getSessionPID(id);

	return AudioController::getPIDName(pid);
}

std::string AudioController::getSessionTitle(SessionID_t id) {
	if (id == -1)
		return "master";
	DWORD pid = getSessionPID(id);

	return AudioController::getPIDTitle(pid);
}

DWORD AudioController::getSessionPID(SessionID_t id) {
	CComPtr<IAudioSessionControl> pSessionControl;
	CComPtr<IAudioSessionControl2> pSC2;
	int sessionCount = getSessionCount();
	if (id >= sessionCount || id < 0) {
		return NULL;
	}
	// Get SessionControl
	if (FAILED(AudioController::_pAudioSessionEnumerator->GetSession(id, &pSessionControl))) {
		return NULL;
	}
	// audio session 2 for extended name info
	if (FAILED(pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (VOID**)&pSC2))) {
		return NULL;
	}

	DWORD procid;
	pSC2->GetProcessId(&procid);
	return procid;
}

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
	return msclr::interop::marshal_as<std::string>(procname);
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

NameIDMap_t AudioController::getNameIDMap() {
	const int count = AudioController::getSessionCount();
	NameIDMap_t map;
	for (int i = 0; i < count; i++) {
		std::string name = AudioController::getPIDName(AudioController::getSessionPID(i));
		std::transform(name.begin(), name.end(), name.begin(), ::towlower);

		auto mit = map.find(name);
		if (mit != map.end()) {
			mit->second.push_back(i);
		} else {
			map.insert({ name, std::vector<SessionID_t>({i}) });
		}
	}
	return map;
}

bool AudioController::setSessionMute(SessionID_t id, bool mute) {
	int sessionCount = getSessionCount();
	if (id == -1)
		return setMasterMute(mute);
	if (id >= sessionCount || id < 0) {
		return false;
	}
	CComPtr<IAudioSessionControl> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (FAILED(AudioController::_pAudioSessionEnumerator->GetSession(id, &pSessionControl))) {
		return false;
	}
	// Ask for SimpleAudioVolume
	if (FAILED(pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
		return false;
	}

	return S_OK == pSAV->SetMute(mute, NULL);
}

bool AudioController::setSessionMute(std::vector<SessionID_t> ids, bool mute) {
	bool ok = true;
	for (const auto& id : ids) {
		ok *= AudioController::setSessionMute(id, mute);
	}
	return ok;
}

bool AudioController::setMasterMute(bool mute) {
	return S_OK == AudioController::_pEpVol->SetMute(mute, NULL);
}

bool AudioController::getSessionMute(SessionID_t id) {
	int sessionCount = getSessionCount();
	if (id == -1)
		return getMasterMute();
	if (id >= sessionCount || id < 0) {
		return false;
	}
	CComPtr<IAudioSessionControl> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (FAILED(AudioController::_pAudioSessionEnumerator->GetSession(id, &pSessionControl))) {
		return false;
	}
	// Ask for SimpleAudioVolume
	if (FAILED(pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
		return false;
	}
	BOOL mute;
	pSAV->GetMute(&mute);
	return mute != 0;
}

bool AudioController::getSessionMute(std::vector<SessionID_t> ids) {
	bool mute = false;
	for (const auto& id : ids) {
		mute |= AudioController::getSessionMute(id);
	}
	return mute;
}

bool AudioController::getMasterMute() {
	BOOL mute;
	AudioController::_pEpVol->GetMute(&mute);
	return mute != 0;
}

bool AudioController::setSessionVolume(SessionID_t id, float vol) {
	int sessionCount = getSessionCount();
	if (id == -1)
		return setMasterVolume(vol);
	if (id >= sessionCount || id < 0) {
		return false;
	}
	CComPtr<IAudioSessionControl> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (FAILED(AudioController::_pAudioSessionEnumerator->GetSession(id, &pSessionControl))) {
		return false;
	}
	// Ask for SimpleAudioVolume
	if (FAILED(pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
		return false;
	}

	return S_OK == pSAV->SetMasterVolume(vol, NULL);
}

bool AudioController::setSessionVolume(std::vector<SessionID_t> ids, float vol) {
	bool ok = true;
	for (const auto& id : ids) {
		ok *= AudioController::setSessionVolume(id, vol);
	}
	return ok;
}

bool AudioController::setMasterVolume(float vol) {
	return S_OK == AudioController::_pEpVol->SetMasterVolumeLevelScalar(vol, NULL);
}

float AudioController::getSessionVolume(SessionID_t id) {
	CComPtr<IAudioSessionControl> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	int sessionCount = getSessionCount();
	float vol;
	if (id == -1)
		return getMasterVolume();
	if (id >= sessionCount || id < 0) {
		return -1;
	}

	// Get SessionControl
	if (FAILED(AudioController::_pAudioSessionEnumerator->GetSession(id, &pSessionControl))) {
		return -1;
	}
	// Ask for SimpleAudioVolume
	if (FAILED(pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
		return -1;
	}

	pSAV->GetMasterVolume(&vol);

	return vol;
}

float AudioController::getSessionVolume(std::vector<SessionID_t> ids) {
	float highest = 0.0;
	for (const auto& id : ids) {
		highest = std::max<float>(AudioController::getSessionVolume(id), highest);
	}
	return highest;
}

float AudioController::getMasterVolume() {
	float vol;
	AudioController::_pEpVol->GetMasterVolumeLevelScalar(&vol);
	return vol;
}
