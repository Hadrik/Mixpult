#pragma once

#define MESSAGE_ERR_RETRY(s, r)\
  if (MessageBoxA(nullptr, s, "Mixpult", MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY) {\
    goto r;\
  } else {\
    throw;\
  }