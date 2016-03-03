// Sebastian Evoniak
// This program generates Worley Noise according to the user's input parameters.

#ifndef UNICODE
#define UNICODE
#endif

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define BUTTON_ID 100
#define EDIT_ID 150
#define RADIO_ID 160
#define STATIC_ID 170
#define POPUP_ID 200
#define PROGBAR_ID 300
#define MY_MAX_PATH 260
#define MY_MAX_LENGTH 8192

#include <Windows.h>
#include <sstream>
#include <CommCtrl.h>
#include "resource.h"
#include "NoiseGenerator.h"
using namespace std;

//forward declarations
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK PopWndProc(HWND,UINT,WPARAM,LPARAM);
void registerPopupWindowClass();
void setupUIElements(HWND);
void createBackBuffer(HWND, HDC);
void paintWindow(HWND, HDC);
void updateProg();
void saveFile();
void prepDefDirStr(wchar_t* str, wchar_t* name, int len);

//globals
HINSTANCE hInst;
int cxClient, cyClient, numPoints;
double ratio;
HWND hPopup;
HWND hButton_ok, hButton_cancel;
HWND hGroup1, hGroup2;
HWND hEdit_x, hEdit_y, hEdit_num;
HWND hRadio;
HWND hStatic_w, hStatic_h, hStatic_num;
HWND hProgBar;
HFONT hFontText, hFontGroup;
HBITMAP hBackBitmap;
BYTE* pColorBytes;
NoiseGenerator* ngen;
int menuCheckedItem;

