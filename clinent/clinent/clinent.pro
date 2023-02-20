TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread

SOURCES += \
        getch.c \
        hall.c \
        main.c

HEADERS += \
    Protocol.h \
    getch.h \
    hall.h
