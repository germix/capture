#ifndef SELECTIONTRAFO_H
#define SELECTIONTRAFO_H
#include <QRect>
#include <QCursor>
#include <QObject>


enum TRAFOMODE
{
	TM_SCALE =0,    // The numbering is important, because these constants are also used as array indices.
	TM_ROTATE=1,
	TM_SHEAR =2
};
enum TRAFOHANDLE
{
	TH_NONE        =0,
	TH_BODY        =1,
//	TH_CENTER      =2,  // For future use, e.g. drag the origin of rotation.

	TH_TOP         =0x10,
	TH_BOTTOM      =0x20,
	TH_LEFT        =0x40,
	TH_RIGHT       =0x80,

	TH_TOP_LEFT    =TH_TOP    | TH_LEFT,
	TH_TOP_RIGHT   =TH_TOP    | TH_RIGHT,
	TH_BOTTOM_LEFT =TH_BOTTOM | TH_LEFT,
	TH_BOTTOM_RIGHT=TH_BOTTOM | TH_RIGHT
};


class QTimer;
class QWidget;
class QPainter;
class QMouseEvent;

class SelectionTrafo;

class SelectionTrafoTransformPoint
{
public:
	virtual ~SelectionTrafoTransformPoint() {}
public:
	virtual QRect toImage(const QRect& rc) = 0;
	virtual QRect toViewport(const QRect& rc) = 0;
	virtual QPoint toImage(const QPoint& pos) = 0;
	virtual QPoint toViewport(const QPoint& pos) = 0;
};

class SelectionTrafo : public QObject
{
	Q_OBJECT
public:
	QTimer*			timer;
	QPoint			pointE;
	QPoint			pointS;

	QRect			selectionRect;
	int				selectionState;

	QPoint			deltaSize;

	int				trafoMode;
	int				currentTrafoHandle;

	int				dashOffset;

	// ...
	QWidget*		canvas;
	QCursor			cursorRotate;
	QCursor			cursorSelection;

	SelectionTrafoTransformPoint* tp;
	bool			valid;

	QSize			frameSize;
public:
	enum
	{
		STATE_IDLE,
		STATE_SELECT,
		STATE_UNDECIDED,
		STATE_DRAG,
		STATE_TRANSFORM,
	};
public:
	SelectionTrafo(QWidget* w, SelectionTrafoTransformPoint* tp);
	virtual ~SelectionTrafo();
public:
public:
	void		onPaint(QPainter& p);

	void		onMouseMove(QMouseEvent* e);
	void		onMousePress(QMouseEvent* e);
	void		onMouseRelease(QMouseEvent* e);
public:
	void		drawLines(QPainter& p);
	void		drawHandles(QPainter& p);

	void		suggestCursor(int trafoHandle);
	TRAFOHANDLE	checkForHandle(QPoint point);
public:
	bool		isDragContents();
	bool		isTransformContents();
	void		setFrameSize(const QSize& size)
	{
		frameSize = size;
		clearSelection();
	}
	bool		hasSelection() const
	{
		return !selectionRect.isNull();
	}
	QRect		getSelectionRect() const;

	void		clearSelection();
private:
	void		setSelectionRect(const QRect& rc, bool update = true);
	void		setSelectionRect(const QPoint& p1, const QPoint& p2, bool update = true);
public slots:
	void		slotTimeout();
signals:
	void		onSelectionChanged(bool valid, const QRect& rect);
};

#endif // SELECTIONTRAFO_H