// Creates the main window and control loop
int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	const wchar_t CLASS_NAME[] = L"WorleyNoiseGenClass";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	RegisterClass(&wc);

	RECT rc = {0, 0, 1600, 900};
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, NULL);
	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Worley Noise Generator", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, 30, 30, rc.right-rc.left, rc.bottom-rc.top,
							   NULL, NULL, hInstance, NULL);

	if (hwnd == 0)
		return 0;
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// Standard WndProc function for events
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch(uMsg)
	{
	case WM_DESTROY:
		{
			DeleteObject(hFontText);
			DeleteObject(hFontGroup);
			DeleteObject(hBackBitmap);
			if (ngen)
				delete ngen;
			PostQuitMessage(0);
			return 0;
		}

	case WM_CREATE:
		{
			hFontText = CreateFont(18,0,0,0,0,0,0,0,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,0,0,0,L"Arial");
			hFontGroup = CreateFont(16,0,0,0,0,0,0,0,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,0,0,0,L"Arial");
			registerPopupWindowClass();
			hBackBitmap = 0;
			ngen = 0;
			menuCheckedItem = ID_NOISETYPE_CLOSEST;
			ratio = 16.0 / 9.0;
			return 0;
		}

	case WM_PAINT:
		{	
			hdc = BeginPaint(hwnd, &ps);
			if (ngen == 0)
				EndPaint(hwnd, &ps);
			else
			{
				if (hBackBitmap == 0)
					createBackBuffer(hwnd, hdc);	//populate the back buffer if it has been deleted
				paintWindow(hwnd, hdc);
			}
			EndPaint(hwnd, &ps);
			return 0;
		}

	case WM_SIZE:
		{
			cxClient = LOWORD (lParam);
			cyClient = HIWORD (lParam);
			return 0;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			// Options - Config menu choice
			// Creates and displays the pop-up menu
			case ID_OPTIONS_CONFIG:			
				{
					POINT pt;
					pt.x = 0;
					pt.y = 0;
					ClientToScreen(hwnd, &pt);
					if (!hPopup)
					{
						hPopup = CreateWindowEx(0, L"WorleyNoiseGenPopupClass", L"Worley Noise Generator Options", WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_VISIBLE, 
											pt.x+50, pt.y+50, 320, 270, hwnd, NULL, hInst, NULL);
						EnableWindow(hwnd, FALSE);
					}
					return 0;
				}
			// Noisetype - "Closest" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_CLOSEST:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_CLOSEST, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_CLOSEST;
						ngen->generateNoise(TYPE_CLOSEST_LINEAR);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}
			// Noisetype - "2nd closest" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_2NDCLOSEST:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_2NDCLOSEST, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_2NDCLOSEST;
						ngen->generateNoise(TYPE_2NDCLOSEST_LINEAR);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}
			// Noisetype - "Closest minus 2nd closest, clamped" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_1STMINUS2ND_C:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_1STMINUS2ND_C, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_1STMINUS2ND_C;
						ngen->generateNoise(TYPE_1STMINUS2ND_CLAMPED);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}

			// Noisetype - "2nd closest minus closest, clamped" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_2NDMINUS1ST_C:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_2NDMINUS1ST_C, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_2NDMINUS1ST_C;
						ngen->generateNoise(TYPE_2NDMINUS1ST_CLAMPED);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}
			// Noisetype - "Closest minus 2nd closest, unclamped" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_1STMINUS2ND_UC:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_1STMINUS2ND_UC, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_1STMINUS2ND_UC;
						ngen->generateNoise(TYPE_1STMINUS2ND_UNCLAMPED);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}
			// Noisetype - "2nd closest minus closest, unclamped" menu choice
			// Gets the noise generator to generate noise and prepares the back buffer for re-population
			case ID_NOISETYPE_2NDMINUS1ST_UC:
				{
					if (ngen)
					{
						HMENU hMenu = GetMenu(hwnd);
						CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
						CheckMenuItem(hMenu, ID_NOISETYPE_2NDMINUS1ST_UC, MF_CHECKED);
						menuCheckedItem = ID_NOISETYPE_2NDMINUS1ST_UC;
						ngen->generateNoise(TYPE_2NDMINUS1ST_UNCLAMPED);
						DeleteObject(hBackBitmap);
						hBackBitmap = 0;
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				}
			// Save to File - Bitmap menu choice
			// Saves the image to disk
			case ID_SAVETOFILE_BITMAP:
				{
					if (ngen)
						saveFile();
					break;
				}
			}
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// WndProc callback functions for the pop-up window
LRESULT CALLBACK PopWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch(uMsg)
	{
	case WM_CLOSE:
		{
			EnableWindow(GetWindow(hPopup, GW_OWNER), TRUE);	//return control to parent window
			break;
		}

	case WM_DESTROY:
		{
			DestroyWindow(hButton_ok);
			DestroyWindow(hButton_cancel);
			DestroyWindow(hEdit_x);
			DestroyWindow(hEdit_y);
			DestroyWindow(hEdit_num);
			DestroyWindow(hRadio);
			DestroyWindow(hStatic_w);
			DestroyWindow(hStatic_h);
			DestroyWindow(hStatic_num);
			DestroyWindow(hGroup1);
			DestroyWindow(hGroup2);
			DestroyWindow(hProgBar);
			DestroyWindow(hPopup);	
			hPopup = 0;
			break;
		}

	case WM_CREATE:
		{
			setupUIElements(hwnd);
			return 0;
		}

	case WM_PAINT:
		{	
			hdc = BeginPaint(hwnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			EndPaint(hwnd, &ps);
			return 0;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{

			case EDIT_ID:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						if (SendMessage(hRadio, BM_GETCHECK, 0, 0) == BST_CHECKED)
						{
						}
					}
					break;
				}

			case EDIT_ID+1:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						if (SendMessage(hRadio, BM_GETCHECK, 0, 0) == BST_CHECKED)
						{
						}
					}
					break;
				}

			case BUTTON_ID:		//ok button
				{
					//read user input
					wchar_t strx[8] = {};
					wchar_t stry[8] = {};
					wchar_t strn[8] = {};
					strx[0] = stry[0] = strn[0] = 8;
					SendMessage(hEdit_x, EM_GETLINE, 0, (LPARAM) &strx);
					SendMessage(hEdit_y, EM_GETLINE, 0, (LPARAM) &stry);
					SendMessage(hEdit_num, EM_GETLINE, 0, (LPARAM) &strn);
					wstringstream ssx, ssy, ssn;
					ssx << strx;
					ssy << stry;
					ssn << strn;
					int x = stoi(ssx.str());
					int y = stoi(ssy.str());
					int n = stoi(ssn.str());

					//resize the window
					RECT rc = {0, 0, x, y};
					AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, NULL);
					HWND hParent = GetWindow(hwnd, GW_OWNER);
					RECT windRc = {};
					GetWindowRect(hParent, &windRc);
					MoveWindow(hParent, windRc.left, windRc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
					
					//generate the noise
					ShowWindow(hProgBar, TRUE);
					if (ngen)
						delete ngen;
					ngen = new NoiseGenerator(x, y, n, updateProg);
					ShowWindow(hProgBar, FALSE);

					HMENU hMenu = GetMenu(hParent);
					CheckMenuItem(hMenu, menuCheckedItem, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_NOISETYPE_CLOSEST, MF_CHECKED);
					menuCheckedItem = ID_NOISETYPE_CLOSEST;

					//prepare back buffer
					DeleteObject(hBackBitmap);
					hBackBitmap = 0;
					InvalidateRect(hParent, NULL, TRUE);
					SendMessage(hwnd, WM_CLOSE, 0 ,0);
					break;
				}

			case BUTTON_ID+1:	//cancel button
				{
					SendMessage(hwnd, WM_CLOSE, 0 ,0);
					break;
				}
			}
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Registers the Window Class for the pop-up menu
void registerPopupWindowClass()
{
	const wchar_t CLASS_NAME[] = L"WorleyNoiseGenPopupClass";
	WNDCLASS wc = {};
	wc.lpfnWndProc = PopWndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = CLASS_NAME;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE+1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClass(&wc);
}

