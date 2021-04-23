#include "WindowSelector.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include "WindowSelectorRectWidget.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <dwmapi.h>

QRect GetRectOfWindowFromMousePosition(HWND* hwnd)
{
	POINT pt;
	HWND hWndInPoint;
	QRect rectReturn;

	GetCursorPos(&pt);
	hWndInPoint = WindowFromPoint(pt);
	if(hWndInPoint != NULL)
	{
		RECT rc;
		HRESULT hr;
		OSVERSIONINFOW osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionExW(&osvi);
#if 0
		qDebug("osvi.dwMajorVersion: %d", osvi.dwMajorVersion);
		qDebug("osvi.dwMinorVersion: %d", osvi.dwMinorVersion);
#endif
		hr = -1;
		if(osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2))
		{
			hr = DwmGetWindowAttribute(hWndInPoint, DWMWA_EXTENDED_FRAME_BOUNDS, &rc, sizeof(RECT));
		}
		if(!SUCCEEDED(hr))
		{
			GetWindowRect(hWndInPoint, &rc);
		}
		rectReturn.setCoords(rc.left, rc.top, rc.right, rc.bottom);
	}
	*hwnd = hWndInPoint;
	return rectReturn;
}
#else
#include <QtX11Extras/QX11Info>
#include <X11/Xlib.h>
QRect GetRectOfWindowFromMousePosition(Window* hwnd)
{

	Display* display;
	Window root;
	Window ret_root;
	Window ret_child;
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	unsigned int mask;
	QRect rectReturn;

	display = XOpenDisplay(NULL);
	root = XDefaultRootWindow(display);
	if(XQueryPointer(display,
					 root,
					 &ret_root,
					 &ret_child,
					 &root_x,
					 &root_y,
					 &win_x,
					 &win_y,
					 &mask))
	{
		qDebug("+%d+%d", root_x, root_y);
		qDebug("ret_root: %p", ret_root);
		qDebug("ret_child: %p", ret_child);

		int x, y;
		int w, h;
		//unsigned int w, h, bw, depth;
		//XGetGeometry(display, ret_child, ret_child, &x, &y, &w, &h, &bw, &depth);

		XWindowAttributes attr;
		XGetWindowAttributes(display, ret_child, &attr);
#if 1
		x = attr.x;
		y = attr.y;
		w = attr.width;
		h = attr.height;
#else
		x = attr.x+9;
		y = attr.y+8;
		w = attr.width-17;
		h = attr.height-16;
#endif
		rectReturn = QRect(x, y, w, h);
	}
	XCloseDisplay(display);

	*hwnd = ret_child;

	return rectReturn;
}
#endif


class WindowSelectorPrivate
{
public:
	bool						pressed;
	int							size;
	QImage						image1;
	QImage						image2;
	QRect						selectedRect;
	WindowSelectorRectWidget*	selectedRectWidget;
	QColor						selectionColor;

#if defined(Q_OS_WIN)
	HWND						target;
	HWND						previousTarget;
#else
	Window						target;
	Window						previousTarget;
#endif
public:
	WindowSelectorPrivate()
		: pressed(false)
		, selectedRectWidget(0)
		, selectionColor(Qt::red)
#if defined(Q_OS_WIN)
		, target(0)
		, previousTarget(0)
#elif defined(Q_OS_LINUX)
		, target(-1)
		, previousTarget(-1)
#endif
	{
	}
	~WindowSelectorPrivate()
	{
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
WindowSelector::WindowSelector(QWidget* parent, int size, const QImage& image1, const QImage& image2) : QLabel(parent)
{
	d = new WindowSelectorPrivate();
	setCursor(QCursor(Qt::SizeAllCursor));
	setFrameShadow(QFrame::Sunken);
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	d->size = size;
	d->image1 = image1;
	d->image2 = image2;

	setPixmap(QPixmap::fromImage(image1));
}
WindowSelector::~WindowSelector()
{
	delete d;
}
QColor		WindowSelector::getSelectionColor() const
{
	return d->selectionColor;
}
void		WindowSelector::setSelectionColor(const QColor& color)
{
	d->selectionColor = color;
}
QSize		WindowSelector::sizeHint() const
{
	return QSize(d->size, d->size);
}
QSize		WindowSelector::minimumSizeHint() const
{
	return sizeHint();
}
void		WindowSelector::keyPressEvent(QKeyEvent* e)
{
	if(e->key() == Qt::Key_Escape)
	{
		d->pressed = false;
		setPixmap(QPixmap::fromImage(d->image1));
		releaseMouse();
		releaseKeyboard();

		if(d->selectedRectWidget)
		{
			delete d->selectedRectWidget;
			d->selectedRect = QRect();
			d->selectedRectWidget = 0;
		}

		emit onCancelSelection((WId)d->previousTarget);
		d->target = NULL;
	}
}
void		WindowSelector::mouseMoveEvent(QMouseEvent* e)
{
	Q_UNUSED(e);
	if(d->pressed)
	{
		QRect rc = GetRectOfWindowFromMousePosition(&d->target);
		if(rc != d->selectedRect)
		{
			d->selectedRect = rc;
			if(d->selectedRectWidget)
				delete d->selectedRectWidget;

			d->selectedRectWidget = new WindowSelectorRectWidget(d->selectedRect, d->selectionColor);

			emit onChangeSelection((WId)d->target);
		}
		setCursor(QCursor(Qt::SizeAllCursor));
	}
}
void		WindowSelector::mousePressEvent(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		d->pressed = true;
		setPixmap(QPixmap::fromImage(d->image2));
		grabMouse();
		grabKeyboard();

		setCursor(Qt::WaitCursor);
		emit onStartSelection(0);
		setCursor(QCursor(Qt::SizeAllCursor));

		mouseMoveEvent(e);
	}
}
void		WindowSelector::mouseReleaseEvent(QMouseEvent* e)
{
	Q_UNUSED(e);
	if(d->pressed)
	{
		d->pressed = false;
		setPixmap(QPixmap::fromImage(d->image1));
		releaseMouse();
		releaseKeyboard();

		QRect rc = d->selectedRect;
		if(d->selectedRectWidget)
		{
			delete d->selectedRectWidget;
			d->selectedRect = QRect();
			d->selectedRectWidget = 0;
		}
		emit onFinishSelection((WId)d->target, rc);
		d->previousTarget = d->target;
		d->target = NULL;
	}
}
