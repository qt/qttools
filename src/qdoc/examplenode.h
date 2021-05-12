/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef EXAMPLENODE_H
#define EXAMPLENODE_H

#include "pagenode.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class ExampleNode : public PageNode
{
public:
    ExampleNode(Aggregate *parent, const QString &name) : PageNode(Node::Example, parent, name) {}
    [[nodiscard]] QString imageFileName() const override { return m_imageFileName; }
    void setImageFileName(const QString &ifn) override { m_imageFileName = ifn; }
    [[nodiscard]] const QStringList &files() const { return m_files; }
    [[nodiscard]] const QStringList &images() const { return m_images; }
    [[nodiscard]] const QString &projectFile() const { return m_projectFile; }
    void setFiles(const QStringList &files, const QString &projectFile)
    {
        m_files = files;
        m_projectFile = projectFile;
    }
    void setImages(const QStringList &images) { m_images = images; }
    void appendFile(QString &file) { m_files.append(file); }
    void appendImage(QString &image) { m_images.append(image); }

private:
    QString m_imageFileName {};
    QString m_projectFile {};
    QStringList m_files {};
    QStringList m_images {};
};

QT_END_NAMESPACE

#endif // EXAMPLENODE_H
