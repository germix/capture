#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H
#include <QAbstractScrollArea>

class ImageView;
class ImageView_i;

class ImageView : public QAbstractScrollArea
{
	Q_OBJECT
	ImageView_i* d;
public:
	enum SIZEMODE
	{
		SIZEMODE_NORMAL,
		SIZEMODE_STRETCH,
		SIZEMODE_ASPECTRATIO,
	};

	enum
	{
		TOOL_HAND,
		TOOL_SELECTION,
	};
public:
	ImageView(QWidget* parent = 0);
	~ImageView();

public:
	void		wheelEvent(QWheelEvent* e);

	void		paintEvent(QPaintEvent* e);
	void		resizeEvent(QResizeEvent* e);

	void		keyPressEvent(QKeyEvent* e);
	void		mouseMoveEvent(QMouseEvent* e);
	void		mousePressEvent(QMouseEvent* e);
	void		mouseReleaseEvent(QMouseEvent* e);
public:
	int			getTool() const;
	void		setTool(int tool);

	QPixmap		getPixmap() const;
	void		loadPixmap(const QPixmap& pixmap);
	void		loadPixmap(const QString& filename);

	SIZEMODE	getSizeMode() const;
	void		setSizeMode(SIZEMODE mode);

	double		getScaleFactor() const;
	void		setScaleFactor(double factor);

	QColor		getBackColor() const;
	void		setBackColor(const QColor& color);

	QColor		getBorderColor() const;
	void		setBorderColor(const QColor& color);

	bool		isBorderVisible() const;
	void		setBorderVisible(bool visible);

	bool		hasImage() const;
	bool		hasSelection() const;
	QRect		getSelectionRect() const;
	QImage		getSelectionImage() const;

signals:
	void		imageChanged();
	void		scaleChanged(double s);
	void		onSelectionChanged(bool valid, const QRect& rect);

public slots:
	void		slotSelectionChanged(bool valid, const QRect& rect)
	{
		emit onSelectionChanged(valid, rect);
	}
	void		slotScrollBarValueChanged(int value);
};

#endif // IMAGEVIEW_H
