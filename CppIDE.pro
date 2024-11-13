QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += include/

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/completionwidget.cpp \
    src/highlighter.cpp

# Header files
HEADERS += \
    include/mainwindow.h \
    include/completionwidget.h \
    include/highlighter.h

# Forms
FORMS += \
    resources/mainwindow.ui

# Resources
RESOURCES += \
    resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Output directories
DESTDIR = build/
OBJECTS_DIR = build/obj/
MOC_DIR = build/moc/
RCC_DIR = build/rcc/
UI_DIR = build/ui/
