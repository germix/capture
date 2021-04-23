#include <QtCore>

#if defined(Q_OS_WIN)
#include <windows.h>

void SetOnTopWindow(HWND hwnd)
{
	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>

void SetOnTopWindow(Window hwnd)
{
}
#endif
