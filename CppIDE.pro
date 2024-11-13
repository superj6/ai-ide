QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CppIDE
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    completionwidget.cpp

HEADERS += \
    mainwindow.h \
    completionwidget.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
