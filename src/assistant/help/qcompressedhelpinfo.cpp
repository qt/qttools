// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
