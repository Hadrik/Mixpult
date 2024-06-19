#include "BasicSerial.h"
#include "AudioController.h"
#include "SliderController.h"
#include "MasterController.h"
#include "OtherController.h"
#include "ConfigLoader.h"
#include "MessageBoxes.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#define IDI_ICON 101
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_QUIT (WM_USER + 2)
#define ID_TRAY_SHOW_SESSIONS (WM_USER + 3)
#define ID_TRAY_OPEN_CONFIG (WM_USER + 4)
#define SETVOLUME_CB_ID 0
#define HANDLESERIAL_CB_ID 1

std::vector<std::vector<SliderController*>> sliders;
BasicSerial s;
std::string config_path;

void CALLBACK setVolume() {
  // Split string to vector
  std::vector<std::string> d;
  std::stringstream ss(s.getLastCommand());
  std::string item;
  while (std::getline(ss, item, '|')) {
    d.push_back(item);
  }

  // Assing volume to each controller
  for (size_t i = 0; i < std::min<size_t>(sliders.size(), d.size()); i++) {
    for (auto& slider : sliders[i]) {
      if (d[i] == "m") {
        slider->setMute(true);
      } else {
        int val;
        try {
          val = std::stoi(d[i]);
        } catch (...) {
          return;
        }
        slider->setVolume(val / 1024.0f);
      }
    }
  }
}

void respond(std::string) {
  s.write("resp");
}

void CALLBACK handleSerial() {
  s.update();
}

void CALLBACK showSessionsPopup() {
  auto names = AudioController::getAllSessions();
  std::string str;
  for (const auto& name : names) {
    str.append(name + "\n");
  }
  MESSAGE_INFO(str.c_str());
}

void CALLBACK openConfig() {
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  auto r = ShellExecuteA(nullptr, nullptr, config_path.c_str(), nullptr, nullptr, SW_SHOW);
  CoUninitialize();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  static HMENU hMenu;
  static NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
  switch (message) {
  case WM_TRAYICON:
    switch (lParam) {
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      POINT pt;
      GetCursorPos(&pt);
      SetForegroundWindow(hWnd);
      TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
      break;
    }
    break;
  case WM_CREATE:
    // Add the tray icon when the window is created
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_QUIT, L"Quit");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW_SESSIONS, L"Show current sessions");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_OPEN_CONFIG, L"Open config file");
    nid.hWnd = hWnd;
    nid.uID = IDI_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    lstrcpyW(nid.szTip, L"Mixpult");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Hide the window
    ShowWindow(hWnd, SW_HIDE);
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case ID_TRAY_QUIT:
      Shell_NotifyIcon(NIM_DELETE, &nid);
      PostQuitMessage(0);
      break;
    case ID_TRAY_SHOW_SESSIONS:
      showSessionsPopup();
      break;
    case ID_TRAY_OPEN_CONFIG:
      openConfig();
      break;
    }
    break;
  case WM_TIMER:
    switch (wParam) {
    case SETVOLUME_CB_ID:
      setVolume();
      break;
    case HANDLESERIAL_CB_ID:
      handleSerial();
      break;
    }
    break;
  case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, &nid);
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

HWND createWindow(HINSTANCE hInstance, int nCmdShow) {
  const LPCWSTR CLASS_NAME = L"Mixpult";

  WNDCLASS wc = {};

  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc)) {
    MessageBox(NULL, L"Window registration failed!", CLASS_NAME, MB_ICONEXCLAMATION | MB_OK);
    throw;
  }

  HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"My Window", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, NULL);

  if (hWnd == NULL) {
    MessageBox(NULL, L"Window creation failed!", CLASS_NAME, MB_ICONEXCLAMATION | MB_OK);
    throw;
  }

  ShowWindow(hWnd, SW_HIDE);
  UpdateWindow(hWnd);
  return hWnd;
}

std::string getPath(LPSTR cmd) {
  std::istringstream iss(cmd);
  std::vector<std::string> args((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  if (args.size() == 1) {
    std::replace(args[0].begin(), args[0].end(), '/', '\\');
    return args.at(0);
  }
  return std::string(".\\config.json");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  HWND hWnd = createWindow(hInstance, nCmdShow);
  config_path = getPath(lpCmdLine);

  // Read config
  Config_t cfg = ConfigLoader::load(config_path);

  // Get all sessions
  AudioController::init();
  const auto names = AudioController::getAllSessions();
  std::cout << "Discovered streams:\n";
  for (const auto& name : names) {
    std::cout << name;
    std::cout << "\n";
  }

  // Set up controllers
  for (const auto& slider : cfg.sliders) {
    std::vector<SliderController*> vsc;

    // Array?
    if (slider.size() > 1) {
      if (slider[0] == "!other") {
        std::vector<std::string> c = slider;
        c.erase(c.begin());
        vsc.push_back(new OtherController(c));
      } else {
        for (const auto& app : slider) {
          vsc.push_back(new SliderController(app));
        }
      }
    } else {
      if (slider[0] == "!master") {
        vsc.push_back(new MasterController());
      } else if (slider[0] == "!other") {
        vsc.push_back(new OtherController());
      } else {
        vsc.push_back(new SliderController(slider[0]));
      }
    }

    sliders.push_back(vsc);
  }

  // Configure serial connection
  if (!s.open(cfg.port)) return 1;
  s.registerCommandCallback("ping", respond);
  // Create callback timers
  SetTimer(hWnd, SETVOLUME_CB_ID, 100, NULL);
  SetTimer(hWnd, HANDLESERIAL_CB_ID, 50, NULL);

  // Start message loop
  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}
