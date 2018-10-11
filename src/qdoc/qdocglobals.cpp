/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdocglobals.h"

QDocGlobals::QDocGlobals()
{
}

bool QDocGlobals::highlighting()
{
    return m_highlighting;
}

void QDocGlobals::enableHighlighting(bool value)
{
    m_highlighting = value;
}

bool QDocGlobals::showInternal()
{
    return m_showInternal;
}

void QDocGlobals::setShowInternal(bool value)
{
    m_showInternal = value;
}

bool QDocGlobals::singleExec()
{
    return m_singleExec;
}
void QDocGlobals::setSingleExec(bool value)
{
    m_singleExec = value;
}

bool QDocGlobals::writeQaPages()
{
    return m_writeQaPages;
}
void QDocGlobals::setWriteQaPages(bool value)
{
    m_writeQaPages = value;
}

bool QDocGlobals::redirectDocumentationToDevNull()
{
    return m_redirectDocumentationToDevNull;
}

void QDocGlobals::setRedirectDocumentationToDevNull(bool value)
{
    m_redirectDocumentationToDevNull = value;
}

bool QDocGlobals::noLinkErrors()
{
    return m_noLinkErrors;
}

void QDocGlobals::setNoLinkErrors(bool value)
{
    m_noLinkErrors = value;
}

bool QDocGlobals::autolinkErrors()
{
    return m_autolinkErrors;
}

void QDocGlobals::setAutolinkErrors(bool value)
{
    m_autolinkErrors = value;
}

bool QDocGlobals::obsoleteLinks()
{
    return m_obsoleteLinks;
}

void QDocGlobals::setObsoleteLinks(bool value)
{
    m_obsoleteLinks = value;
}

QStringList QDocGlobals::defines()
{
    return m_defines;
}

void QDocGlobals::addDefine(const QStringList &valueList)
{
    m_defines += valueList;
}

QStringList QDocGlobals::includesPaths()
{
    return m_includesPaths;
}

void QDocGlobals::addIncludePath(const QString &flag, const QString &path)
{
    QString includePath = flag + path;
    m_includesPaths << includePath;
}

QStringList &QDocGlobals::dependModules()
{
    return m_dependModules;
}

QStringList QDocGlobals::indexDirs()
{
    return m_indexDirs;
}

void QDocGlobals::appendToIndexDirs(const QString &path)
{
    m_indexDirs += path;
}

QString QDocGlobals::currentDir()
{
    return m_currentDir;
}

void QDocGlobals::setCurrentDir(const QString &path)
{
    m_currentDir = path;
}

QString QDocGlobals::previousCurrentDir()
{
    return m_previousCurrentDir;
}

void QDocGlobals::setPreviousCurrentDir(const QString &path)
{
    m_previousCurrentDir = path;
}

QHash<QString, QString> &QDocGlobals::defaults()
{
    return m_defaults;
}
