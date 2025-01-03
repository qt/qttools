// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
