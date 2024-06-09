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

std::set<std::string> AudioController::getAllSessions() {
	std::set<std::string> names;
	CComPtr<IAudioSessionControl> s;
	CComPtr<IAudioSessionControl2> s2;
	int count = getSessionCount();

	for (int i = 0; i < count; i++) {
		if (FAILED(_pAudioSessionEnumerator->GetSession(i, &s))) {
			continue;
		}

		if (FAILED(s->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&s2))) {
			continue;
		}

		DWORD pid;
		if (FAILED(s2->GetProcessId(&pid))) {
			continue;
		}

		std::string process_name = AudioController::getPIDName(pid);
		std::transform(process_name.begin(), process_name.end(), process_name.begin(), ::towlower);
		names.insert(process_name);
		s.Release();
		s2.Release();
	}

	return names;
}

bool AudioController::setSessionMute(std::string name, bool mute) {
	std::vector<CComPtr<IAudioSessionControl>> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (!AudioController::_getSessionControlByName(name, pSessionControl)) {
		return false;
	}

	bool ok = true;
	for (auto& sc : pSessionControl) {
		// Ask for SimpleAudioVolume
		if (FAILED(sc->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
			return false;
		}

		HRESULT s;
		s = pSAV->SetMute(mute, NULL);
		ok &= (s == S_OK);
		sc.Release();
		pSAV.Release();
	}

	return ok;
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
	std::vector<CComPtr<IAudioSessionControl>> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (!AudioController::_getSessionControlByName(name, pSessionControl)) {
		return false;
	}

	bool mute = true;
	for (auto& sc : pSessionControl) {
		// Ask for SimpleAudioVolume
		if (FAILED(sc->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
			return false;
		}

		BOOL m;
		pSAV->GetMute(&m);
		mute &= (bool)m;
		sc.Release();
		pSAV.Release();
	}

	return mute;
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
	std::vector<CComPtr<IAudioSessionControl>> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;
	// Get SessionControl
	if (!AudioController::_getSessionControlByName(name, pSessionControl)) {
		return false;
	}

	bool hr = true;
	for (auto& sc : pSessionControl) {
		// Ask for SimpleAudioVolume
		if (FAILED(sc->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
			return false;
		}

		HRESULT s;
		s = pSAV->SetMasterVolume(vol, NULL);
		hr &= (s == S_OK);
		sc.Release();
		pSAV.Release();
	}

	return hr;
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
	std::vector<CComPtr<IAudioSessionControl>> pSessionControl;
	CComPtr<ISimpleAudioVolume> pSAV;

	// Get SessionControl
	if (!AudioController::_getSessionControlByName(name, pSessionControl)) {
		return -1;
	}

	float maxvol = -1;
	for (auto& sc : pSessionControl) {
		// Ask for SimpleAudioVolume
		if (FAILED(sc->QueryInterface(__uuidof(ISimpleAudioVolume), (VOID**)&pSAV))) {
			continue;
		}

		float vol;
		if (FAILED(pSAV->GetMasterVolume(&vol))) {
			continue;
		}
		maxvol = std::max<float>(vol, maxvol);
		sc.Release();
		pSAV.Release();
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

// Dont forget to release!
bool AudioController::_getSessionControlByName(std::string name, std::vector<CComPtr<IAudioSessionControl>> &ascs) {
  CComPtr<IAudioSessionControl> s = NULL;
	CComPtr<IAudioSessionControl2> s2 = NULL;
	std::transform(name.begin(), name.end(), name.begin(), ::towlower);
	int count = getSessionCount();

	for (int i = 0; i < count; i++) {
		if (FAILED(_pAudioSessionEnumerator->GetSession(i, &s))) {
			continue;
		}

		if (FAILED(s->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&s2))) {
			continue;
		}

		DWORD pid;
		if (FAILED(s2->GetProcessId(&pid))) {
			continue;
		}

		std::string process_name = AudioController::getPIDName(pid);
		std::transform(process_name.begin(), process_name.end(), process_name.begin(), ::towlower);
		if (name == process_name) {
			ascs.emplace_back(s);
			return true;
		}
		s.Release();
		s2.Release();
	}

	ascs = {};
  return false;
}
