#include "SelectionTrafo.h"

#include <QWidget>
#include <QBitmap>
#include <QTimer>
#include <QMouseEvent>
#include <QPainter>

static const TRAFOHANDLE HandleTable[3][3][3]=
{
	// The handles for trafo mode TM_SCALE.
	{
		{ TH_TOP_LEFT,    TH_TOP,    TH_TOP_RIGHT },
		{ TH_LEFT,        TH_NONE,   TH_RIGHT },
		{ TH_BOTTOM_LEFT, TH_BOTTOM, TH_BOTTOM_RIGHT }
	},

	// The handles for trafo mode TM_ROTATE.
	{
		{ TH_TOP_LEFT,    TH_NONE, TH_TOP_RIGHT },
		{ TH_NONE,        TH_NONE, TH_NONE },
		{ TH_BOTTOM_LEFT, TH_NONE, TH_BOTTOM_RIGHT }
	},

	// The handles for trafo mode TM_SHEAR.
	{
		{ TH_NONE, TH_TOP,    TH_NONE },
		{ TH_LEFT, TH_NONE,   TH_RIGHT },
		{ TH_NONE, TH_BOTTOM, TH_NONE }
	}
};
static const int HandleRadius  = 4;
static const int HandlePadding = 6;

///////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct _CURSORDATA
{
	long hx;
	long hy;
	long cx;
	long cy;
	uchar XorBits[128];
	uchar AndBits[128];
}CURSORDATA;

static CURSORDATA curdata_rotate =
{
	15,
	13,
	32,
	32,
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0xF0, 0x01, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x80, 0x07, 0x00, 0x02, 0x80, 0x00, 0x00, 0x02, 0x80, 0x00,
		0x00, 0x02, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x03, 0x70, 0x00,
		0x00, 0x02, 0x10, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0xF8, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0xFC, 0x03, 0x00,
		0x00, 0xFC, 0x01, 0x00, 0x00, 0x1F, 0x60, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x80, 0x03, 0xF8, 0x01,
		0x80, 0x03, 0xFC, 0x03, 0x80, 0x03, 0xE0, 0x07, 0x80, 0x03, 0xE0, 0x00, 0x80, 0x03, 0xE0, 0x00,
		0x80, 0x03, 0xE0, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x03, 0x70, 0x00, 0x00, 0x0F, 0x7C, 0x00,
		0x00, 0xFE, 0x1F, 0x00, 0x00, 0xFC, 0x0F, 0x00, 0x00, 0xF8, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	},
};
#ifdef CUSTOM_SELECTION_CURSOR
static CURSORDATA curdata_selection =
{
	12,
	12,
	32,
	32,
	{
		0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	},
};
#endif

QCursor CursorFromData(CURSORDATA* cur)
{
	return QCursor(
		QBitmap::fromData(QSize(cur->cx, cur->cy), cur->XorBits),
		QBitmap::fromData(QSize(cur->cx, cur->cy), cur->AndBits),
		cur->hx, cur->hy);
}

QRegion selRegion;

void addSelectionRectIntoRegion(SelectionTrafoTransformPoint* tp, int trafoMode, QRegion& rgn, QRect rc, bool handles = true)
{
	rc = tp->toViewport(rc);
	if(handles)
	{
		for(int y = 0; y < 3; y++)
		{
			for(int x = 0; x < 3; x++)
			{
				if(HandleTable[trafoMode][y][x] == TH_NONE)
					continue;
				const QPoint handlePos(
						rc.x() + rc.width()/2*x + (x-1)*HandlePadding + ((x == 2 && rc.width()&1) ? 1 : 0),
						rc.y() + rc.height()/2*y + (y-1)*HandlePadding + ((y == 2 && rc.height()&1) ? 1 : 0));
				const QRect  handleRect(handlePos.x()-HandleRadius, handlePos.y()-HandleRadius, 2*HandleRadius, 2*HandleRadius);

				rgn = rgn.united(handleRect.adjusted(-2, -2, 2, 2));
			}
		}
	}
	rgn = rgn.united(QRect(rc.left()-1, rc.y(), 2, rc.height()));
	rgn = rgn.united(QRect(rc.right()-1, rc.y(), 2, rc.height()));

	rgn = rgn.united(QRect(rc.x(), rc.top(), rc.width(), 2));
	rgn = rgn.united(QRect(rc.x(), rc.bottom(), rc.width(), 2));

	//rgn = rgn.united(rc.adjusted(-1, -1, 1, 1));
}

