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

#ifndef QJSON_H
#define QJSON_H

#include <QString>
#include <QVariant>

class QJson
{
    Q_FLAGS(DecodeOption EncodeOptions)
    Q_FLAGS(DecodeOption DecodeOptions)

public:
    enum EncodeOption
    {
        EncodeUnknownTypesAsNull = 0x01,
        Compact = 0x02
    };
    Q_DECLARE_FLAGS(EncodeOptions, EncodeOption)

    enum DecodeOption
    {
        DecodeObjectsAsHash = 0x01,
        AllowUnquotedStrings = 0x02,
        AllowMissingComma = 0x04,
        AllowLazyJSON = AllowUnquotedStrings | AllowMissingComma
    };
    Q_DECLARE_FLAGS(DecodeOptions, DecodeOption)

    class Error
    {
    public:
        enum Type {
            NoError = 0,
            UnexpectedEnd,
            UnexpectedCharacter,
            ExpectedColon,
            IllegalNumber,
            UnknownKeyword,
            UnknownType
        };

        Error() : t(NoError), pos(0) {}
        Error(Type t, int pos = 0) : t(t), pos(pos) {}

        bool isNull() const { return t == NoError; }
        bool isError() const { return !isNull(); }
        Type type() const { return t; }
        int position() const { return pos; }
        QString text() const;

    private:
        Type t;
        int pos;
    };


    static QString encode(const QVariant &data, Error *error = 0, int indentation = 4);
    static QString encode(const QVariant &data, EncodeOptions options, Error *error = 0, int indentation = 4);

    static QVariant decode(const QString &json, Error *error = 0);
    static QVariant decode(const QString &json, DecodeOptions options, Error *error = 0);

    //! Use this method to treat the given type, for example an enumerator, as it would be an integer. This may lead to program crash if the type can't be treated as an integer.
    static void treatMetaTypeAsInteger(int metaType);

private:
    QJson();

    static QString serializeValue(const QVariant &data, EncodeOptions options, Error *error, int indentation, QString currentLinePrefix);
    static QString serializeString(QString data);
    template<typename QVariantHashOrMap>
    static QString serializeObject(QVariantHashOrMap container, EncodeOptions options, Error *error, int indentation, const QString &optionalNewLine, const QString &optionalIndentedNewLine, const QString &indentedLinePrefix);

    static QVariant parseValue(const QString &json, int &index, DecodeOptions options, bool &success, Error *error);
    template<typename ContainerType>
    static QVariant parseObject(const QString &json, int &index, DecodeOptions options, bool &success, Error *error);
    static QVariant parseArray(const QString &json, int &index, DecodeOptions options, bool &success, Error *error);
    static QVariant parseString(const QString &json, int &index, bool &success, Error *error);
    static QVariant parseUnquotedString(const QString &json, int &index, bool &success, Error *error);
    static QVariant parseNumber(const QString &json, int &index, bool &success, Error *error);
    static QVariant parseBool(const QString &json, int &index, bool &success, Error *error);
    static QVariant parseNull(const QString &json, int &index, bool &success, Error *error);
    static int skipWhitespace(const QString &json, int &index);
    static bool checkAvailable(const QString &json, int &index, bool &success, Error *error, int minAvailable = 1);
    static bool checkToken(const QString &json, int &index, bool &success, Error *error, QString token);

    static QList<int> metaTypes_int;
};

#endif // QJSON_H
