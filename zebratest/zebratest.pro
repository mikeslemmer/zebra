#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T21:33:30
#
#-------------------------------------------------

QT       += testlib
QT       += network

QT       -= gui

TARGET = tst_sortedsettest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += ../sortedset.h
SOURCES += tst_sortedsettest.cpp
           ../sortedset.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
