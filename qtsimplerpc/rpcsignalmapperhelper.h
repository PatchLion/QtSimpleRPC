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

/*
  NOTE:
  This file should not appear in .pro as the moc output is
  already contained in the .cpp file (a modified version).
*/

#ifndef GENERICSIGNALMAPPER_H
#define GENERICSIGNALMAPPER_H

#include "rpcsignalmapperhelper.h"

#include <QObject>
#include <QMetaMethod>
#include <QVariantList>

class RpcSignalMapperHelper : public QObject
{
    Q_OBJECT

public:
    explicit RpcSignalMapperHelper(QMetaMethod mappedMethod, QObject *parent = 0);

signals:
    //! Mapped signal which gets emitted whenever the slot map() has been called.
    //! Note that it only gets emitted when map() has been called using Qt's
    //! signal slot mechanism (by connecting your signal to map()).
    void mapped(QObject *sender, QMetaMethod signal, QVariantList arguments);

public slots:
    //! Slot to be mapped. Note that although this slot doesn't have any
    //! arguments, it does some magic behind the scenes to get them as
    //! passed by the emitted signal. They are converted to QVariants
    //! and then passed to the mapped() signal.
    //! Note that a call to this method does nothing but print a warning.
    //! You need to use Qt's signal slot mechanism for this to work.
    void map();

private:
    //! Internal signal handler. This gets called whenever a signal
    //! has been emitted on the connected object, which would normally
    //! call Qt's internal qt_static_metacall, which then calls the
    //! real slot. We replaced this step to have our own handler.
    void internalSignalHandler(void **arguments);

    //! The signal which is connected to map(). This meta method is used
    //! to get the type info of the parameters of the meta call.
    QMetaMethod method;
};

#endif // GENERICSIGNALMAPPER_H
