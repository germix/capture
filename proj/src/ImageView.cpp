#include "ImageView.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>
#include <QFileInfo>


#include "ImageView.h"
#include "SelectionTrafo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "bsb.h"
QImage LoadKapImage(const char* filename)
{
	int x;
	int y;
	BSBImage bsb;
	uint8_t			*buf;

	if(!bsb_open_header(filename, &bsb))
	{
		return QImage();
	}
	buf = (uint8_t *)malloc(bsb.width);

	QImage image = QImage(bsb.width, bsb.height, QImage::Format_RGB32);
	uint *pixel = (uint *) image.scanLine(0);

	/* Read rows from bsb file and write rows to PPM */
	for (y = 0; y < bsb.height; y++)
	{
		bsb_seek_to_row(&bsb, y);
		bsb_read_row(&bsb, buf);

		/* Each pixel is a triplet of Red,Green,Blue samples */
		for (x = 0; x < bsb.width; x++)
		{
			QRgb c = qRgb(
					bsb.red[(int)buf[x]],
					bsb.green[(int)buf[x]],
					bsb.blue[(int)buf[x]]);

			*pixel++ = c;
		}
	}
	free(buf);

	bsb_close(&bsb);

	return image;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

enum DRAWALIGN
{
	DRAWALIGN_HCENTER		 = 0,
	DRAWALIGN_LEFT			 = 0x00000001,
	DRAWALIGN_RIGHT			 = 0x00000002,
	DRAWALIGN_VCENTER		 = 0,
	DRAWALIGN_TOP			 = 0x00000004,
	DRAWALIGN_BOTTOM		 = 0x00000008,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class ImageView_i : public SelectionTrafoTransformPoint
{
public:
	ImageView*					m_this;
	QPixmap*					m_pixmap;
	QWidget*					m_viewport;

	ImageView::SIZEMODE			m_sizeMode;
	QColor						m_backColor;
	QColor						m_borderColor;
	uint						m_drawAlign;
	bool						m_showBorder;
	double						m_scaleFactor;

	//
	// Mover imagen con el mouse
	//
	bool						m_dragging;
	QPoint						m_deltaSize;
	// ...
	int							tool;
	SelectionTrafo				trafo;
	QRect						imageRect;

	int							spaceSize;
	QRect						areaRect;

	QRect						workspaceRect;
public:
	ImageView_i(ImageView* t)
		: m_this(t)
		, m_pixmap(0)
		, m_viewport(0)
		// ...
		, m_sizeMode(ImageView::SIZEMODE_NORMAL)
		, m_backColor(Qt::white)
		, m_borderColor(Qt::black)
		, m_drawAlign(0)
		, m_showBorder(true)
		, m_scaleFactor(1.0)
		// ...
		, m_dragging(false)
		, trafo(t->viewport(), this)
	{
		tool = ImageView::TOOL_HAND;

		spaceSize = 10;
	}
	~ImageView_i()
	{
		if(m_pixmap)
			delete m_pixmap;
	}
public:
	void		updateScrollBars()
	{
		if(!m_pixmap || m_sizeMode != ImageView::SIZEMODE_NORMAL)
		{
			m_this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			m_this->setVerticalScrollBarPolicy  (Qt::ScrollBarAlwaysOff);
			return;
		}
		m_this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		m_this->setVerticalScrollBarPolicy  (Qt::ScrollBarAsNeeded);

		int hmax;
		int vmax;
		int width = m_viewport->width();
		int height = m_viewport->height();
		QScrollBar* hbar = m_this->horizontalScrollBar();
		QScrollBar* vbar = m_this->verticalScrollBar();

		if(spaceSize == 0)
		{
			if(!m_showBorder)
				hmax = qMax(0, int(m_pixmap->width() * m_scaleFactor) - width);
			else
				hmax = qMax(0, int((m_pixmap->width() * m_scaleFactor)+2) - width);

			if(!m_showBorder)
				vmax = qMax(0, int(m_pixmap->height() * m_scaleFactor) - height);
			else
				vmax = qMax(0, int((m_pixmap->height() * m_scaleFactor)+2) - height);
		}
		else
		{
			if(!m_showBorder)
				hmax = qMax(0, int((m_pixmap->width() * m_scaleFactor)+(spaceSize*2)) - width);
			else
				hmax = qMax(0, int((m_pixmap->width() * m_scaleFactor)+2+(spaceSize*2)) - width);

			if(!m_showBorder)
				vmax = qMax(0, int((m_pixmap->height() * m_scaleFactor)+(spaceSize*2)) - height);
			else
				vmax = qMax(0, int((m_pixmap->height() * m_scaleFactor)+2+(spaceSize*2)) - height);
		}
		hbar->setRange(0, hmax);
		hbar->setPageStep(width);
		vbar->setRange(0, vmax);
		vbar->setPageStep(height);
	}
	int			hScrollPos()
	{
		if(m_sizeMode == ImageView::SIZEMODE_NORMAL)
		{
			return m_this->horizontalScrollBar()->value();
		}
		return 0;
	}
	int			vScrollPos()
	{
		if(m_sizeMode == ImageView::SIZEMODE_NORMAL)
		{
			return m_this->verticalScrollBar()->value();
		}
		return 0;
	}
	void		buildImageRectNormal()
	{
		int x = 0;
		int y = 0;
		int sp = spaceSize + ((m_showBorder) ? 1 : 0);
		int iImageWidth = m_pixmap->width()*m_scaleFactor;
		int iImageHeight = m_pixmap->height()*m_scaleFactor;
		int iWindowWidth = m_viewport->width();
		int iWindowHeight = m_viewport->height();
		int iWorkspaceWidth = iImageWidth;
		int iWorkspaceHeight = iImageHeight;

		iWorkspaceWidth += sp*2;
		iWorkspaceHeight += sp*2;

		if(iWorkspaceWidth < iWindowWidth)
		{
			if(m_drawAlign & DRAWALIGN_LEFT)
				x = 0;
			else if(m_drawAlign & DRAWALIGN_RIGHT)
				x = iWindowWidth - iWorkspaceWidth;
			else
				x = (iWindowWidth - iWorkspaceWidth)/2;
		}
		else
		{
			x = -m_this->horizontalScrollBar()->value();
		}
		if(iWorkspaceHeight < iWindowHeight)
		{
			if(m_drawAlign & DRAWALIGN_TOP)
				y = 0;
			else if(m_drawAlign & DRAWALIGN_BOTTOM)
				y = iWindowHeight - iWorkspaceHeight;
			else
				y = (iWindowHeight - iWorkspaceHeight)/2;
		}
		else
		{
			y = -m_this->verticalScrollBar()->value();
		}
		imageRect = QRect(x+sp, y+sp, iImageWidth, iImageHeight);
		workspaceRect = QRect(x, y, iWorkspaceWidth, iWorkspaceHeight);
	}
	void		buildImageRectAspectRatio()
	{
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
		int sp = spaceSize + ((m_showBorder) ? 1 : 0);
		int iImageWidth = m_pixmap->width();
		int iImageHeight = m_pixmap->height();
		double dWindowWidth = m_viewport->width();
		double dWindowHeight = m_viewport->height();
		double dWorkspaceWidth = iImageWidth + (double)sp*2.0;
		double dWorkspaceHeight = iImageHeight + (double)sp*2.0;

		double dImageAspectRatio = dWorkspaceWidth / dWorkspaceHeight;
		double dWindowAspectRatio = dWindowWidth / dWindowHeight;

		if(dImageAspectRatio > dWindowAspectRatio)
		{
			long lNewHeight = long(dWindowWidth / dWorkspaceWidth * dWorkspaceHeight);

			x = 0;
			y = (dWindowHeight - lNewHeight) / 2;
			w = dWindowWidth;
			h = lNewHeight;
		}
		else if(dImageAspectRatio < dWindowAspectRatio)
		{
			long lNewWidth = long(dWindowHeight / dWorkspaceHeight * dWorkspaceWidth);

			x = (dWindowWidth - lNewWidth) / 2;
			y = 0;
			w = lNewWidth;
			h = dWindowHeight;
		}
		imageRect = QRect(x+sp, y+sp, w-(sp*2), h-(sp*2));
		workspaceRect = QRect(x, y, w, h);

		trafo.setFrameSize(imageRect.size());
	}
	void		buildImageRectStretch()
	{
		int x = 0;
		int y = 0;
		int w = m_viewport->width();
		int h = m_viewport->height();
		int sp = spaceSize + ((m_showBorder) ? 1 : 0);

		imageRect = QRect(x+sp, y+sp, w-(sp*2), h-(sp*2));
		workspaceRect = QRect(x, y, w, h);

		trafo.setFrameSize(imageRect.size());
	}
	void		buildImageRect()
	{
		if(m_pixmap == NULL)
		{
			return;
		}
		switch(m_sizeMode)
		{
			case ImageView::SIZEMODE_NORMAL:
				buildImageRectNormal();
				break;
			case ImageView::SIZEMODE_STRETCH:
				buildImageRectStretch();
				break;
			case ImageView::SIZEMODE_ASPECTRATIO:
				buildImageRectAspectRatio();
				break;
		}
	}
	QRect toImage(const QRect& rc)
	{
		return QRect(toImage(rc.topLeft()), toImage(rc.bottomRight()));
	}
	QRect toViewport(const QRect& rc)
	{
		return QRect(toViewport(rc.topLeft()), toViewport(rc.bottomRight()));
	}
	QPoint toImage(const QPoint& pos)
	{
		return (pos - imageRect.topLeft());
	}
	QPoint toViewport(const QPoint& pos)
	{
		return (pos + imageRect.topLeft());
	}
	void update(const QRegion& rgn)
	{
		m_viewport->update(rgn);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageView::ImageView(QWidget* parent)
	: QAbstractScrollArea(parent)
{
	d = new ImageView_i(this);
	d->m_pixmap = 0;
	d->m_viewport = viewport();

	setMouseTracking(true);
	connect(&d->trafo, SIGNAL(onSelectionChanged(bool,QRect)), this, SLOT(slotSelectionChanged(bool,QRect)));

	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBarValueChanged(int)));
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBarValueChanged(int)));
}
ImageView::~ImageView()
{
	delete d;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void		ImageView::wheelEvent(QWheelEvent* e)
{
	if(e->modifiers() & Qt::ShiftModifier)
		horizontalScrollBar()->setValue(horizontalScrollBar()->value()-e->delta());
	else
		verticalScrollBar()->setValue(verticalScrollBar()->value()-e->delta());
/*
	float f;

	f = d->m_scaleFactor + 0.001*e->delta();
	if(f < 32.0/d->m_pixmap.width())
		f = 32.0/d->m_pixmap.width();

	setScaleFactor(f);
*/
}
void		ImageView::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
	QPainter p(viewport());

#if 1
	p.fillRect(viewport()->rect(), d->m_backColor);
#else
	int s = 16;
	for(int y = 0; y < height(); y += s)
	{
		int i = ((y/s)&1);
		for(int x = 0; x < width(); x += s)
		{
			QRect rc(x, y, s, s);
			QColor color = (i&1) ? QColor(230, 230, 230) : Qt::white;
			p.fillRect(rc, color);
			i++;
		}
	}
#endif
	if(!d->m_pixmap)
	{
		return;
	}
	if(d->m_showBorder)
	{
		p.setPen(d->m_borderColor);
		p.drawRect(d->imageRect.x()-1,
				   d->imageRect.y()-1,
				   d->imageRect.width()+1,
				   d->imageRect.height()+1);
	}

	p.drawPixmap(d->imageRect.x(),
				 d->imageRect.y(),
				 d->imageRect.width(),
				 d->imageRect.height(),
				 *d->m_pixmap);
	//p.fillRect(d->workspaceRect, QColor(150, 50, 21, 40));

	if(d->tool == TOOL_SELECTION)
	{
		d->trafo.onPaint(p);
	}
}
void		ImageView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
	d->buildImageRect();
	d->updateScrollBars();
}
void		ImageView::keyPressEvent(QKeyEvent* e)
{
	switch(d->tool)
	{
		case TOOL_HAND:
			break;
		case TOOL_SELECTION:
			switch(e->key())
			{
				case Qt::Key_Escape:
					d->trafo.clearSelection();
					break;
				case Qt::Key_Enter:
				case Qt::Key_Return:
					break;
			}
			break;
	}
}
void		ImageView::mouseMoveEvent(QMouseEvent* e)
{
	switch(d->tool)
	{
		case TOOL_HAND:
			if(d->m_dragging)
			{
				verticalScrollBar()->setValue(-(e->globalPos() - d->m_deltaSize).y());
				horizontalScrollBar()->setValue(-(e->globalPos() - d->m_deltaSize).x());
			}
			break;
		case TOOL_SELECTION:
			d->trafo.onMouseMove(e);
			break;
	}

}
void		ImageView::mousePressEvent(QMouseEvent* e)
{
	switch(d->tool)
	{
		case TOOL_HAND:
			if(d->m_sizeMode == SIZEMODE_NORMAL)
			{
				if(d->m_pixmap && !d->m_pixmap->isNull())
				{
					d->m_viewport->grabMouse(QCursor(Qt::SizeAllCursor));

					d->m_dragging = true;
					d->m_deltaSize = e->globalPos() - QPoint(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
				}
			}
			break;
		case TOOL_SELECTION:
			d->trafo.onMousePress(e);
			break;
	}

}
void		ImageView::mouseReleaseEvent(QMouseEvent* e)
{
	switch(d->tool)
	{
		case TOOL_HAND:
			if(d->m_dragging)
			{
				d->m_viewport->releaseMouse();
				d->m_dragging = false;
			}
			break;
		case TOOL_SELECTION:
			d->trafo.onMouseRelease(e);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int			ImageView::getTool() const
{
	return d->tool;
}
void		ImageView::setTool(int tool)
{
	if(d->tool != tool)
	{
		d->tool = tool;
		d->trafo.clearSelection();
	}
}
void		ImageView::loadPixmap(const QPixmap& pixmap)
{
	if(d->m_pixmap)
	{
		delete d->m_pixmap;
		d->m_pixmap = NULL;
	}
	if(!pixmap.isNull())
	{
		d->m_pixmap = new QPixmap(pixmap);
	}
	d->m_scaleFactor = 1.0;

	d->updateScrollBars();
	d->m_viewport->update();

	d->buildImageRect();
	d->trafo.setFrameSize(d->imageRect.size());


	emit imageChanged();
	emit scaleChanged(d->m_scaleFactor);
}
QPixmap		ImageView::getPixmap() const
{
	if(d->m_pixmap == NULL)
		return QPixmap();
	return *d->m_pixmap;
}
void		ImageView::loadPixmap(const QString& filename)
{
#if 0
	QImage img = QImage(filename);
	QPixmap pixmap = QPixmap::fromImage(img);
	if(!pixmap.isNull())
	{
		loadPixmap(pixmap);
	}
#else
	QImage img;
	QFileInfo info(filename);

	if(info.suffix().toLower() == "kap")
	{
		img = LoadKapImage(filename.toLatin1());
	}
	else
	{
		img = QImage(filename);
	}
	QPixmap pixmap = QPixmap::fromImage(img);
	if(!pixmap.isNull())
	{
		loadPixmap(pixmap);
	}
#endif
}
ImageView::SIZEMODE	ImageView::getSizeMode() const
{
	return d->m_sizeMode;
}
void		ImageView::setSizeMode(SIZEMODE mode)
{
	if(d->m_sizeMode != mode)
	{
		d->m_sizeMode = mode;

		d->updateScrollBars();
		d->m_viewport->update();
	}
}
double		ImageView::getScaleFactor() const
{
	return d->m_scaleFactor;
}
void		ImageView::setScaleFactor(double factor)
{
	if(d->m_scaleFactor != factor && (factor > 0.001 && factor < 1000))
	{
#if 0
			double zoom = factor;
			double oldZoom = d->m_scaleFactor;

			QPoint center = QPoint(d->m_viewport->width() / 2, d->m_viewport->height() / 2);

			int left = qMax( (d->m_viewport->width() - d->m_pixmap->width()) / 2, 0);
			int top = qMax( (d->m_viewport->height() - d->m_pixmap->height()) / 2, 0);
			QPoint oldOffset = QPoint(left, top);

			QPointF oldScroll = QPointF(d->hScrollPos(), d->vScrollPos()) - oldOffset;

			QPointF scroll = (zoom / oldZoom) * (oldScroll + center) - center;

			d->updateScrollBars();
			horizontalScrollBar()->setValue(int(scroll.x()));
			verticalScrollBar()->setValue(int(scroll.y()));
		}
#endif
		d->m_scaleFactor = factor;

		d->updateScrollBars();
		d->m_viewport->update();

		emit scaleChanged(d->m_scaleFactor);
	}
}
QColor		ImageView::getBackColor() const
{
	return d->m_backColor;
}
void		ImageView::setBackColor(const QColor& color)
{
	if(d->m_backColor != color)
	{
		d->m_backColor = color;
		viewport()->update();
	}
}
QColor		ImageView::getBorderColor() const
{
	return d->m_borderColor;
}
void		ImageView::setBorderColor(const QColor& color)
{
	if(d->m_borderColor != color)
	{
		d->m_borderColor = color;
		viewport()->update();
	}
}
bool		ImageView::isBorderVisible() const
{
	return d->m_showBorder;
}
void		ImageView::setBorderVisible(bool visible)
{
	if(d->m_showBorder != visible)
	{
		d->m_showBorder = visible;
		d->buildImageRect();
		update();
		viewport()->update();
	}
}
bool		ImageView::hasImage() const
{
	return !d->m_pixmap->isNull();
}
bool		ImageView::hasSelection() const
{
	return d->trafo.hasSelection();
}
QRect		ImageView::getSelectionRect() const
{
	return d->trafo.getSelectionRect();
}
QImage		ImageView::getSelectionImage() const
{
	QRect r = d->trafo.getSelectionRect();
	QImage dst(r.width(), r.height(), QImage::Format_ARGB32);
	QImage src = d->m_pixmap->toImage();
	QPainter p(&dst);

	p.drawImage(0, 0, src, r.x(), r.y(), r.width(), r.height());

	return dst;
}
void		ImageView::slotScrollBarValueChanged(int value)
{
    Q_UNUSED(value);
	d->buildImageRect();
}


