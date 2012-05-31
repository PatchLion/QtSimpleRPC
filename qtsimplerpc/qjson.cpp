/****************************************************************************
**
** Copyright (C) 2012 Sebastian Lehmann
** Contact: contact@l3.ms
**
**
** This file is part of QtSimpleRPC.
**
** QtSimpleRPC is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** QtSimpleRPC is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "qjson.h"
#include <QStringList>
#include <QDebug>


QList<int> QJson::metaTypes_int;

QJson::QJson()
{
}

QString QJson::Error::text() const
{
    switch(t)
    {
    case UnexpectedEnd:
        return QString("Unexpected end of JSON input.");
    case UnexpectedCharacter:
        return QString("Unexpected character at position %1.").arg(pos);
    case ExpectedColon:
        return QString("Unexpected character, expected colon at position %1.").arg(pos);
    case IllegalNumber:
        return QString("Can't parse number at position %1.").arg(pos);
    case UnknownKeyword:
        return QString("Can't parse keyword at position %1. Only keywords `true', `false' and `null' are supported (case-sensitive).").arg(pos);
    case UnknownType:
        return QString("Found unknown type when trying to encode JSON.");
    default:
        return QString();
    }
}


/* ----------------------------------------------------------------------------------------------------------------- */
// ENCODER / SERIALIZER
/* ----------------------------------------------------------------------------------------------------------------- */



QString QJson::encode(const QVariant &data, Error *error, int indentation)
{
    bool deleteError = false;
    if(!error) {
        error = new Error;
        deleteError = true;
    }
    return serializeValue(data, EncodeOptions(), error, indentation, QString(""));
    if(deleteError)
        delete error;
}

QString QJson::encode(const QVariant &data, EncodeOptions options, Error *error, int indentation)
{
    bool deleteError = false;
    if(!error) {
        error = new Error;
        deleteError = true;
    }
    return serializeValue(data, options, error, indentation, QString(""));
    if(deleteError)
        delete error;
}


QString QJson::serializeValue(const QVariant &data, EncodeOptions options, Error *error,
                          int indentation, QString currentLinePrefix)
{
    QString indentedLinePrefix = options.testFlag(Compact) ?
                QString::fromAscii("") :
                (currentLinePrefix + QString::fromAscii(" ").repeated(indentation));

    QString optionalNewLine = options.testFlag(Compact) ?
                QString::fromAscii("") :
                (QString::fromAscii("\n") + currentLinePrefix);

    QString optionalIndentedNewLine = options.testFlag(Compact) ?
                QString::fromAscii("") :
                (QString::fromAscii("\n") + indentedLinePrefix);

    QString encoded;

    switch(data.type())
    {
    case QVariant::Bool:
        encoded += QString::fromAscii(data.toBool() ? "true" : "false");
        break;

    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        Q_ASSERT(data.canConvert(QVariant::String));
        encoded = data.toString();
        break;

    case QVariant::Double:
        encoded = QString::number(data.toDouble(), 'g', 16);
        if(!encoded.contains(QString::fromAscii(".")) && !encoded.contains(QString::fromAscii("e")))
            encoded += ".0";
        break;

    case QVariant::String:
        encoded = serializeString(data.toString());
        break;

    case QVariant::ByteArray:
        encoded = serializeString(QString::fromLocal8Bit(data.toByteArray()));
        break;

    case QVariant::List:
    case QVariant::StringList:
        {
            encoded = QString::fromAscii("[") + optionalIndentedNewLine;
            QVariantList list = data.toList();
            for(int i = 0; i < list.count(); ++i)
            {
                if(i) encoded += QString::fromAscii(",") + optionalIndentedNewLine;
                encoded += serializeValue(list.at(i), options, error, indentation, indentedLinePrefix);
                if(!error->isNull())
                    return QString();
            }
            encoded += optionalNewLine + QString::fromAscii("]");
        }
        break;

    case QVariant::Map:
        encoded = serializeObject<QVariantMap>(data.toMap(), options, error, indentation, optionalNewLine, optionalIndentedNewLine, indentedLinePrefix);
        break;

    case QVariant::Hash:
        encoded = serializeObject<QVariantHash>(data.toHash(), options, error, indentation, optionalNewLine, optionalIndentedNewLine, indentedLinePrefix);
        break;

    case QVariant::Invalid:
        encoded = QString::fromAscii("null");
        break;

    default:
        if(data.type() == QVariant::UserType && metaTypes_int.contains(data.userType()))
        {
            // reinterpret the internal contents of the variant
            int value = *reinterpret_cast<const int*>(data.constData());
            encoded = QVariant(value).toString();
        }
        else if(!options.testFlag(EncodeUnknownTypesAsNull))
        {
            if(error)
                *error = Error(Error::UnknownType);
            return QString();
        }
        else
        {
            encoded = QString::fromAscii("null");
        }
        break;
    }

    return encoded;
}

