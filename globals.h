#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtCore>
#include <QtNetwork>

static const quint16 SERVER_PORT = 6379;
static const char *CMD_QUIT = "QUIT";

#define RESP_EOL            "\r\n"
#define RESP_INT            ':'
#define RESP_ARRAY          '*'
#define RESP_STRING         '$'
#define RESP_ERROR          "-ERR "
#define RESP_QUOTE          "\""
static const char *RESP_NIL =                   "nil" RESP_EOL;

#ifdef DEBUG
#define LOGDEBUG(msg) qDebug() << __PRETTY_FUNCTION__ << msg;
#else
#define LOGDEBUG(msg)
#endif

#endif // GLOBALS_H
