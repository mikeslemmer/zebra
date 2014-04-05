#-------------------------------------------------
#
# Project created by QtCreator 2014-04-02T23:53:25
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET   = zebra

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    netserver.cpp \
    zebraapp.cpp \
    sortedset.cpp

HEADERS += \
    netserver.h \
    globals.h \
    zebraapp.h \
    sortedset.h \
    respformatter.h