// Creates and positions the various UI elements of the pop-up window
void setupUIElements(HWND hwnd)
{
	// Buttons
	hButton_ok = CreateWindow(L"Button", L"Okay", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 30, 190, 100, 30,
							hwnd, (HMENU) BUTTON_ID, hInst, NULL);
	hButton_cancel = CreateWindow(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 160, 190, 100, 30,
							hwnd, (HMENU) (BUTTON_ID + 1), hInst, NULL); 
	SendMessage(hButton_ok, WM_SETFONT, (WPARAM)hFontText, TRUE);
	SendMessage(hButton_cancel, WM_SETFONT, (WPARAM)hFontText, TRUE);

	// Groups
	hGroup1 = CreateWindow(L"Button", L"Size", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 10, 280, 80,
							hwnd, (HMENU) (BUTTON_ID + 2), hInst, NULL);
	hGroup2 = CreateWindow(L"Button", L"Points", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 100, 280, 50,
							hwnd, (HMENU) (BUTTON_ID + 3), hInst, NULL);
	SendMessage(hGroup1, WM_SETFONT, (WPARAM)hFontGroup, TRUE);
	SendMessage(hGroup2, WM_SETFONT, (WPARAM)hFontGroup, TRUE);

	// Edits
	wstringstream ssx, ssy, ssn;
	ssx << cxClient;
	ssy << cyClient;
	if (ngen)
		ssn << ngen->getNumPoints();
	else
		ssn << 150;
	hEdit_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", ssx.str().c_str(), WS_CHILD | WS_VISIBLE | ES_NUMBER, 70, 30, 70, 22,
							hwnd, (HMENU) EDIT_ID, hInst, NULL);
	hEdit_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", ssy.str().c_str(), WS_CHILD | WS_VISIBLE | ES_NUMBER, 210, 30, 70, 22,
							hwnd, (HMENU) (EDIT_ID + 1), hInst, NULL);
	hEdit_num = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", ssn.str().c_str(), WS_CHILD | WS_VISIBLE | ES_NUMBER, 175, 120, 70, 22,
							hwnd, (HMENU) (EDIT_ID + 2), hInst, NULL);
	SendMessage(hEdit_x, WM_SETFONT, (WPARAM)hFontText, TRUE);
	SendMessage(hEdit_y, WM_SETFONT, (WPARAM)hFontText, TRUE);
	SendMessage(hEdit_num, WM_SETFONT, (WPARAM)hFontText, TRUE);

	// Radio
	hRadio = CreateWindow(L"Button", L" keep aspect ratio", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 55, 55, 150, 30,
							hwnd, (HMENU) RADIO_ID, hInst, NULL);
	SendMessage(hRadio, WM_SETFONT, (WPARAM)hFontText, TRUE);

	// Static
	hStatic_w = CreateWindow(L"Static", L"Width:", WS_CHILD | WS_VISIBLE | SS_SIMPLE, 20, 30, 100, 30,
							hwnd, (HMENU) STATIC_ID, hInst, NULL);
	hStatic_h = CreateWindow(L"Static", L"Height:", WS_CHILD | WS_VISIBLE | SS_SIMPLE, 150, 30, 100, 30,
							hwnd, (HMENU) STATIC_ID, hInst, NULL);
	hStatic_num = CreateWindow(L"Static", L"Number of points:", WS_CHILD | WS_VISIBLE | SS_SIMPLE, 20, 120, 150, 30,
							hwnd, (HMENU) STATIC_ID, hInst, NULL);
	SendMessage(hStatic_w, WM_SETFONT, (WPARAM)hFontText, TRUE);
	SendMessage(hStatic_h, WM_SETFONT, (WPARAM)hFontText, TRUE);
	SendMessage(hStatic_num, WM_SETFONT, (WPARAM)hFontText, TRUE);

	//Prog Bar
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControls();

	hProgBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | PBS_SMOOTH, 
						10, 160, 280, 20, hwnd, NULL, hInst, NULL);
	SendMessage(hProgBar, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
}

