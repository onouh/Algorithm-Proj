QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = HanoiTower
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    hanoi.cpp \
    hanoiwidget.cpp

HEADERS += \
    hanoi.h \
    hanoiwidget.h \
    mainwindow.h

DESTDIR     = $$PWD/build
OBJECTS_DIR = $$PWD/build/.obj
MOC_DIR     = $$PWD/build/.moc
