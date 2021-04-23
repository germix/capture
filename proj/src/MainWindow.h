#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>

namespace Ui {
    class MainWindow;
}

class ImageView;
class WindowSelector;

#include "Languages.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow*	ui;

	ImageView*		imageView;
	WindowSelector*	windowSelector;

	int				captureDesktopDelay;

	Languages		languages;
public:
	explicit MainWindow(QWidget* parent = 0);
	~MainWindow();
private:
	void changeEvent(QEvent* e);
private:
	void saveImage(const QImage& image);
	void loadPixmap(const QPixmap& pixmap);
private slots:
	void slotFileExit();
	void slotFileSave();
	void slotFileSaveSelection();
	void slotEditCopy();
	void slotEditClear();
	void slotEditCopySelection();
	void slotOptionsAutoCopy();
	void slotOptionsAutoHide();
	void slotOptionsAutoHideDelay();
	void slotOptionsCaptureClientArea();
	void slotOptionsCaptureWindowArea();
	void slotOptionsShowBorder();
	void slotOptionsBorderColor();
	void slotOptionsBackgroundColor();
	void slotOptionsSelectionColor();
	void slotHelpAbout();

	void slotShootScreen();
	void slotCaptureDesktop();

    void slotWindowStartSelection(WId p);
    void slotWindowChangeSelection(WId p);
    void slotWindowCancelSelection(WId p);
    void slotWindowFinishSelection(WId p, const QRect& rc);

	void slotImageViewSelectionChanged(bool valid, const QRect& rect);
	void slotImageViewCustomContextMenuRequested(const QPoint& pos);

	void slotSetToolMove();
	void slotSetToolSelection();
};

#endif // MAINWINDOW_H
