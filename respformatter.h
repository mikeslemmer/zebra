#ifndef RESPFORMATTER_H
#define RESPFORMATTER_H

#include "globals.h"

class RespFormatter
{
private:
    RespFormatter() {}

public:
    inline static QByteArray formatText(const QByteArray &baText)
    {
        if (baText.isNull())
        {
            return RESP_STRING + QByteArray("-1") + RESP_EOL;
        }

        return RESP_STRING + QByteArray::number(baText.length()) + RESP_EOL + baText + RESP_EOL;
    }

    inline static QByteArray formatArray(const QList<QByteArray> &listInput)
    {
        QByteArray baFormatted = RESP_ARRAY + QByteArray::number(listInput.count()) + RESP_EOL;

        foreach (QByteArray baInput, listInput)
        {
            baFormatted += RespFormatter::formatText(baInput);
        }

        return baFormatted;
    }

    inline static QByteArray formatError(const QByteArray &baError)
    {
        return RESP_ERROR + baError + RESP_EOL;
    }

    inline static QByteArray formatInt(int nInput)
    {
        return RESP_INT + QByteArray::number(nInput) + RESP_EOL;
    }
};

#endif // RESPFORMATTER_H