QString QJson::serializeString(QString data)
{
    QString encoded;
    encoded.append(QChar::fromAscii('"'));
    for(int i = 0; i < data.length(); ++i)
    {
        QChar ch = data.at(i);

        // printable ASCII character?
        if(ch.unicode() >= 32 && ch.unicode() < 128)
            encoded.append(ch);
        else
        {
            switch(ch.unicode())
            {
            case 8:
                encoded.append(QString::fromAscii("\\b"));
                break;
            case 9:
                encoded.append(QString::fromAscii("\\t"));
                break;
            case 10:
                encoded.append(QString::fromAscii("\\n"));
                break;
            case 12:
                encoded.append(QString::fromAscii("\\f"));
                break;
            case 13:
                encoded.append(QString::fromAscii("\\r"));
                break;
            case '"':
                encoded.append(QString::fromAscii("\\\""));
                break;
            case '\\':
                encoded.append(QString::fromAscii("\\\\"));
                break;
            case '/':
                encoded.append(QString::fromAscii("\\/"));
                break;
            default:
                encoded.append(QString::fromAscii("\\u") + QString::number(ch.unicode(), 16)
                               .rightJustified(4, QChar::fromAscii('0')));
            }
        }
    }
    encoded.append(QChar::fromAscii('"'));
    return encoded;
}

void QJson::treatMetaTypeAsInteger(int metaType)
{
    metaTypes_int << metaType;
}

template<typename QVariantHashOrMap>
QString QJson::serializeObject(QVariantHashOrMap container, EncodeOptions options, Error *error, int indentation, const QString &optionalNewLine, const QString &optionalIndentedNewLine, const QString &indentedLinePrefix)
{
    QString encoded = QString::fromAscii("{") + optionalIndentedNewLine;
    typename QVariantHashOrMap::iterator i;
    bool first = true;
    for (i = container.begin(); i != container.end(); ++i)
    {
        if(!first)
            encoded += QString::fromAscii(",") + optionalIndentedNewLine;
        first = false;
        encoded += serializeString(i.key());
        encoded += options.testFlag(Compact) ? QString::fromAscii(":") : QString::fromAscii(" : ");
        encoded += serializeValue(i.value(), options, error, indentation, indentedLinePrefix);
        if(!error->isNull())
            return QString();
    }
    encoded += optionalNewLine + QString::fromAscii("}");
    return encoded;
}


/* ----------------------------------------------------------------------------------------------------------------- */
// DECODER / PARSER
/* ----------------------------------------------------------------------------------------------------------------- */



QVariant QJson::decode(const QString &json, Error *error)
{
    return decode(json, DecodeOptions(), error);
}

QVariant QJson::decode(const QString &json, DecodeOptions options, Error *error)
{
    //there are currently no options defined
    Q_UNUSED(options);

    bool success = true;

    if(!json.isNull() && !json.isEmpty())
    {
        int index = 0;
        return parseValue(json, index, options, success, error);
    }
    else
    {
        // To simplify things, this is not treated as an error but as valid input.
        return QVariant();
    }
}


