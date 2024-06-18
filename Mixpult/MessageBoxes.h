#pragma once
#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

#define MESSAGE_ERR_RETRY(s, r)\
  if (MessageBoxA(nullptr, s, "Mixpult", MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY) {\
    goto r;\
  } else {\
    throw;\
  }

#define MESSAGE_DEBUG(s)\
  {\
  __pragma (warning(push))\
  __pragma (warning(disable: 6387))\
  auto __size = sizeof(s) + sizeof(__FILE__) + sizeof(TOSTR(__LINE__));\
  char* __b = (char*)malloc(__size);\
  sprintf_s(__b, __size, "%s\n%s:%s", s, __FILE__, TOSTR(__LINE__));\
  MessageBoxA(nullptr, __b, "Mixpult", MB_ICONWARNING | MB_OK);\
  free(__b);\
  __pragma (warning(pop))\
  }

#define MESSAGE_INFO(s)\
  MessageBoxA(nullptr, s, "Mixpult", MB_ICONINFORMATION | MB_OK);