// Populates the back buffer with the desired color information for each pixel
void createBackBuffer(HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	// standard bitmap info header
	BITMAPINFO bitInfo;
	ZeroMemory(&bitInfo, sizeof(BITMAPINFO));
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biWidth = rc.right;
	bitInfo.bmiHeader.biHeight = -rc.bottom;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biBitCount = 24;
	bitInfo.bmiHeader.biCompression = BI_RGB;

	hBackBitmap = CreateDIBSection(hdc, &bitInfo, DIB_RGB_COLORS, (void**)&pColorBytes, NULL, NULL);

	int padding = (4 - 3*rc.right % 4) % 4;		//bitmaps are padded to multiples of 4 bytes
	int span = 3 * rc.right + padding;
	for (int y = 0; y < rc.bottom; y++)
	{
		for (int x = 0; x < rc.right; x++)
		{
			int byteStartIndex = y * span + 3 * x;
			int pixelIndex = y * rc.right + x;
			pColorBytes[byteStartIndex] = ngen->getBlueOfPixel(pixelIndex);				//b
			pColorBytes[byteStartIndex+1] = ngen->getGreenOfPixel(pixelIndex);			//g
			pColorBytes[byteStartIndex+2] = ngen->getRedOfPixel(pixelIndex);			//r		
		}
		int paddingIndex = (y+1)*span - padding;
		for (int i = 0; i < padding; i++)
		{
			pColorBytes[paddingIndex] = 0;		//fill padding with 0, if neccessary
			paddingIndex++;
		}
	}
}

// Copies the back buffer to the screen for display
void paintWindow(HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	HDC backBufferDC = CreateCompatibleDC(hdc);
	SelectObject(backBufferDC, hBackBitmap);

	BitBlt(hdc, 0, 0, rc.right, rc.bottom, backBufferDC, 0, 0, SRCCOPY);

	DeleteDC(backBufferDC);
	backBufferDC = 0;
}

// Callback function for the noise generator to use to update the progress bar
void updateProg()
{
	LRESULT res = SendMessage(hProgBar, PBM_DELTAPOS, 1, 0);
}

// Saves the screen image as a bitmap file
void saveFile()
{
	wchar_t filenameFull[MY_MAX_PATH];
	wchar_t filename[256];

	prepDefDirStr(filenameFull, L"\\worleyNoise.bmp", 16);
	
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filenameFull;
	ofn.nMaxFile = sizeof(filenameFull);
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle = sizeof(filename);
	ofn.lpstrFilter = L"bitmap (*.bmp)\0*.BMP\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		HANDLE file = CreateFile(filenameFull,GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file)
		{
			int padding = (4 - 3*cxClient % 4) % 4;
			int span = 3 * cxClient + padding;
			int size = cyClient * span;
			BITMAPFILEHEADER bmfh;
			ZeroMemory(&bmfh, sizeof(BITMAPFILEHEADER));
			bmfh.bfType = 0x4d42; // 'BM'
			bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			bmfh.bfSize = bmfh.bfOffBits + size;

			BITMAPINFOHEADER bmih;
			ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));
			bmih.biSize = sizeof(BITMAPINFOHEADER);
			bmih.biWidth = cxClient;
			bmih.biHeight = cyClient;
			bmih.biPlanes = 1;
			bmih.biBitCount = 24;
			bmih.biCompression = BI_RGB;
			bmih.biXPelsPerMeter = 0x0EC4;
			bmih.biYPelsPerMeter = 0x0EC4;

			BYTE* pWriteBytes = new BYTE[size];
			for (int y = 0; y < cyClient; y++)
			{
				int oldIndex = y * span;
				int newIndex = (cyClient-1 - y) * span;
				for (int x = 0; x < span; x++)
					pWriteBytes[newIndex+x] = pColorBytes[oldIndex+x];
			}

			unsigned long written;
			BOOL rc = WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &written, NULL);
			rc = WriteFile(file, &bmih, sizeof(BITMAPINFOHEADER), &written, NULL);
			rc = WriteFile(file, pWriteBytes, size, &written, NULL);

			delete[] pWriteBytes;
			CloseHandle(file);
		}
	}
}

// Presets the current directory and default file name
void prepDefDirStr(wchar_t* str, wchar_t* name, int len)
{
	wchar_t currentDir[MY_MAX_PATH] = L"\0";
	GetCurrentDirectory(MY_MAX_PATH, currentDir);

	for (int i = 0; i < MY_MAX_PATH; i++)
	{
		if (currentDir[i] != L'\0')
			str[i] = currentDir[i];
		else
		{
			for (int j = 0; j < len; j++)
				str[i+j] = name[j];
			str[i+len] = L'\0';
			break;
		}
	}
}