#ifndef WINDOWSELECTOR_H
#define WINDOWSELECTOR_H
#include <QLabel>

class WindowSelector;
class WindowSelectorPrivate;

class WindowSelector : public QLabel
{
	Q_OBJECT
	WindowSelectorPrivate* d;
public:
	WindowSelector(QWidget* parent, int size, const QImage& image1, const QImage& image2);
	~WindowSelector();
public:
	QColor		getSelectionColor() const;
	void		setSelectionColor(const QColor& color);
public:
	QSize		sizeHint() const;
	QSize		minimumSizeHint() const;

	void		keyPressEvent(QKeyEvent* e);
	void		mouseMoveEvent(QMouseEvent* e);
	void		mousePressEvent(QMouseEvent* e);
	void		mouseReleaseEvent(QMouseEvent* e);
signals:
	void		onStartSelection(WId p);
	void		onChangeSelection(WId p);
	void		onCancelSelection(WId p);
	void		onFinishSelection(WId p, const QRect& rc);
};

#endif // WINDOWSELECTOR_H
