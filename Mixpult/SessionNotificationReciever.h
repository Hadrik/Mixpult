#pragma once

#include <audiopolicy.h>

class SessionNotificationReciever : public IAudioSessionNotification {
public:
  SessionNotificationReciever() : _cRefAll(1) {};

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

  HRESULT OnSessionCreated(IAudioSessionControl* pNewSession)
  {
    if (pNewSession) {
    }
  }

private:
  ~SessionNotificationReciever() {};

  LONG _cRefAll;
};

