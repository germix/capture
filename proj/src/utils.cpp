#include "utils.h"

#if 0
#include <dwmapi.h>
#include <windows.h>
#include <QtWinExtras>

bool IsWindow1OrGreater()
{
	OSVERSIONINFOW osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionExW(&osvi);

	if(osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2))
	{
		return true;
	}
	return false;
}

void GetCorrectWindowRect(HWND hWnd, RECT* prc)
{
	RECT rc;
#if 1
	HRESULT hr = 0;
	OSVERSIONINFOW osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionExW(&osvi);
	if(osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2))
		hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rc, sizeof(RECT));
	if(!SUCCEEDED(hr))
		GetWindowRect(hWnd, &rc);
#else
	WINDOWINFO wi;

	wi.cbSize = sizeof(wi);
	GetWindowInfo(hWnd, &wi);

	rc = wi.rcWindow;
	rc.left += wi.cxWindowBorders;
	rc.right -= wi.cxWindowBorders-1;
	rc.bottom -= wi.cyWindowBorders-1;

#endif

	*prc = rc;

	qDebug("%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
}

HBITMAP WindowToHBITMAP(HWND hWndTarget, BOOL bCaptureClient)
{
	if(IsWindow(hWndTarget))
	{
		HDC hDC;
		HDC hDCMem;
		HBITMAP hBmOld;
		HBITMAP hBmMem;
		RECT rcRect;

		if(bCaptureClient)
		{
			hDC = GetDC(hWndTarget);
			GetClientRect(hWndTarget, &rcRect);
		}
		else
		{
			hDC = GetWindowDC(hWndTarget);
#if 1
			GetWindowRect(hWndTarget, &rcRect);
#else
			GetCorrectWindowRect(hWndTarget, &rcRect);
#endif
			rcRect.right -= rcRect.left-1;
			rcRect.bottom -= rcRect.top-1;
			if(IsWindow1OrGreater())
			{
				WINDOWINFO wi;
				wi.cbSize = sizeof(wi);
				GetWindowInfo(hWndTarget, &wi);
				rcRect.right -= wi.cxWindowBorders*2 - 1;
				rcRect.bottom -= wi.cyWindowBorders;
			}
		}
		hDCMem = CreateCompatibleDC(hDC);
		hBmMem = CreateCompatibleBitmap(hDC, rcRect.right, rcRect.bottom);
		hBmOld = (HBITMAP)SelectObject(hDCMem, hBmMem);

		if(IsWindow1OrGreater())
		{
			WINDOWINFO wi;
			wi.cbSize = sizeof(wi);
			GetWindowInfo(hWndTarget, &wi);
			BitBlt(hDCMem,
				   -wi.cxWindowBorders+1,
				   0,
				   rcRect.right+wi.cxWindowBorders,
				   rcRect.bottom+wi.cyWindowBorders+1,
				   hDC, 0, 0, SRCCOPY);
		}
		else
		{

			BitBlt(hDCMem, 0, 0, rcRect.right, rcRect.bottom, hDC, 0, 0, SRCCOPY);
		}
		SelectObject(hDCMem, hBmOld);

		DeleteDC(hDCMem);
		ReleaseDC(hWndTarget, hDC);

		return hBmMem;
	}
	return NULL;
}

/*
BOOL CaptureHWnd(HWND hWndTarget, BOOL bCaptureClient)
{
	if(IsWindow(hWndTarget))
	{
		HDC hDC;
		HDC hDCMem;
		HBITMAP hBmOld;
		HBITMAP hBmMem;
		RECT rcRect;

		if(bCaptureClient)
		{
			hDC = GetDC(hWndTarget);
			GetClientRect(hWndTarget, &rcRect);
		}
		else
		{
			hDC = GetWindowDC(hWndTarget);
			GetWindowRect(hWndTarget, &rcRect);
			rcRect.right -= rcRect.left;
			rcRect.bottom -= rcRect.top;
		}
		hDCMem = CreateCompatibleDC(hDC);
		hBmMem = CreateCompatibleBitmap(hDC, rcRect.right, rcRect.bottom);
		hBmOld = (HBITMAP)SelectObject(hDCMem, hBmMem);

		BitBlt(hDCMem, 0, 0, rcRect.right, rcRect.bottom, hDC, 0, 0, SRCCOPY);
		SelectObject(hDCMem, hBmOld);

		if(OpenClipboard(0))
		{
			EmptyClipboard();
#ifdef SUPPORT_DIBS
#endif
			SetClipboardData(CF_BITMAP, hBmMem);
			CloseClipboard();
		}
		else
		{
			DeleteObject(hBmMem);
		}
		DeleteDC(hDCMem);
		ReleaseDC(hWndTarget, hDC);
		return TRUE;
	}
	return FALSE;
}
*/


//--------------------------------------------------------------------------------------------------

#include <QPixmap>
#include <Windows.h>

// https://forum.qt.io/topic/58973/capture-cursor/4
QPixmap CaptureCursor(int& px, int& py)
{
	CURSORINFO info;
	info.cbSize = sizeof(info);
	GetCursorInfo(&info);

	int width = GetSystemMetrics(SM_CXCURSOR);
	int height = GetSystemMetrics(SM_CYCURSOR);

	HDC hdc = GetDC(NULL);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);

	HDC bitmapDC = CreateCompatibleDC(hdc);
	HGDIOBJ prevObj = SelectObject(bitmapDC, bitmap);
	DrawIcon(bitmapDC, 0, 0, info.hCursor);

	QPixmap pixmap = QtWin::fromHBITMAP(bitmap, QtWin::HBitmapAlpha);

	SelectObject(bitmapDC, prevObj);
	DeleteObject(bitmap);
	DeleteDC(bitmapDC);
	ReleaseDC(NULL, hdc);

	px = info.ptScreenPos.x;
	py = info.ptScreenPos.y;

	return pixmap;
}

#endif
