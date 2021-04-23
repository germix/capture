#include "WindowSelectorRectWidget.h"
#include <QPainter>

#define BORDER_SIZE 10

WindowSelectorRectWidget::WindowSelectorRectWidget(const QRect& rc, const QColor& color, QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute( Qt::WA_TransparentForMouseEvents );
	setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);

	setGeometry(rc.x(), rc.y(), rc.width(), rc.height());

	currentColor = QColor(color.red(), color.green(), color.blue(), 150);
	startTimer(10);
	show();
}
WindowSelectorRectWidget::~WindowSelectorRectWidget()
{
}
void WindowSelectorRectWidget::timerEvent(QTimerEvent* e)
{
	Q_UNUSED(e);
	if(dir == -1)
	{
		if(currentColor.alpha() == 5)
			dir = +1;
		else
			currentColor.setAlpha(currentColor.alpha()-5);
	}
	else
	{
		if(currentColor.alpha() == 255)
			dir = -1;
		else
			currentColor.setAlpha(currentColor.alpha()+5);
	}
	update();
}
void WindowSelectorRectWidget::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);
	QPainter p(this);
	drawRegionMarks(p, currentColor, rect());
}
void WindowSelectorRectWidget::drawRegionMarks(QPainter& p, const QColor& color, const QRect& rect)
{
	p.fillRect(0, 0, rect.width(), BORDER_SIZE, color);
	p.fillRect(0, BORDER_SIZE, BORDER_SIZE, rect.height(), color);

	p.fillRect(rect.right()-BORDER_SIZE+1, rect.top()+BORDER_SIZE, BORDER_SIZE, rect.height(), color);
	p.fillRect(rect.left()+BORDER_SIZE, rect.bottom()-BORDER_SIZE+1, rect.width()-(BORDER_SIZE*2), BORDER_SIZE, color);
}
