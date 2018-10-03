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

#ifndef QDOCGLOBALS_H
#define QDOCGLOBALS_H

#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qtranslator.h>

QT_BEGIN_NAMESPACE
class QDocGlobals
{
public:
    QDocGlobals();

    bool highlighting();
    void enableHighlighting(bool value);

    bool showInternal();
    void setShowInternal(bool value);

    bool singleExec();
    void setSingleExec(bool value);

    bool writeQaPages();
    void setWriteQaPages(bool value);

    bool redirectDocumentationToDevNull();
    void setRedirectDocumentationToDevNull(bool value);

    bool noLinkErrors();
    void setNoLinkErrors(bool value);

    bool autolinkErrors();
    void setAutolinkErrors(bool value);

    bool obsoleteLinks();
    void setObsoleteLinks(bool value);

    QStringList defines();
    void addDefine(const QStringList &valueList);

    QStringList includesPaths();
    void addIncludePath(const QString &flag, const QString &path);

    QStringList &dependModules();

    QStringList indexDirs();
    void appendToIndexDirs(const QString &path);

    QString currentDir();
    void setCurrentDir(const QString &path);

    QString previousCurrentDir();
    void setPreviousCurrentDir(const QString &path);

    QHash<QString, QString> &defaults();

private:
    bool m_highlighting = false;
    bool m_showInternal = false;
    bool m_singleExec = false;
    bool m_writeQaPages = false;
    bool m_redirectDocumentationToDevNull = false;
    bool m_noLinkErrors = false;
    bool m_autolinkErrors = false;
    bool m_obsoleteLinks = false;

    QStringList m_defines;
    QStringList m_includesPaths;
    QStringList m_dependModules;
    QStringList m_indexDirs;
    QString m_currentDir;
    QString m_previousCurrentDir;
    QHash<QString,QString> m_defaults;
};

QT_END_NAMESPACE

#endif // QDOCGLOBALS_H
