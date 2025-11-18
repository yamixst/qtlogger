QT = core concurrent

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    main.cpp

include(../../qtlogger_link.pri)

TARGET = file_rotation

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
