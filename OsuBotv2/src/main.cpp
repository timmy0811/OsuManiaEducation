#include <iostream>
#include <windows.h>
#include <time.h>

#include <chrono>
#include <thread>

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif // DEBUG

#define CLK_THRESHHOLD 0xFF

// TODO: Shall be passed as arguments
#define PXL_Y 1162

#define PXL_X1 1035
#define PXL_X2 1185
#define PXL_X3 1335
#define PXL_X4 1485

// Keys
#define K_D 0x44
#define K_F 0x46
#define K_K 0x4B
#define K_L 0x4C

// Screen Capture
inline unsigned int CalcRGBSum(RGBTRIPLE& rgb);
inline void ColorRefToRGBTrip(const COLORREF& ref, RGBTRIPLE& trip);

// Handling Keypresses
void KeyPress(INPUT* input, DWORD type, WORD button);
void KeyPress(INPUT* input, DWORD type, WORD button, DWORD flags, const char key);
inline const bool HandleKeyline(HDC& dc, DWORD type, WORD button, const char key, unsigned int x, unsigned int& clickRef);

// Setup and Debug
void ShowDebug(double dt, unsigned int incr, unsigned int clicks);
void SetupKey(WORD key, const char keyS);

constexpr bool DoSetup = false;

int main() {
	// Wait for startup
	std::cout << "Waiting 3 seconds..." << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	std::cout << "Done!" << std::endl;

	bool isRunning = true;
	HDC dc = GetDC(NULL);

	unsigned int increment = 0;
	unsigned int clicks = 0;

	time_t timeLast = time(0);
	double dt = 0;

	// Setup
	if (DoSetup) {
		SetupKey(K_D, 'd');
		SetupKey(K_F, 'f');
		SetupKey(K_K, 'k');
		SetupKey(K_F, 'a');

		return 0;
	}

	while (isRunning) {
		// Handle Key Lines
		HandleKeyline(dc, INPUT_KEYBOARD, K_D, 'd', PXL_X1, clicks);
		HandleKeyline(dc, INPUT_KEYBOARD, K_F, 'f', PXL_X2, clicks);
		HandleKeyline(dc, INPUT_KEYBOARD, K_K, 'k', PXL_X3, clicks);
		HandleKeyline(dc, INPUT_KEYBOARD, K_L, 'a', PXL_X4, clicks);

		// Send Input
		/*UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		if (uSent != ARRAYSIZE(inputs)) {
			std::cout << "Sending Input failed: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
		}*/

		// Escape
		if (GetKeyState(VK_ESCAPE) & 0x8000) {
			std::cout << "Exiting." << std::endl;
			isRunning = false;
		}

		/*increment++;
		dt = difftime(time(0), timeLast);
		timeLast = time(0);*/

		//osuShowDebug(dt, increment, clicks);
	}

	ReleaseDC(NULL, dc);

	return 0;
}

inline unsigned int CalcRGBSum(RGBTRIPLE& rgb)
{
	return rgb.rgbtRed + rgb.rgbtGreen + rgb.rgbtBlue;
}

inline void ColorRefToRGBTrip(const COLORREF& ref, RGBTRIPLE& trip)
{
	trip.rgbtRed = GetRValue(ref);
	trip.rgbtGreen = GetGValue(ref);
	trip.rgbtBlue = GetBValue(ref);
}

void KeyPress(INPUT* input, DWORD type, WORD button)
{
	input->type = type;
	input->ki.wVk = button;
}

void KeyPress(INPUT* input, DWORD type, WORD button, DWORD flags, const char key)
{
	input->type = type;
	input->ki.wVk = button;
	input->ki.dwFlags = flags;
	input->ki.wScan = MapVirtualKey(LOBYTE(key), 0);
}

inline const bool HandleKeyline(HDC& dc, DWORD type, WORD keyCode, const char key, unsigned int x, unsigned int& clickRef)
{
	static RGBTRIPLE rgb;
	static INPUT inputs[] = { 0 };
	bool res = false;
	UINT uSent;

	ColorRefToRGBTrip(GetPixel(dc, x, PXL_Y), rgb);
	if (CalcRGBSum(rgb) >= CLK_THRESHHOLD) {
		KeyPress(inputs, type, keyCode, KEYEVENTF_SCANCODE, key);
		uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

		clickRef++;
		res = true;
	}
	else {
		KeyPress(inputs, INPUT_KEYBOARD, keyCode, KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE, key);
		uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

		res = false;
	}

	if (uSent != ARRAYSIZE(inputs)) {
		std::cout << "Sending Input failed: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
	}

	return res;
}

void ShowDebug(double dt, unsigned int incr, unsigned int clicks)
{
	system("cls");
	//std::cout << dt << std::endl;
	std::cout << "Checks/ Sec: " << (dt == 0.f ? (1.f/ dt) : 0) << std::endl;
	std::cout << "Cycletime: " << dt << std::endl;
	std::cout << "Clicks total: " << clicks << std::endl;
	std::cout << "Cycles total: " << incr << std::endl;
}

void SetupKey(WORD key, const char keyS)
{
	INPUT inputs[] = { 0 };

	std::cout << "Setting up: " << (char)key << std::endl;

	KeyPress(inputs, INPUT_KEYBOARD, key, KEYEVENTF_SCANCODE, keyS);
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	std::this_thread::sleep_for(std::chrono::milliseconds(30));

	KeyPress(inputs, INPUT_KEYBOARD, key, KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE, keyS);
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
