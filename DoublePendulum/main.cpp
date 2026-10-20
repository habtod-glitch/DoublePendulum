#include <windows.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
 int windowwidth = 800;
 int windowheight = 600;
const float dt = 0.1f;
const float g = 9.81f;
HDC     backDC = nullptr;
HBITMAP backBmp = nullptr;
HBITMAP oldBmp = nullptr;
void InitBackBuffer(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	backDC = CreateCompatibleDC(hdc);
	backBmp = CreateCompatibleBitmap(hdc, windowwidth, windowheight);
	oldBmp = (HBITMAP)SelectObject(backDC, backBmp);
	ReleaseDC(hwnd, hdc);
}

class Pendulum {
public:
	float theta1=1.5f;
	float theta2=1.5f;
	float omega1=0.0f;
	float omega2 = 0.0f;
	float pivotx=400.0f;
	float pivoty=150.0f;
	float l1=300.0f;
	float l2=300.0f;
	float time = 0.0f;

		void Update()
		{
		
			float alpha1 = (
				-g * (2.0f * sinf(theta1) + sinf(theta1 - 2.0f * theta2))
				- 2.0f * sinf(theta1 - theta2) * (
					omega2 * omega2 * l2
					+ omega1 * omega1 * l1 * cosf(theta1 - theta2)
					)
				) / (
					l1 * (2.0f - cosf(2.0f * theta1 - 2.0f * theta2))
					);

			float alpha2 = (
				2.0f * sinf(theta1 - theta2) * (
					omega1 * omega1 * l1
					+ g * cosf(theta1)
					+ omega2 * omega2 * l2 * cosf(theta1 - theta2)
					)
				) / (
					l2 * (2.0f - cosf(2.0f * theta1 - 2.0f * theta2))
					);

			omega1 += alpha1 * dt;
			omega2 += alpha2 * dt;
			theta1 += omega1 * dt;
			theta2 += omega2 * dt;

		}
	
		void Draw(HDC hdc)
		{
			float x1 = pivotx + l1 * sinf(theta1);
			float y1 = pivoty + l1 * cosf(theta1);
			float x2 = x1 + l2 * sinf(theta2);
			float y2 = y1 + l2 * cosf(theta2);

			// Нити
			HPEN pen = CreatePen(PS_SOLID, 4, RGB(0, 0, 0));
			HGDIOBJ oldPen = SelectObject(hdc, pen);
			MoveToEx(hdc, (int)pivotx, (int)pivoty, nullptr);
			LineTo(hdc, (int)x1, (int)y1);
			LineTo(hdc, (int)x2, (int)y2);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);

			// Первый груз (синий)
			HBRUSH brush1 = CreateSolidBrush(RGB(0, 0, 255));
			HGDIOBJ oldBrush1 = SelectObject(hdc, brush1);   // ← привязали
			Ellipse(hdc, int(x1) - 25, int(y1) - 25, int(x1) + 25, int(y1) + 25);
			SelectObject(hdc, oldBrush1);                     // ← отвязали
			DeleteObject(brush1);

			// Второй груз (зелёный)
			HBRUSH brush2 = CreateSolidBrush(RGB(0, 255, 0));
			HGDIOBJ oldBrush2 = SelectObject(hdc, brush2);   // ← привязали (ЭТОГО НЕ ХВАТАЛО!)
			Ellipse(hdc, int(x2) - 25, int(y2) - 25, int(x2) + 25, int(y2) + 25);
			SelectObject(hdc, oldBrush2);                     // ← отвязали
			DeleteObject(brush2);
		}
}; 
Pendulum pendulums;
void Draw()
{
	HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
	RECT FullRect = { 0, 0, windowwidth, windowheight };
	FillRect(backDC, &FullRect, bgBrush);
	DeleteObject(bgBrush);
	pendulums.Draw(backDC);
}
void Update() {
	pendulums.Update();
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_CREATE:
		InitBackBuffer(hwnd);
		pendulums.pivotx = windowwidth / 2.0f;   // 768
		pendulums.pivoty = windowheight / 4.0f;   // 216
		SetTimer(hwnd, 1, 7, nullptr);
		return 0;
	case WM_TIMER:
		Update();
		InvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		Draw();
		BitBlt(hdc, 0, 0, windowwidth, windowheight,
			backDC, 0, 0, SRCCOPY);

		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		return 0;


	case WM_DESTROY:
		KillTimer(hwnd, 1);
		SelectObject(backDC, oldBmp);
		DeleteObject(backBmp);
		DeleteDC(backDC);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	windowwidth = GetSystemMetrics(SM_CXSCREEN);
	windowheight = GetSystemMetrics(SM_CYSCREEN);
	srand((unsigned)time(nullptr));
	const wchar_t CLASS_NAME[] = L"ParticleSimClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;

	RegisterClass(&wc);

	RECT rect = { 0, 0, windowwidth, windowheight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	int screenW = GetSystemMetrics(SM_CXSCREEN); // Ширина экрана
	int screenH = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, L"Particle Simulator",
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		screenW, screenH,
		nullptr,
		nullptr, hInstance, nullptr
	);

	ShowWindow(hwnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
