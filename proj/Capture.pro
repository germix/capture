#-------------------------------------------------
#
# Project created by QtCreator 2017-03-14T02:07:44
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

linux {
	LIBS += -lX11
	QT += x11extras
}
win32 {
	LIBS += -lgdi32 -ldwmapi
	QT += winextras
}

DEFINES += USE_MINIMIZE

#---------------------------------------------------------------------------------------------------
# Version
#---------------------------------------------------------------------------------------------------

DEFINES += MAJOR_VERSION=1
DEFINES += MINOR_VERSION=0

#---------------------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------------------

TARGET = Capture

CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
	DEFINES += DEBUG
}

#---------------------------------------------------------------------------------------------------
# Destination directory
#---------------------------------------------------------------------------------------------------

DESTDIR = ../bin

#---------------------------------------------------------------------------------------------------
# Source files
#---------------------------------------------------------------------------------------------------

SOURCES += \
	src/main.cpp\
	src/MainWindow.cpp \
	src/bsb_io.c \
	src/WindowSelector.cpp \
	src/ImageView.cpp \
	src/AboutDialog.cpp \
	src/utils.cpp \
	src/SelectionTrafo.cpp \
	src/Languages.cpp \
	src/WindowSelectorRectWidget.cpp \
	src/utils/suffix.cpp \
	src/utils/CaptureClientArea.cpp \
	src/utils/CaptureWindowArea.cpp \
	src/utils/SetOnTopWindow.cpp \
	src/utils/CaptureCursor.cpp

HEADERS  += \
	src/MainWindow.h \
	src/bsb.h \
	src/WindowSelector.h \
	src/ImageView.h \
	src/AboutDialog.h \
	src/utils.h \
	src/SelectionTrafo.h \
	src/Languages.h \
	src/WindowSelectorRectWidget.h

FORMS    += \
	src/MainWindow.ui \
	src/AboutDialog.ui

#---------------------------------------------------------------------------------------------------
# Resource files
#---------------------------------------------------------------------------------------------------

RESOURCES += res/resource.qrc

win32:RC_FILE = res/resource_win32.rc

#---------------------------------------------------------------------------------------------------
# Translation files
#---------------------------------------------------------------------------------------------------

TRANSLATIONS = translate/capture_es.ts translate/capture_en.ts
