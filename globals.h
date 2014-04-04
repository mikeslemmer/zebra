#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtCore>
#include <QtNetwork>

static const quint16 SERVER_PORT = 9090;
static const char *CMD_QUIT = "QUIT";

#define RESP_EOL            "\n"
#define RESP_INTEGER        "(integer) "
#define RESP_EMPTY          "(empty list or set)"
#define RESP_ARRAY_PAREN    ") "
#define RESP_ERROR          "(error) ERR "
#define RESP_QUOTE          "\""
static const char *RESP_NIL =                   "nil" RESP_EOL;

#define LOGDEBUG(msg) qDebug() << __PRETTY_FUNCTION__ << msg;


#endif // GLOBALS_H
