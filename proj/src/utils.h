#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QByteArray>
#include <QPixmap>

QByteArray suffix(const QString& fileName);

#if 0
BOOL CaptureHWnd(HWND hWndTarget, BOOL bCaptureClient);

HBITMAP WindowToHBITMAP(HWND hWndTarget, BOOL bCaptureClient);

void GetCorrectWindowRect(HWND hWnd, RECT* prc);
#endif

QPixmap CaptureCursor(int& px, int& py);

#if defined(Q_OS_WIN)
#include <windows.h>
#include <dwmapi.h>

void SetOnTopWindow(HWND hwnd);
QPixmap CaptureClientArea(HWND hwnd);
QPixmap CaptureWindowArea(HWND hwnd, const QRect& rc);

#elif defined(Q_OS_LINUX)
#include <QtX11Extras/QtX11Extras>
#include <X11/Xlib.h>

void SetOnTopWindow(Window hwnd);
QPixmap CaptureClientArea(Window hwnd);
QPixmap CaptureWindowArea(Window hwnd, const QRect& rc);
#endif

#endif // UTILS_H
