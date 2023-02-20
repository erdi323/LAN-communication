TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread

SOURCES += \
        main.c \
        user.c

HEADERS += \
    Protocol.h \
    user.h
