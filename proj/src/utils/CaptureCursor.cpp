#include <QPixmap>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <QtWinExtras>

// https://forum.qt.io/topic/58973/capture-cursor/4
QPixmap CaptureCursor(int& px, int& py)
{
	CURSORINFO ci;
	ci.cbSize = sizeof(ci);
	GetCursorInfo(&ci);

	ICONINFO ii;
	GetIconInfo(ci.hCursor, &ii);
	DeleteObject(ii.hbmMask);
	DeleteObject(ii.hbmColor);

	int width = GetSystemMetrics(SM_CXCURSOR);
	int height = GetSystemMetrics(SM_CYCURSOR);

	HDC hdc = GetDC(NULL);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);

	HDC bitmapDC = CreateCompatibleDC(hdc);
	HGDIOBJ prevObj = SelectObject(bitmapDC, bitmap);
	DrawIconEx(bitmapDC, 0, 0, ci.hCursor, width, height, 0, NULL, DI_NORMAL);

	QPixmap pixmap = QtWin::fromHBITMAP(bitmap, QtWin::HBitmapAlpha);

	SelectObject(bitmapDC, prevObj);
	DeleteObject(bitmap);
	DeleteDC(bitmapDC);
	ReleaseDC(NULL, hdc);

	px = ci.ptScreenPos.x - ii.xHotspot;
	py = ci.ptScreenPos.y - ii.yHotspot;

	return pixmap;
}

#else

QPixmap CaptureCursor(int& px, int& py)
{
	px = 0;
	py = 0;
	return QPixmap();
}

#endif
