/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhelpcollectiondetails.h"

#include "qhelpdbreader_p.h"

#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

class QHelpCollectionDetailsPrivate : public QSharedData
{
public:
    QHelpCollectionDetailsPrivate() = default;
    QHelpCollectionDetailsPrivate(const QHelpCollectionDetailsPrivate &other)
        : QSharedData(other)
        , m_namespaceName(other.m_namespaceName)
        , m_component(other.m_component)
        , m_version(other.m_version)
    { }
    ~QHelpCollectionDetailsPrivate() = default;

    QString m_namespaceName;
    QString m_component;
    QString m_version;
};

/*!
    \class QHelpCollectionDetails
    \since 5.13
    \inmodule QtHelp
    \brief The QHelpCollectionDetails class provides access to
    the details about a compressed help file.

    The detailed information about the compressed
    help file can be fetched by calling the fromCompressedHelpFile()
    static method, providing the path to the compressed
    help file.

    The class provides access to various information about a compressed help file.
    The namespace associated with the given compressed help file is
    namespaceName(), the associated component name is component()
    and version() provides version information.

    \sa QHelpFilterEngine
*/

/*!
    Constructs empty information about a compressed help file.
*/
QHelpCollectionDetails::QHelpCollectionDetails()
    : d(new QHelpCollectionDetailsPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QHelpCollectionDetails::QHelpCollectionDetails(const QHelpCollectionDetails &) = default;

/*!
    Move-constructs a QHelpCollectionDetails instance,
    making it point to the same object that \a other was pointing to,
    so that it contains the information the \a other used to contain.
*/
QHelpCollectionDetails::QHelpCollectionDetails(QHelpCollectionDetails &&) = default;

/*!
    Destroys the QHelpCollectionDetails.
*/
QHelpCollectionDetails::~QHelpCollectionDetails() = default;

/*!
    Makes this QHelpCollectionDetails into a copy of \a other, so the two
    are identical, and returns a reference to this QHelpCollectionDetails.
*/
QHelpCollectionDetails &QHelpCollectionDetails::operator=(const QHelpCollectionDetails &) = default;

/*!
    Move-assigns \a other to this QHelpCollectionDetails instance.
*/
QHelpCollectionDetails &QHelpCollectionDetails::operator=(QHelpCollectionDetails &&) = default;

/*!
    Returns the namespace name of the compressed help file.
*/
QString QHelpCollectionDetails::namespaceName() const
{
    return d->m_namespaceName;
}

/*!
    Returns the component of the compressed help file.
*/
QString QHelpCollectionDetails::component() const
{
    return d->m_component;
}

/*!
    Returns the version of the compressed help file.
*/
QString QHelpCollectionDetails::version() const
{
    return d->m_version;
}

/*!
    Returns the QHelpCollectionDetails instance for the
    \a documentationFileName of the existing qch file.
*/
QHelpCollectionDetails QHelpCollectionDetails::fromCompressedHelpFile(const QString &documentationFileName)
{
    QHelpDBReader reader(documentationFileName,
        QHelpGlobal::uniquifyConnectionName(QLatin1String("GetHelpDetails"),
        QThread::currentThread()), nullptr);
    if (reader.init()) {
        QHelpCollectionDetails details;
        details.d->m_namespaceName = reader.namespaceName();
        details.d->m_component = reader.virtualFolder();
        details.d->m_version = reader.version();
        return details;
    }
    return QHelpCollectionDetails();
}

QT_END_NAMESPACE
