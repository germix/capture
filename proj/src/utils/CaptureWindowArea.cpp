#include <QPixmap>

#if 1
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

#if defined(Q_OS_WIN)
#include <windows.h>
QPixmap CaptureWindowArea(HWND hwnd, const QRect& rc)
#else
#include <X11/Xlib.h>
QPixmap CaptureWindowArea(Window hwnd, const QRect& rc)
#endif
{
	Q_UNUSED(hwnd);
	QPixmap pixmap(rc.width()-1, rc.height()-1);
	QPixmap pixmapDesktop = QPixmap::grabWindow(QApplication::desktop()->winId());
	QPainter p(&pixmap);
	p.drawPixmap(0, 0, pixmapDesktop, rc.x(), rc.y(), rc.width()-1, rc.height()-1);
	return pixmap;
}
#else
#if defined(Q_OS_WIN)
#include <windows.h>
#include <QtWinExtras>

bool IsWindow1OrGreater();

QPixmap CaptureWindowArea(HWND hwnd, const QRect& rc)
{
	/*
	HDC hDC;
	HDC hDCMem;
	HBITMAP hBmOld;
	HBITMAP hBmMem;
	RECT rc;

	hDC = GetDC(hwnd);
#if 1
	GetWindowRect(hWndTarget, &rc);
#else
	GetCorrectWindowRect(hWndTarget, &rc);
#endif
	rc.right -= rc.left-1;
	rc.bottom -= rc.top-1;
	if(IsWindow1OrGreater())
	{
		WINDOWINFO wi;
		wi.cbSize = sizeof(wi);
		GetWindowInfo(hWndTarget, &wi);
		rc.right -= wi.cxWindowBorders*2 - 1;
		rc.bottom -= wi.cyWindowBorders;
	}

	hDCMem = CreateCompatibleDC(hDC);
	hBmMem = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
	hBmOld = (HBITMAP)SelectObject(hDCMem, hBmMem);
	if(IsWindow1OrGreater())
	{
		WINDOWINFO wi;
		wi.cbSize = sizeof(wi);
		GetWindowInfo(hWndTarget, &wi);
		BitBlt(hDCMem,
			   -wi.cxWindowBorders+1,
			   0,
			   rc.right+wi.cxWindowBorders,
			   rc.bottom+wi.cyWindowBorders+1,
			   hDC, 0, 0, SRCCOPY);
	}
	else
	{

		BitBlt(hDCMem, 0, 0, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
	}
	SelectObject(hDCMem, hBmOld);

	DeleteDC(hDCMem);
	ReleaseDC(hwnd, hDC);

	QPixmap pixmap = QtWin::fromHBITMAP(hBmMem);

	DeleteObject(hBmMem);

	return pixmap;
	*/

	QPixmap pixmap(rc.width(), rc.height());
	QPixmap pixmapDesktop = QPixmap::grabWindow(QApplication::desktop()->winId());
	QPainter p(&pixmap);
	p.drawPixmap(0, 0, pixmapDesktop, rc.x(), rc.y(), rc.width(), rc.height());
	return pixmap;
}
#elif defined(Q_OS_LINUX)
QPixmap CaptureWindowArea(Window hwnd, const QRect& rc)
{
	return QPixmap();
}
#endif


#endif
