#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "utils.h"
#include "ImageView.h"
#include "WindowSelector.h"
#include "AboutDialog.h"

#include <QSettings>
#include <QFileDialog>
#include <QImageWriter>
#include <QClipboard>
#include <QInputDialog>
#include <QColorDialog>
#include <QPainter>
#include <QDesktopWidget>
#include <QTimer>

#include <QThread>

#define SETTINGS_APPLICATION "Capture"
#define SETTINGS_ORGANIZATION "Germix"

#define DEF_AUTO_HIDE_DELAY		1500
#define MIN_AUTO_HIDE_DELAY		0
#define MAX_AUTO_HIDE_DELAY		10000

#define AUTO_MINIMIZE_DELAY		300

QImage ColorToImage(const QColor& color, int size)
{
	QImage image(size, size, QImage::Format_RGB32);
	image.fill(color.rgb());
	return image;
}
QPixmap ColorToPixmap(const QColor& color, int size)
{
	return QPixmap::fromImage(ColorToImage(color, size));
}
QIcon ColorToIcon(const QColor& color, int size = 16)
{
	return QIcon(ColorToPixmap(color, size));
}

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->actionOptionsAutoMinimize->setVisible(false);
	// ...
	QAction* action =
			ui->mainToolBar->addWidget(windowSelector = new WindowSelector(
			this,
			24,
			QImage(":/capture-window.png"),
			QImage(":/capture-window-on.png")));
	action->setToolTip(tr("Capture window"));
	connect(windowSelector, SIGNAL(onStartSelection(WId)), this, SLOT(slotWindowStartSelection(WId)));
	connect(windowSelector, SIGNAL(onChangeSelection(WId)), this, SLOT(slotWindowChangeSelection(WId)));
	connect(windowSelector, SIGNAL(onCancelSelection(WId)), this, SLOT(slotWindowCancelSelection(WId)));
	connect(windowSelector, SIGNAL(onFinishSelection(WId,QRect)), this, SLOT(slotWindowFinishSelection(WId,QRect)));
	// ...
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addAction(ui->actionFileSave);
	ui->mainToolBar->addAction(ui->actionFileSaveSelection);

	setCentralWidget(imageView = new ImageView());
	imageView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(imageView, SIGNAL(onSelectionChanged(bool,QRect)), this, SLOT(slotImageViewSelectionChanged(bool,QRect)));
	connect(imageView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotImageViewCustomContextMenuRequested(QPoint)));

	// ...
	QActionGroup* ag = new QActionGroup(this);
	ag->addAction(ui->actionOptionsCaptureClientArea);
	ag->addAction(ui->actionOptionsCaptureWindowArea);
	// ...
	ui->menu_Options->addSeparator();
	ui->menu_Options->addAction(ui->mainToolBar->toggleViewAction());

	//
	// Cargar configuración
	//
	QSettings s(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
	restoreGeometry(s.value("WindowGeometry").toByteArray());
	restoreState(s.value("WindowState").toByteArray());
	// ...
	ui->actionOptionsAutoHide->setChecked(s.value("AutoHide", true).toBool());
	ui->actionOptionsAutoCopy->setChecked(s.value("AutoCopy", true).toBool());
	captureDesktopDelay = s.value("AutoHideDelay", DEF_AUTO_HIDE_DELAY).toInt();
	ui->actionOptionsCaptureCursor->setChecked(s.value("CaptureCursor", false).toBool());
	if(!(captureDesktopDelay >= 0 && captureDesktopDelay <= MAX_AUTO_HIDE_DELAY))
	{
		captureDesktopDelay = DEF_AUTO_HIDE_DELAY;
	}
	// ...
	switch(s.value("CopyWindowMode", 0).toInt())
	{
		case 0:	ui->actionOptionsCaptureClientArea->setChecked(true);	break;
		case 1:	ui->actionOptionsCaptureWindowArea->setChecked(true);	break;
	}
	ui->actionOptionsAutoMinimize->setChecked(s.value("AutoMinimize", false).toBool());
	// ...
	ui->actionOptionsShowBorder->setChecked(s.value("ShowBorder", false).toBool());
	imageView->setBorderVisible(ui->actionOptionsShowBorder->isChecked());
	imageView->setBorderColor(QColor(s.value("BorderColor", "black").toString()));
	imageView->setBackColor(QColor(s.value("BackgroundColor", "white").toString()));
	// ...
	windowSelector->setSelectionColor(QColor(s.value("SelectionColor", "red").toString()));
	// ...
	languages.init(ui->menu_Languages, "translations", "capture", s.value("Language").toString());

	ui->actionOptionsBorderColor->setIcon(ColorToIcon(imageView->getBorderColor()));
	ui->actionOptionsBackgroundColor->setIcon(ColorToIcon(imageView->getBackColor()));
	ui->actionOptionsSelectionColor->setIcon(ColorToIcon(windowSelector->getSelectionColor()));
}
MainWindow::~MainWindow()
{
	//
	// Guardar la configuración
	//
	QSettings s(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
	s.setValue("AutoCopy", ui->actionOptionsAutoCopy->isChecked());
	s.setValue("AutoHide", ui->actionOptionsAutoHide->isChecked());
	s.setValue("AutoHideDelay", captureDesktopDelay);
	s.setValue("CaptureCursor", ui->actionOptionsCaptureCursor->isChecked());

	// ...
	if(ui->actionOptionsCaptureClientArea->isChecked())
		s.setValue("CopyWindowMode", 0);
	else
		s.setValue("CopyWindowMode", 1);
	s.setValue("AutoMinimize", ui->actionOptionsAutoMinimize->isChecked());

	// ...
	s.setValue("ShowBorder", ui->actionOptionsShowBorder->isChecked());
	s.setValue("BorderColor", imageView->getBorderColor().name());
	s.setValue("BackgroundColor", imageView->getBackColor().name());
	s.setValue("SelectionColor", windowSelector->getSelectionColor().name());

	// ...
	s.setValue("Language", languages.language());

	// ...
	s.setValue("WindowState", saveState());
	s.setValue("WindowGeometry", saveGeometry());
	// ...
	delete ui;
}
void MainWindow::saveImage(const QImage& image)
{
	if(!image.isNull())
	{
		QString fileName;
		QStringList nameFilters;
		QList<QByteArray> supportedFormats = QImageWriter::supportedImageFormats();

		for(int i = 0; i < supportedFormats.size(); i++)
		{
			nameFilters << "*." + QString(supportedFormats.at(i));
		}
		fileName = QFileDialog::getSaveFileName(this,
						tr("Save image"),
						QString(),
						tr("Image Files (%1)").arg(nameFilters.join(" ")));

		if(!(fileName.isEmpty()))
		{
			QImageWriter writer(fileName, suffix(fileName));

			writer.write(image);
		}
	}
}
void MainWindow::loadPixmap(const QPixmap& pixmap)
{
	imageView->loadPixmap(pixmap);

	ui->actionFileSave->setEnabled(!pixmap.isNull());
	ui->actionEditCopy->setEnabled(!pixmap.isNull());
	ui->actionEditClear->setEnabled(!pixmap.isNull());
#if 1
	ui->actionFileSaveSelection->setEnabled(false);
#else
	ui->actionFileSaveSelection->setEnabled(!pixmap.isNull());
#endif
	if(ui->actionOptionsAutoHide->isChecked())
	{
		show();
	}
	if(ui->actionOptionsAutoCopy->isChecked())
	{
		QApplication::clipboard()->setPixmap(pixmap);
	}
}
void MainWindow::slotFileExit()
{
	close();
}
void MainWindow::slotFileSave()
{
	saveImage(imageView->getPixmap().toImage());
}
void MainWindow::slotFileSaveSelection()
{
	saveImage(imageView->getSelectionImage());
}
void MainWindow::slotEditCopy()
{
	QApplication::clipboard()->setPixmap(imageView->getPixmap());
}
extern QTranslator currentTranslator;

void MainWindow::slotEditClear()
{
	loadPixmap(QPixmap());
}
void MainWindow::slotEditCopySelection()
{
	if(imageView->hasSelection())
	{
		QApplication::clipboard()->setImage(imageView->getSelectionImage());
	}
}
void MainWindow::slotOptionsAutoCopy()
{
}
void MainWindow::slotOptionsAutoHide()
{
}
void MainWindow::slotOptionsAutoHideDelay()
{
	bool ok;
	int value = QInputDialog::getInt(this, tr("Auto hide delay"), tr("Delay:"), captureDesktopDelay, 0, 10000, 1, &ok);
	if(ok)
	{
		captureDesktopDelay = value;
	}
}
void MainWindow::slotOptionsCaptureClientArea()
{
}
void MainWindow::slotOptionsCaptureWindowArea()
{
}
void MainWindow::slotOptionsShowBorder()
{
	imageView->setBorderVisible(ui->actionOptionsShowBorder->isChecked());
}
void MainWindow::slotOptionsBorderColor()
{
	QColor color = QColorDialog::getColor(imageView->getBorderColor(), this);
	if(color.isValid())
	{
		imageView->setBorderColor(color);
		ui->actionOptionsBorderColor->setIcon(ColorToIcon(color));
	}
}
void MainWindow::slotOptionsBackgroundColor()
{
	QColor color = QColorDialog::getColor(imageView->getBackColor(), this);
	if(color.isValid())
	{
		imageView->setBackColor(color);
		ui->actionOptionsBackgroundColor->setIcon(ColorToIcon(color));
	}
}
void MainWindow::slotOptionsSelectionColor()
{
	QColor color = QColorDialog::getColor(windowSelector->getSelectionColor(), this);
	if(color.isValid())
	{
		windowSelector->setSelectionColor(color);
		ui->actionOptionsSelectionColor->setIcon(ColorToIcon(color));
	}
}
void MainWindow::slotHelpAbout()
{
	AboutDialog dlg;
	dlg.exec();
}
void MainWindow::slotShootScreen()
{
	qApp->beep();
#if 0
	loadPixmap(QPixmap::grabWindow(QApplication::desktop()->winId()));
#else
	QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());

	if(ui->actionOptionsCaptureCursor->isChecked())
	{
		int px;
		int py;
		QPixmap cursor = CaptureCursor(px, py);
		QPainter p(&pixmap);

		p.drawPixmap(px, py, cursor);
	}

	loadPixmap(pixmap);
#endif
	ui->actionCaptureDesktop->setEnabled(true);
}
void MainWindow::slotCaptureDesktop()
{
	ui->actionCaptureDesktop->setEnabled(false);

	if(ui->actionOptionsAutoHide->isChecked())
	{
		hide();
	}
	QTimer::singleShot(captureDesktopDelay, this, SLOT(slotShootScreen()));
}
void MainWindow::slotWindowStartSelection(WId p)
{
	Q_UNUSED(p);
#if 0
	if(ui->actionOptionsAutoMinimize->isChecked())
	{
		setWindowState(Qt::WindowMinimized);
		QThread::sleep(AUTO_MINIMIZE_DELAY);
	}
#endif
}
void MainWindow::slotWindowChangeSelection(WId p)
{
	Q_UNUSED(p);
}
void MainWindow::slotWindowCancelSelection(WId p)
{
	Q_UNUSED(p);
	if(ui->actionOptionsAutoMinimize->isChecked())
	{
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	}
}
void MainWindow::slotWindowFinishSelection(WId p, const QRect& rc)
{
	if(!p)
		return;

#if defined(Q_OS_WIN)
	HWND hwnd = (HWND)p;
#elif defined(Q_OS_LINUX)
	Window hwnd = (Window)p;
#endif
//	if(ui->actionOptionsSetFocus->isChecked())
	{
		SetOnTopWindow(hwnd);
	}
	QThread::msleep(200);

	QPixmap pixmap = ui->actionOptionsCaptureClientArea->isChecked() ? CaptureClientArea(hwnd) : CaptureWindowArea(hwnd, rc);
	if(!pixmap.isNull())
	{
		if(ui->actionOptionsCaptureCursor->isChecked())
		{
			int px;
			int py;
			QPixmap cursor = CaptureCursor(px, py);
			QPainter p(&pixmap);
#if defined(Q_OS_WIN)
			if(ui->actionOptionsCaptureClientArea->isChecked())
			{
				POINT pt;
				pt.x = px;
				pt.y = py;
				ScreenToClient(hwnd, &pt);
				px = pt.x;
				py = pt.y;
			}
			else
			{
#if 0
				RECT rc;
				HRESULT hr;
				OSVERSIONINFOW osvi;
				osvi.dwOSVersionInfoSize = sizeof(osvi);
				GetVersionExW(&osvi);
				hr = 0;
				if(osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2))
				{
					hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rc, sizeof(RECT));
				}
				if(!SUCCEEDED(hr))
				{
					GetWindowRect(hwnd, &rc);
				}
				px = px - rc.left;
				py = py - rc.top;
#else
				px = px - rc.x();
				py = py - rc.y();
#endif
			}
			p.drawPixmap(px, py, cursor);
#elif defined(Q_OS_LINUX)
			if(ui->actionOptionsCaptureClientArea->isChecked())
			{
			}
			else
			{
			}
#endif
		}
		loadPixmap(pixmap);
	}
	if(ui->actionOptionsAutoMinimize->isChecked())
	{
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	}
}
void MainWindow::slotImageViewSelectionChanged(bool valid, const QRect& rect)
{
	if(!valid)
		ui->statusBar->clearMessage();
	else
		ui->statusBar->showMessage(tr("Selection: (%1,%2,%3,%4)")
								   .arg(rect.x())
								   .arg(rect.y())
								   .arg(rect.width())
								   .arg(rect.height()));

	if(valid && !rect.isEmpty())
		ui->actionFileSaveSelection->setEnabled(true);
	else
		ui->actionFileSaveSelection->setEnabled(false);
}
void MainWindow::slotImageViewCustomContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);
	// ...
	QMenu menu;

	if(imageView->getTool() == ImageView::TOOL_SELECTION)
	{
		menu.addAction(ui->actionFileSave);
		if(imageView->hasSelection())
			menu.addAction(ui->actionFileSaveSelection);
		menu.addSeparator();

		menu.addAction(QIcon(":/tool-move.png"), tr("Move"), this, SLOT(slotSetToolMove()));

		if(!imageView->getPixmap().isNull())
		{
			menu.addSeparator();
			menu.addAction(ui->actionEditCopy);
			if(imageView->hasSelection())
				menu.addAction(ui->actionEditCopySelection);
		}
	}
	else
	{
		menu.addAction(ui->actionFileSave);
		menu.addSeparator();
		menu.addAction(QIcon(":/tool-selection.png"), tr("Selection"), this, SLOT(slotSetToolSelection()));
		if(!imageView->getPixmap().isNull())
		{
			menu.addSeparator();
			menu.addAction(ui->actionEditCopy);
		}
	}
	menu.exec(QCursor::pos());
}
void MainWindow::slotSetToolMove()
{
	imageView->setTool(ImageView::TOOL_HAND);
}
void MainWindow::slotSetToolSelection()
{
	imageView->setTool(ImageView::TOOL_SELECTION);
}
void MainWindow::changeEvent(QEvent* e)
{
	if(e != NULL)
	{
		switch(e->type())
		{
#if 1
			case QEvent::LocaleChange:
				{
					QString locale = QLocale::system().name();
					locale.truncate(locale.lastIndexOf('_'));
					languages.load(locale);
				}
				break;
#endif
			case QEvent::LanguageChange:
				ui->retranslateUi(this);
				break;
			default:
				break;
		}
	}
	QMainWindow::changeEvent(e);
}
