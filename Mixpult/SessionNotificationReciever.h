#pragma once

#include <audiopolicy.h>
#include <functional>

//FIXME: Doesnt work idk

class SessionNotificationReciever : public IAudioSessionNotification {
public:
  SessionNotificationReciever(std::function<HRESULT(IAudioSessionControl*)> cb) : _cRefAll(1), _callback(cb) {};

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvInterface) {
    if (IID_IUnknown == riid) {
      AddRef();
      *ppvInterface = (IUnknown*)this;
    } else if (__uuidof(IAudioSessionNotification) == riid) {
      AddRef();
      *ppvInterface = (IAudioSessionNotification*)this;
    } else {
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

  HRESULT OnSessionCreated(IAudioSessionControl* pNewSession) {
    return _callback(pNewSession);
  }

  ~SessionNotificationReciever() {};
private:

  LONG _cRefAll;
  std::function<HRESULT(IAudioSessionControl*)> _callback;
};

