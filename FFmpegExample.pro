QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 设置 .exe 输出目录为 bin
DESTDIR = $$PWD/bin

FFMPEG_HOME = $$PWD/ffmpeg/ffmpeg-5.1.3-265
INCLUDEPATH += $${FFMPEG_HOME}/include
LIBS += -L$${FFMPEG_HOME}/lib \
           -lavcodec \
           -lavdevice \
           -lavfilter \
           -lavformat \
           -lavutil \
           -lswscale \
           -lswresample

DEFINES += QT_DEPRECATED_WARNINGS \
    SDL_MAIN_HANDLED

SDL_HOME = $$PWD/SDL2/SDL2-2.0.22
INCLUDEPATH += $${SDL_HOME}/include
LIBS += -L$${SDL_HOME}/lib/x64 \
            -lSDL2

msvc{
#解决msvc编译器中文问题
    QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
}
