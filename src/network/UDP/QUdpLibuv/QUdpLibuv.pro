QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DESTDIR = ../../../../bin
LIBS += -L$$_PRO_FILE_PWD_/../../../../lib
LIBS += -luv

INCLUDEPATH +=$$_PRO_FILE_PWD_/../../../../include
SOURCES += \
        EventDispatcherLibUv.cpp \
        EventDispatcherLibUvPrivate.cpp \
        UvUdpSocket.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
	EventDispatcherLibUv.h \
	EventDispatcherLibUvPrivate.h \
	UvUdpSocket.h
