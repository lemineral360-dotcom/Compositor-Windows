#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

#include <exception>
#include <filesystem>
#include <string>

#include "core/project_manifest.h"

namespace {

constexpr wchar_t kWindowClass[] = L"CompositorWindowsEditor";
constexpr UINT kOpenProject = 1001;
constexpr UINT kExit = 1002;
constexpr UINT kAbout = 1003;

std::wstring g_status = L"Ready — open a .comp project to validate it.";

void showProjectDialog(HWND window) {
  wchar_t path[32768]{};
  BROWSEINFOW info{};
  info.hwndOwner = window;
  info.lpszTitle = L"Select a Compositor .comp project folder";
  info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  PIDLIST_ABSOLUTE selected = SHBrowseForFolderW(&info);
  if (!selected) return;
  const bool resolved = SHGetPathFromIDListW(selected, path) != FALSE;
  CoTaskMemFree(selected);
  if (!resolved) return;

  try {
    const auto manifest = compositor::readManifest(std::filesystem::path(path));
    g_status = L"Project valid: " + std::to_wstring(manifest.width) + L" × " +
               std::to_wstring(manifest.height) + L", " +
               std::to_wstring(manifest.layers.size()) + L" layer(s).";
  } catch (const std::exception& error) {
    const auto length = MultiByteToWideChar(CP_UTF8, 0, error.what(), -1, nullptr, 0);
    std::wstring detail(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, error.what(), -1, detail.data(), length);
    g_status = L"Could not open project: " + detail;
  }
  InvalidateRect(window, nullptr, TRUE);
}

HMENU createMenu() {
  HMENU bar = CreateMenu();
  HMENU file = CreatePopupMenu();
  AppendMenuW(file, MF_STRING, kOpenProject, L"&Open .comp Folder…\tCtrl+O");
  AppendMenuW(file, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(file, MF_STRING, kExit, L"E&xit");
  AppendMenuW(bar, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"&File");

  HMENU help = CreatePopupMenu();
  AppendMenuW(help, MF_STRING, kAbout, L"&About Compositor for Windows");
  AppendMenuW(bar, MF_POPUP, reinterpret_cast<UINT_PTR>(help), L"&Help");
  return bar;
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_COMMAND:
      switch (LOWORD(wparam)) {
        case kOpenProject: showProjectDialog(window); return 0;
        case kExit: DestroyWindow(window); return 0;
        case kAbout:
          MessageBoxW(window,
              L"Compositor for Windows\n\nNative Windows port in active development.\n"
              L"This build validates Compositor project packages v1–v7.",
              L"About", MB_OK | MB_ICONINFORMATION);
          return 0;
      }
      break;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      HDC dc = BeginPaint(window, &paint);
      RECT bounds{};
      GetClientRect(window, &bounds);
      HBRUSH background = CreateSolidBrush(RGB(32, 34, 39));
      FillRect(dc, &bounds, background);
      DeleteObject(background);

      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, RGB(235, 236, 240));
      RECT title{32, 28, bounds.right - 32, 70};
      DrawTextW(dc, L"Compositor", -1, &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
      SetTextColor(dc, RGB(170, 174, 184));
      RECT status{32, 76, bounds.right - 32, bounds.bottom - 32};
      DrawTextW(dc, g_status.c_str(), -1, &status, DT_LEFT | DT_WORDBREAK);
      EndPaint(window, &paint);
      return 0;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(window, message, wparam, lparam);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.hInstance = instance;
  windowClass.lpfnWndProc = windowProcedure;
  windowClass.lpszClassName = kWindowClass;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassExW(&windowClass)) return 1;

  HWND window = CreateWindowExW(0, kWindowClass, L"Compositor for Windows",
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1120, 760, nullptr,
      createMenu(), instance, nullptr);
  if (!window) return 2;
  ShowWindow(window, show);
  UpdateWindow(window);

  MSG message{};
  while (GetMessageW(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
  CoUninitialize();
  return static_cast<int>(message.wParam);
}
