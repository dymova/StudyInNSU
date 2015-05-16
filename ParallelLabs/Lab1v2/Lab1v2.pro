TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

INCLUDEPATH += /usr/include/mpich/
LIBS += -lmpich -lopa -lpthread -lrt
QMAKE_CXXFLAGS += -Bsymbolic-functions
QMAKE_CXX = mpic
QMAKE_CC= gcc -std=c99