SelectionTrafo::SelectionTrafo(QWidget* w, SelectionTrafoTransformPoint* _tp)
	:
	timer(NULL),
	pointE(QPoint()),
	pointS(QPoint()),
	selectionRect(QRect()),
	selectionState(STATE_IDLE),
	trafoMode(TM_SCALE),
	dashOffset(0),
	canvas(w),
	tp(_tp)
{
	valid = false;
	// ...
	cursorRotate = CursorFromData(&curdata_rotate);
#ifdef CUSTOM_SELECTION_CURSOR
	cursorSelection = CursorFromData(&curdata_selection);
#else
	cursorSelection = QCursor(Qt::CrossCursor);
#endif

	timer = new QTimer();
	timer->setInterval(50);
	connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}
SelectionTrafo::~SelectionTrafo()
{
}
void		SelectionTrafo::onPaint(QPainter& p)
{
/*
	p.save();
	p.setClipRegion(selRegion);
	p.fillRect(selectionRect, Qt::red);
	p.restore();
*/
	if(valid)
	{
		drawLines(p);
		drawHandles(p);
	}

}
void		SelectionTrafo::onMouseMove(QMouseEvent* e)
{
	if(frameSize.isEmpty())
	{
		canvas->setCursor(Qt::ArrowCursor);
		return;
	}
	QPoint pos = tp->toImage(e->pos());
//	Qt::MouseButton button = e->button();
	//if(button == Qt::LeftButton)
	{
		switch(selectionState)
		{
			case STATE_IDLE:
				{
					suggestCursor(checkForHandle(pos));
				}
				break;
			case STATE_SELECT:
				{
					setSelectionRect(pointS, (pointE = pos), true);
				}
				break;
			case STATE_UNDECIDED:
				{
					deltaSize = QPoint(pos.x() - selectionRect.x(), pos.y() - selectionRect.y());
					selectionState = STATE_DRAG;
				}
				//
				// continuar
				//
			case STATE_DRAG:
				{
					QRect rect = selectionRect;

					rect.moveTopLeft(pos - deltaSize);
					// ...
					if(!isDragContents())
					{
						if(rect.left() < 0)						rect.translate(-rect.left(), 0);
						if(rect.top() < 0)						rect.translate(0, -rect.top());
						if(rect.right() > frameSize.width())	rect.translate(-(rect.right()-frameSize.width()), 0);
						if(rect.bottom() > frameSize.height())	rect.translate(0, -(rect.bottom()-frameSize.height()));
					}
					else
					{
						if(rect.left() > frameSize.width())		rect.translate(-(rect.left()-frameSize.width()), 0);
						if(rect.top() > frameSize.height())		rect.translate(0, -(rect.top()-frameSize.height()));
						if(rect.right() < 0)					rect.translate(-rect.right(), 0);
						if(rect.bottom() < 0)					rect.translate(0, -rect.bottom());
					}
					setSelectionRect(rect);
				}
				break;
			case STATE_TRANSFORM:
				{
					QRect rect = selectionRect;

					if(!isTransformContents())
					{
						if(currentTrafoHandle & TH_LEFT)
						{
							rect.setLeft(qMax(0, qMin(pos.x(), rect.right())));
						}
						else if(currentTrafoHandle & TH_RIGHT)
						{
							rect.setRight(qMin(frameSize.width(), qMax(pos.x(), rect.left())));
						}
						if(currentTrafoHandle & TH_TOP)
						{
							rect.setTop(qMax(0, qMin(pos.y(), rect.bottom())));
						}
						else if(currentTrafoHandle & TH_BOTTOM)
						{
							rect.setBottom(qMin(frameSize.height(), qMax(pos.y(), rect.top())));
						}
					}
					else
					{
						if(currentTrafoHandle & TH_LEFT)
						{
							rect.setLeft(qMin(pos.x(), qMin(rect.right(), frameSize.width())));
						}
						else if(currentTrafoHandle & TH_RIGHT)
						{
							rect.setRight(qMax(pos.x(), qMax(0, rect.left())));
						}
						if(currentTrafoHandle & TH_TOP)
						{
							rect.setTop(qMin(pos.y(), qMin(rect.bottom(), frameSize.height())));
						}
						else if(currentTrafoHandle & TH_BOTTOM)
						{
							rect.setBottom(qMax(pos.y(), qMax(0, rect.top())));
						}
					}
					setSelectionRect(rect);
				}
				break;
		}
	}
}
void		SelectionTrafo::onMousePress(QMouseEvent* e)
{
	if(frameSize.isEmpty())
	{
		return;
	}
	QPoint pos = tp->toImage(e->pos());
	Qt::MouseButton button = e->button();
	if(button == Qt::LeftButton)
	{
		TRAFOHANDLE trafoHandle = checkForHandle(pos);

		pointS = pos;
		pointE = pos;
		if(timer) timer->stop();
		suggestCursor(trafoHandle);

		valid = true;
		switch(trafoHandle)
		{
			case TH_NONE:
				selectionState = STATE_SELECT;
				setSelectionRect(pos, pos);
				break;
			case TH_BODY:
				selectionState = STATE_UNDECIDED;
				break;
			default:
				selectionState = STATE_TRANSFORM;
				currentTrafoHandle = trafoHandle;
				break;
		}
	}
}
void		SelectionTrafo::onMouseRelease(QMouseEvent* e)
{
	if(frameSize.isEmpty())
	{
		return;
	}
	QPoint pos = tp->toImage(e->pos());
	Qt::MouseButton button = e->button();
	if(button == Qt::LeftButton)
	{
		switch(selectionState)
		{
			case STATE_IDLE:
				break;
			case STATE_SELECT:
				pointE = pos;
				canvas->update();
				//
				// continuar
				//
			case STATE_DRAG:
			case STATE_TRANSFORM:
				selectionState = STATE_IDLE;
				currentTrafoHandle = TH_NONE;
				break;
			case STATE_UNDECIDED:
				selectionState = STATE_IDLE;
				break;
		}
		if(!selectionRect.isNull())
		{
			if(timer) timer->start();
		}
	}
}
void		SelectionTrafo::drawLines(QPainter& p)
{
	QPen pen;
	QBrush brush;
	QVector<qreal> dashes;
	QRect rect = selectionRect.adjusted(0, 0, -1, -1);

	rect = tp->toViewport(rect);
	dashes << 4 << 4;
	pen.setDashPattern(dashes);
	p.setBrush(Qt::NoBrush);

	for(int i = 0; i < 2; i++)
	{
		if(i == 0)
		{
			pen.setColor(Qt::black);
			pen.setDashOffset(dashOffset);
		}
		else
		{
			pen.setColor(Qt::white);
			pen.setDashOffset(dashOffset+4);
		}
		p.setPen(pen);

		p.drawRect(rect);
	}
}
void		SelectionTrafo::drawHandles(QPainter& p)
{
	QPen pen;
	QBrush brush;

	QRect rect = selectionRect.adjusted(0, 0, -1, -1);

	rect = tp->toViewport(rect);

	pen = QPen(QColor(80, 80, 80));
	brush = QBrush(QColor(184, 188, 205));
	p.setPen(pen);
	p.setBrush(brush);

	for(int y = 0; y < 3; y++)
	{
		for(int x = 0; x < 3; x++)
		{
			if(HandleTable[trafoMode][y][x] == TH_NONE)
				continue;
//			const QPoint handlePos(
//					rect.x() + rect.width()/2*x + (x-1)*HandlePadding,
//					rect.y() + rect.height()/2*y + (y-1)*HandlePadding);
			const QPoint handlePos(
					rect.x() + rect.width()/2*x + (x-1)*HandlePadding + ((x == 2 && rect.width()&1) ? 1 : 0),
					rect.y() + rect.height()/2*y + (y-1)*HandlePadding + ((y == 2 && rect.height()&1) ? 1 : 0));
			const QRect  handleRect(handlePos.x()-HandleRadius, handlePos.y()-HandleRadius, 2*HandleRadius, 2*HandleRadius);

			if(trafoMode == TM_ROTATE)
			{
				p.drawEllipse(handleRect);
			}
			else
			{
				p.drawRect(handleRect);
			}
		}
	}
}
void		SelectionTrafo::suggestCursor(int trafoHandle)
{
	if(trafoHandle == TH_BODY)
	{
		canvas->setCursor(QCursor(Qt::SizeAllCursor));
		return;
	}
	if(trafoMode == TM_SCALE)
	{
		switch(trafoHandle)
		{
			case TH_TOP_LEFT:
			case TH_BOTTOM_RIGHT:
				canvas->setCursor(QCursor(Qt::SizeFDiagCursor));
				return;
			case TH_TOP_RIGHT:
			case TH_BOTTOM_LEFT:
				canvas->setCursor(QCursor(Qt::SizeBDiagCursor));
				return;
			case TH_RIGHT:
			case TH_LEFT:
				canvas->setCursor(QCursor(Qt::SizeHorCursor));
				return;
			case TH_TOP:
			case TH_BOTTOM:
				canvas->setCursor(QCursor(Qt::SizeVerCursor));
				return;
			default:
				break;
		}
	}
	if(trafoMode == TM_ROTATE)
	{
		canvas->setCursor(cursorRotate);
		return;
	}
	if(trafoMode == TM_SHEAR)
	{
		switch(trafoHandle)
		{
			case TH_RIGHT:
			case TH_LEFT:
				canvas->setCursor(QCursor(Qt::SizeVerCursor));
				return;
			case TH_TOP:
			case TH_BOTTOM:
				canvas->setCursor(QCursor(Qt::SizeHorCursor));
				return;
			default:
				break;
		}
	}
	canvas->setCursor(cursorSelection);
}
TRAFOHANDLE	SelectionTrafo::checkForHandle(QPoint point)
{
	QRect rect = selectionRect;

	if(rect.isNull())
	{
		return TH_NONE;
	}
//	rect.rRight()--;
//	rect.rBottom()--;

	if(rect.contains(point))
	{
		return TH_BODY;
	}
	//
	// Chequear sobre cual handle esta el punto
	//
	int         bestDist   = HandleRadius*HandleRadius+1;
	TRAFOHANDLE bestHandle = TH_NONE;

	for(int y = 0; y < 3; y++)
	{
		for(int x = 0; x < 3; x++)
		{
			if(HandleTable[trafoMode][y][x] == TH_NONE)
				continue;
//			const QPoint handlePos(
//					rect.x() + rect.width()/2*x + (x-1)*HandlePadding,
//					rect.y() + rect.height()/2*y + (y-1)*HandlePadding);
			const QPoint handlePos(
					rect.x() + rect.width()/2*x + (x-1)*HandlePadding + ((x == 2 && rect.width()&1) ? 1 : 0),
					rect.y() + rect.height()/2*y + (y-1)*HandlePadding + ((y == 2 && rect.height()&1) ? 1 : 0));
//			const QRect  handleRect(handlePos.x()-HandleRadius, handlePos.y()-HandleRadius, 2*HandleRadius, 2*HandleRadius);

			const QPoint delta = handlePos-point;
			const int    dist  = delta.x()*delta.x() + delta.y()*delta.y();

			if(dist < bestDist)
			{
				bestDist   = dist;
				bestHandle = HandleTable[trafoMode][y][x];
			}
		}
	}
	return bestHandle;
}
bool		SelectionTrafo::isDragContents()
{
	return false;
}
bool		SelectionTrafo::isTransformContents()
{
	return false;
}
QRect		SelectionTrafo::getSelectionRect() const
{
	return selectionRect;
}
void		SelectionTrafo::clearSelection()
{
	valid = false;
	timer->stop();
	setSelectionRect(QRect());
}
void		SelectionTrafo::setSelectionRect(const QRect& rc, bool update)
{
	if(selectionRect != rc)
	{
		QRegion rgn;

		if(update)
			addSelectionRectIntoRegion(tp, trafoMode, rgn, selectionRect);

		selectionRect = rc;

		if(update)
		{
			addSelectionRectIntoRegion(tp, trafoMode, rgn, selectionRect);
			canvas->update(rgn);
			selRegion = rgn;
		}
		emit onSelectionChanged(valid, selectionRect);
	}
}
void		SelectionTrafo::setSelectionRect(const QPoint& p1, const QPoint& p2, bool update)
{
	Q_UNUSED(update);
	int   x1 = qBound(0, qMin(p1.x(), p2.x()), frameSize.width()+1);
	int   y1 = qBound(0, qMin(p1.y(), p2.y()), frameSize.height()+1);
	int   x2 = qBound(0, qMax(p1.x(), p2.x()), frameSize.width()+1);
	int   y2 = qBound(0, qMax(p1.y(), p2.y()), frameSize.height()+1);
	setSelectionRect(QRect(x1, y1, x2-x1, y2-y1));
}
void		SelectionTrafo::slotTimeout()
{
	QRegion rgn;

	dashOffset++;
	if(dashOffset == 8)
		dashOffset = 0;

	addSelectionRectIntoRegion(tp, trafoMode, rgn, selectionRect, false);
	canvas->update(rgn);
}
