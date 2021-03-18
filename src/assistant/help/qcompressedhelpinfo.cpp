/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "qcompressedhelpinfo.h"

#include "qhelpdbreader_p.h"

#include <QtCore/QThread>
#include <QtCore/QVersionNumber>

QT_BEGIN_NAMESPACE

class QCompressedHelpInfoPrivate : public QSharedData
{
public:
    QCompressedHelpInfoPrivate() = default;
    QCompressedHelpInfoPrivate(const QCompressedHelpInfoPrivate &other)
        : QSharedData(other)
        , m_namespaceName(other.m_namespaceName)
        , m_component(other.m_component)
        , m_version(other.m_version)
        , m_isNull(other.m_isNull)
    { }
    ~QCompressedHelpInfoPrivate() = default;

    QString m_namespaceName;
    QString m_component;
    QVersionNumber m_version;
    bool m_isNull = true;
};

/*!
    \class QCompressedHelpInfo
    \since 5.13
    \inmodule QtHelp
    \brief The QCompressedHelpInfo class provides access to
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
QCompressedHelpInfo::QCompressedHelpInfo()
    : d(new QCompressedHelpInfoPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QCompressedHelpInfo::QCompressedHelpInfo(const QCompressedHelpInfo &) = default;

/*!
    Move-constructs a QCompressedHelpInfo instance,
    making it point to the same object that \a other was pointing to,
    so that it contains the information the \a other used to contain.
*/
QCompressedHelpInfo::QCompressedHelpInfo(QCompressedHelpInfo &&) = default;

/*!
    Destroys the QCompressedHelpInfo.
*/
QCompressedHelpInfo::~QCompressedHelpInfo() = default;

/*!
    Makes this QHelpCollectionDetails into a copy of \a other, so the two
    are identical, and returns a reference to this QHelpCollectionDetails.
*/
QCompressedHelpInfo &QCompressedHelpInfo::operator=(const QCompressedHelpInfo &) = default;

/*!
    Move-assigns \a other to this QCompressedHelpInfo instance.
*/
QCompressedHelpInfo &QCompressedHelpInfo::operator=(QCompressedHelpInfo &&) = default;

/*!
    \fn void QCompressedHelpInfo::swap(QCompressedHelpInfo &other)

    Swaps the compressed help file \a other with this compressed help file. This
    operation is very fast and never fails.
*/

/*!
    Returns the namespace name of the compressed help file.
*/
QString QCompressedHelpInfo::namespaceName() const
{
    return d->m_namespaceName;
}

/*!
    Returns the component of the compressed help file.
*/
QString QCompressedHelpInfo::component() const
{
    return d->m_component;
}

/*!
    Returns the version of the compressed help file.
*/
QVersionNumber QCompressedHelpInfo::version() const
{
    return d->m_version;
}

/*!
    Returns \c true if the info is invalid, otherwise returns
    \c false.
*/
bool QCompressedHelpInfo::isNull() const
{
    return d->m_isNull;
}

/*!
    Returns the QCompressedHelpInfo instance for the
    \a documentationFileName of the existing qch file.
*/
QCompressedHelpInfo QCompressedHelpInfo::fromCompressedHelpFile(const QString &documentationFileName)
{
    QHelpDBReader reader(documentationFileName,
        QHelpGlobal::uniquifyConnectionName(QLatin1String("GetCompressedHelpInfo"),
        QThread::currentThread()), nullptr);
    if (reader.init()) {
        QCompressedHelpInfo info;
        info.d->m_namespaceName = reader.namespaceName();
        info.d->m_component = reader.virtualFolder();
        info.d->m_version = QVersionNumber::fromString(reader.version());
        info.d->m_isNull = false;
        return info;
    }
    return QCompressedHelpInfo();
}

QT_END_NAMESPACE