QVariant QJson::parseValue(const QString &json, int &index, DecodeOptions options, bool &success, Error *error)
{
    skipWhitespace(json, index);
    if(!checkAvailable(json, index, success, error))
        return QVariant();

    switch(json[index].toAscii())
    {
    case '"':
        return QJson::parseString(json, index, success, error);

    case '{':
        if(options & DecodeObjectsAsHash)
            return QJson::parseObject<QVariantHash>(json, index, options, success, error);
        else
            return QJson::parseObject<QVariantMap>(json, index, options, success, error);

    case '[':
        return QJson::parseArray(json, index, options, success, error);

    case 't':
    case 'f':
        if(options & AllowUnquotedStrings)
            return QJson::parseUnquotedString(json, index, success, error);
        else
            return QJson::parseBool(json, index, success, error);

    case 'n':
        if(options & AllowUnquotedStrings)
            return QJson::parseUnquotedString(json, index, success, error);
        else
            return QJson::parseNull(json, index, success, error);

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
        return QJson::parseNumber(json, index, success, error);

    default:
        if(options & AllowUnquotedStrings)
            return QJson::parseUnquotedString(json, index, success, error);
        else
        {
            success = false;
            if(error)
                *error = Error(Error::UnexpectedCharacter, index);
            return QVariant();
        }
    }
}

template<typename ContainerType>
QVariant QJson::parseObject(const QString &json, int &index, DecodeOptions options, bool &success, Error *error)
{
    Q_ASSERT(json[index] == '{');
    index++;
    skipWhitespace(json, index);

    ContainerType object;

    while(checkAvailable(json, index, success, error))
    {
        if(json[index] == QChar::fromAscii('}'))
        {
            index++;
            return object;
        }
        else
        {
            QString key = QJson::parseValue(json, index, options, success, error).toString();
            if(!success)
                return QVariant();

            skipWhitespace(json, index);
            checkAvailable(json, index, success, error);

            if(json[index] == QChar::fromAscii(':'))
                index++;
            else
            {
                success = false;
                if(error)
                    *error = Error(Error::ExpectedColon, index);
                return QVariant();
            }

            skipWhitespace(json, index);

            QVariant value = QJson::parseValue(json, index, options, success, error);
            if(!success)
                return QVariant();

            // Add the key / value pair to the object
            object.insert(key, value);

            int skippedWhitespaces = skipWhitespace(json, index);
            checkAvailable(json, index, success, error);

            switch(json[index].toAscii())
            {
            case ',':
                index++;
                skipWhitespace(json, index);
                break;

            case '}':
                //'}' will be processed in the next iteration
                break;

            default:
                // Only allow missing comma if there is at least one whitespace instead of the comma!
                if((options & AllowMissingComma) && skippedWhitespaces > 0)
                    break;
                else
                {
                    success = false;
                    if(error)
                        *error = Error(Error::UnexpectedCharacter, index);
                    return QVariant();
                }
            }
        }
    }

    return QVariant();
}

QVariant QJson::parseArray(const QString &json, int &index, DecodeOptions options, bool &success, Error *error)
{
    Q_ASSERT(json[index] == '[');
    index++;
    skipWhitespace(json, index);

    QVariantList array;

    while(checkAvailable(json, index, success, error))
    {
        if(json[index] == QChar::fromAscii(']'))
        {
            index++;
            return array;
        }
        else
        {
            QVariant value = QJson::parseValue(json, index, options, success, error);
            if(!success)
                return QVariant();

            // Add the value pair to the array
            array.append(value);

            int skippedWhitespaces = skipWhitespace(json, index);
            checkAvailable(json, index, success, error);

            switch(json[index].toAscii())
            {
            case ',':
                index++;
                skipWhitespace(json, index);
                break;

            case ']':
                //']' will be processed in the next iteration
                break;

            default:
                // Only allow missing comma if there is at least one whitespace instead of the comma!
                if((options & AllowMissingComma) && skippedWhitespaces > 0)
                    break;
                else
                {
                    success = false;
                    if(error)
                        *error = Error(Error::UnexpectedCharacter, index);
                    return QVariant();
                }
            }
        }
    }

    return QVariant();
}


QVariant QJson::parseString(const QString &json, int &index, bool &success, Error *error)
{
    Q_ASSERT(json[index] == '"');
    index++;

    QString string;
    QChar ch;

    while(checkAvailable(json, index, success, error))
    {
        ch = json[index++];

        switch(ch.toAscii())
        {
        case '\\':
            // Escaped character
            if(!checkAvailable(json, index, success, error))
                return QVariant();
            ch = json[index++];
            switch(ch.toAscii())
            {
            case 'b':
                string.append('\b');
                break;
            case 'f':
                string.append('\f');
                break;
            case 'n':
                string.append('\n');
                break;
            case 'r':
                string.append('\r');
                break;
            case 't':
                string.append('\t');
                break;
            case 'u':
                if(!checkAvailable(json, index, success, error, 4))
                    return QVariant();
                string.append(QChar(json.mid(index, 4).toInt(0, 16)));
                index += 4;
                break;
            default:
                string.append(ch);
            }
            break;

        case '"':
            // End of string
            return QVariant(string);

        default:
            string.append(ch);
        }
    }

    return QVariant();
}

