// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDateTime>
#include <QtCore/QFileSystemWatcher>
#include <QtHelp/QHelpEngineCore>
#include "helpenginewrapper.h"
#include "qtdocinstaller.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QtDocInstaller::QtDocInstaller(const QList<DocInfo> &docInfos)
    : m_abort(false), m_docInfos(docInfos)
{
    TRACE_OBJ
}

QtDocInstaller::~QtDocInstaller()
{
    TRACE_OBJ
    if (!isRunning())
        return;
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();
    wait();
}

void QtDocInstaller::installDocs()
{
    TRACE_OBJ
    start(LowPriority);
}

void QtDocInstaller::run()
{
    TRACE_OBJ
    m_qchDir.setPath(QLibraryInfo::path(QLibraryInfo::DocumentationPath));
    m_qchFiles = m_qchDir.entryList(QStringList() << "*.qch"_L1);

    bool changes = false;
    for (const DocInfo &docInfo : std::as_const(m_docInfos)) {
        changes |= installDoc(docInfo);
        m_mutex.lock();
        if (m_abort) {
            m_mutex.unlock();
            return;
        }
        m_mutex.unlock();
    }
    emit docsInstalled(changes);
}

bool QtDocInstaller::installDoc(const DocInfo &docInfo)
{
    TRACE_OBJ
    const QString &component = docInfo.first;
    const QStringList &info = docInfo.second;
    QDateTime dt;
    if (!info.isEmpty() && !info.first().isEmpty())
        dt = QDateTime::fromString(info.first(), Qt::ISODate);

    QString qchFile;
    if (info.size() == 2)
        qchFile = info.last();

    if (m_qchFiles.isEmpty()) {
        emit qchFileNotFound(component);
        return false;
    }
    for (const QString &f : std::as_const(m_qchFiles)) {
        if (f.startsWith(component)) {
            QFileInfo fi(m_qchDir.absolutePath() + QDir::separator() + f);
            if (dt.isValid() && fi.lastModified().toSecsSinceEpoch() == dt.toSecsSinceEpoch()
                && qchFile == fi.absoluteFilePath())
                return false;
            emit registerDocumentation(component, fi.absoluteFilePath());
            return true;
        }
    }

    emit qchFileNotFound(component);
    return false;
}

QT_END_NAMESPACE
