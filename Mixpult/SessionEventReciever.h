#pragma once

#include <audiopolicy.h>
#include <functional>
#include <string>

class SessionEventReciever : public IAudioSessionEvents {
public:
  SessionEventReciever() : _cRefAll(1), _disconnect_cb(nullptr), _sav_cb(nullptr), _state_cb(nullptr) {};

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvInterface) {
    if (IID_IUnknown == riid) {
      AddRef();
      *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IAudioSessionEvents) == riid) {
      AddRef();
      *ppvInterface = (IAudioSessionEvents*)this;
    }
    else {
      *ppvInterface = NULL;
      return E_NOINTERFACE;
    }
    return S_OK;
  }

  ULONG STDMETHODCALLTYPE AddRef() {
    return InterlockedIncrement(&_cRefAll);
  }

  ULONG STDMETHODCALLTYPE Release() {
    ULONG ulRef = InterlockedDecrement(&_cRefAll);
    if (0 == ulRef) {
      delete this;
    }
    return ulRef;
  }

  void setID(unsigned long id) {
    _id = id;
  }

  // Disconnect
  void setDisconnectCallback(std::function<HRESULT(unsigned long, AudioSessionDisconnectReason)> cb) {
    _disconnect_cb = std::move(cb);
  }

  HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason reason) {
    if (_disconnect_cb) {
      return _disconnect_cb(_id, reason);
    }
    else {
      return S_OK;
    }
  }

  // SAV
  void setSAVChangeCallback(std::function<HRESULT(unsigned long, float, BOOL, LPCGUID)> cb) {
    _sav_cb = std::move(cb);
  }

  HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float volume, BOOL mute, LPCGUID context) {
    if (_sav_cb) {
      return _sav_cb(_id, volume, mute, context);
    }
    else {
      return S_OK;
    }
  }

  // State
  void setStateChangeCallback(std::function<HRESULT(unsigned long, AudioSessionState)> cb) {
    _state_cb = std::move(cb);
  }

  HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState state) {
    if (_state_cb) {
      return _state_cb(_id, state);
    }
    else {
      return S_OK;
    }
  }

  // Unused
  HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(DWORD, float*, DWORD, LPCGUID) {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR, LPCGUID) {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID, LPCGUID) {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR, LPCGUID) {
    return S_OK;
  }

  ~SessionEventReciever() {};

private:
  LONG _cRefAll;
  unsigned long _id;
  std::function<HRESULT(unsigned long, AudioSessionDisconnectReason)> _disconnect_cb;
  std::function<HRESULT(unsigned long, float, BOOL, LPCGUID)> _sav_cb;
  std::function<HRESULT(unsigned long, AudioSessionState)> _state_cb;
};