QVariant QJson::parseUnquotedString(const QString &json, int &index, bool &success, Error *error)
{
    QString string;
    QChar ch;

    bool end = false;
    while(!end && checkAvailable(json, index, success, error))
    {
        ch = json[index++];

        switch(ch.toAscii())
        {
        case '\\':
            // Escaped character
            if(!checkAvailable(json, index, success, error))
                return QVariant();
            ch = json[index++];
            switch(ch.toAscii())
            {
            case 'b':
                string.append('\b');
                break;
            case 'f':
                string.append('\f');
                break;
            case 'n':
                string.append('\n');
                break;
            case 'r':
                string.append('\r');
                break;
            case 't':
                string.append('\t');
                break;
            case 'u':
                if(!checkAvailable(json, index, success, error, 4))
                    return QVariant();
                string.append(QChar(json.mid(index, 4).toInt(0, 16)));
                index += 4;
                break;
            default:
                string.append(ch);
            }
            break;

        case ':':
        case ',':
        case ']':
        case '}':
        case '\n':
            // End of string (was one character before this!)
            end = true;
            index--;
            break;

        default:
            string.append(ch);
        }
    }

    //trim string
    string = string.trimmed();

    //handle keywords
    if(string == "true")
        return QVariant(true);
    if(string == "false")
        return QVariant(false);
    if(string == "null")
        return QVariant();

    return QVariant(string);
}

QVariant QJson::parseNumber(const QString &json, int &index, bool &success, Error *error)
{
    int end = index;
    bool endFound = false;
    while(!endFound && end < json.length())
    {
        switch(json[end].toAscii())
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        case 'e':
        case 'E':
        case '+':
        case '-':
            end++;
            break;

        default:
            endFound = true;
            break;
        }
    }

    QString numberStr = json.mid(index, end - index);
    index = end;

    QVariant result;
    bool ok;

    // is floating point number?
    if(numberStr.contains(QChar::fromAscii('.')) ||
       numberStr.contains(QChar::fromAscii('e')) ||
       numberStr.contains(QChar::fromAscii('E')))
        result = QVariant(numberStr.toDouble(&ok));
    else
        result = QVariant(numberStr.toLongLong(&ok));

    if(ok)
        return result;
    else {
        success = false;
        if(error)
            *error = Error(Error::IllegalNumber, index);
        return QVariant();
    }
}

int QJson::skipWhitespace(const QString &json, int &index)
{
    int skipped = 0;
    while(index < json.size())
    {
        switch(json[index].toAscii())
        {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            index++;
            skipped++;
            break;
        default:
            return skipped;
        }
    }
    return skipped;
}

QVariant QJson::parseBool(const QString &json, int &index, bool &success, Error *error)
{
    if(checkToken(json, index, success, error, "true"))
    {
        index += 4;
        return QVariant(true);
    }
    else if(checkToken(json, index, success, error, "false"))
    {
        index += 5;
        return QVariant(false);
    }
    else
    {
        success = false;
        if(error)
            *error = Error(Error::UnknownKeyword, index);
        return QVariant();
    }
}

QVariant QJson::parseNull(const QString &json, int &index, bool &success, Error *error)
{
    if(checkToken(json, index, success, error, "null"))
    {
        index += 4;
        return QVariant(true);
    }
    else
    {
        success = false;
        if(error)
            *error = Error(Error::UnknownKeyword, index);
        return QVariant();
    }
}

bool QJson::checkAvailable(const QString &json, int &index, bool &success, Error *error, int minAvailable)
{
    if(index + minAvailable > json.length())
    {
        success = false;
        if(error)
            *error = Error(Error::UnexpectedEnd);
        return false;
    }
    else
        return true;
}

bool QJson::checkToken(const QString &json, int &index, bool &success, Error *error, QString token)
{
    return checkAvailable(json, index, success, error, token.length())
            && json.mid(index, token.length()) == token;
}
