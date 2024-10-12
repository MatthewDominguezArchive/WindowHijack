#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>

void KillProcessByHWND(HWND hwnd) {
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);

	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
	if (hProcess != NULL) exit(1);
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
}

int main() {
	const char* targetWindow = "Notepad";

	FreeConsole();
	HWND hwnd = FindWindow(targetWindow, 0);
	if (!hwnd) return 1;

	SetMenu(hwnd, NULL);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE);
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	HDC hdc = GetDC(hwnd);
	const int sx = GetSystemMetrics(SM_CXSCREEN); // AKA 0
	const int sy = GetSystemMetrics(SM_CYSCREEN); // AKA 1

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	BITMAPINFO backBufferBmpInfo = {};
	backBufferBmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	backBufferBmpInfo.bmiHeader.biWidth = sx;
	backBufferBmpInfo.bmiHeader.biHeight = sy;
	backBufferBmpInfo.bmiHeader.biPlanes = 1;
	backBufferBmpInfo.bmiHeader.biBitCount = static_cast<WORD>(dm.dmBitsPerPel);
	backBufferBmpInfo.bmiHeader.biCompression = BI_RGB;

	void* backBufferPixels = nullptr;
	HBRUSH bgTransparencyColor = CreateSolidBrush(RGB(0,0,0));
	HBRUSH hbrRed = CreateSolidBrush(RGB(255, 0, 0));
	HDC hdcBackBuffer;
	HBITMAP hbmBackBuffer;

	while (!GetAsyncKeyState(VK_PAUSE)) {
		hdcBackBuffer = CreateCompatibleDC(hdc);
		hbmBackBuffer = CreateDIBSection(hdcBackBuffer, (BITMAPINFO*)&backBufferBmpInfo, DIB_RGB_COLORS, (void**)&backBufferPixels, NULL, 0);
		if (!hbmBackBuffer) return 1;

		DeleteObject(SelectObject(hdcBackBuffer, hbmBackBuffer));
		DeleteObject(SelectObject(hdcBackBuffer, bgTransparencyColor)); // To set background in transparent color key

		// Background
		Rectangle(hdcBackBuffer, 0, 0, sx, sy);

		// Rectangle
		SelectObject(hdcBackBuffer, hbrRed);
		Rectangle(hdcBackBuffer, sx / 2 - 40, sy / 2 - 40, sx / 2 + 40, sy / 2 + 40);

		// Rectangle Fill
		SelectObject(hdcBackBuffer, bgTransparencyColor);
		Rectangle(hdcBackBuffer, sx / 2 - 38, sy / 2 - 38, sx / 2 + 38, sy / 2 + 38);

		// Display Frame
		BitBlt(hdc, 0, 0, sx, sy, hdcBackBuffer, 0, 0, SRCCOPY);

		// Cleanup
		DeleteDC(hdcBackBuffer);
		DeleteObject(hbmBackBuffer);
		backBufferPixels = nullptr;
	}

	KillProcessByHWND(hwnd);
	return EXIT_SUCCESS;
}