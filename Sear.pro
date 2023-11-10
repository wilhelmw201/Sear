QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/anydisk.cpp \
    src/copyutil.cpp \
    src/crc32.cpp \
    src/gptdisk.cpp \
    src/gptpartition.cpp \
    src/ioworker.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/utils.cpp

HEADERS += \
    include/anydisk.h \
    include/copyutil.h \
    include/crc32.h \
    include/gptdisk.h \
    include/gptpartition.h \
    include/ioworker.h \
    include/mainwindow.h \
    include/utils.h

FORMS += \
    src/mainwindow.ui

TRANSLATIONS += \
    translate/Sear_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations
INCLUDEPATH += include
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
