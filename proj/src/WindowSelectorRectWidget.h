#ifndef WINDOWSELECTORRECTWIDGET_H
#define WINDOWSELECTORRECTWIDGET_H
#include <QWidget>

class WindowSelectorRectWidget : public QWidget
{
	Q_OBJECT
	int dir = 1;
	QColor currentColor;
public:
	WindowSelectorRectWidget(const QRect& rc, const QColor& color, QWidget* parent = 0);
	~WindowSelectorRectWidget();
private:
	void timerEvent(QTimerEvent* e);
	void paintEvent(QPaintEvent* e);
private:
	void drawRegionMarks(QPainter& p, const QColor& color, const QRect& rect);
};

#endif // WINDOWSELECTORRECTWIDGET_H
