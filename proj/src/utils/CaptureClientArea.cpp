#include <QPixmap>

#if defined(Q_OS_WIN)
#include <QtWinExtras>

QPixmap CaptureClientArea(HWND hwnd)
{
	HDC hDC;
	HDC hDCMem;
	HBITMAP hBmOld;
	HBITMAP hBmMem;
	RECT rc;

	hDC = GetDC(hwnd);
	GetClientRect(hwnd, &rc);

	hDCMem = CreateCompatibleDC(hDC);
	hBmMem = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
	hBmOld = (HBITMAP)SelectObject(hDCMem, hBmMem);
	BitBlt(hDCMem, 0, 0, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
	SelectObject(hDCMem, hBmOld);

	DeleteDC(hDCMem);
	ReleaseDC(hwnd, hDC);

	QPixmap pixmap = QtWin::fromHBITMAP(hBmMem);

	DeleteObject(hBmMem);

	return pixmap;
}
#elif defined(Q_OS_LINUX)
#include <QtX11Extras/QtX11Extras>
#include <X11/Xlib.h>

QPixmap CaptureClientArea(Window hwnd)
{
	return QPixmap();
}
#endif
