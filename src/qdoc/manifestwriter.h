/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/
#ifndef MANIFESTWRITER_H
#define MANIFESTWRITER_H

#include <QtCore/qlist.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class ExampleNode;
class QDocDatabase;
class QXmlStreamWriter;
class ManifestWriter
{
    struct ManifestMetaFilter
    {
        QSet<QString> m_names {};
        QSet<QString> m_attributes {};
        QSet<QString> m_tags {};
    };

public:
    ManifestWriter();
    void generateManifestFiles();
    void generateManifestFile(const QString &manifest, const QString &element);
    void readManifestMetaContent();
    QString retrieveExampleInstallationPath(const ExampleNode *example) const;

private:
    QSet<QString> m_tags {};
    QString m_manifestDir {};
    QString m_examplesPath {};
    QString m_outputDirectory {};
    QString m_project {};
    QDocDatabase *m_qdb { nullptr };
    QList<ManifestMetaFilter> m_manifestMetaContent {};

    void addTitleWordsToTags(const ExampleNode *example);
    void addWordsFromModuleNamesAsTags();
    void includeTagsAddedWithMetaCommand(const ExampleNode *example);
    void cleanUpTags();
    void writeTagsElement(QXmlStreamWriter *writer);
    template <typename F>
    void processManifestMetaContent(const QString &fullName, F matchFunc);
};

QT_END_NAMESPACE

#endif // MANIFESTWRITER_H
