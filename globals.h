#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtCore>
#include <QtNetwork>

static const quint16 SERVER_PORT = 9090;
static const char *CMD_QUIT = "QUIT";

#define LOGDEBUG(msg) qDebug() << __PRETTY_FUNCTION__ << msg;


#endif // GLOBALS_H
