// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
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
    void generateExampleManifestFile();
    void readManifestMetaContent();
    QString retrieveExampleInstallationPath(const ExampleNode *example) const;

private:
    QString m_manifestDir {};
    QString m_examplesPath {};
    QString m_outputDirectory {};
    QString m_project {};
    QDocDatabase *m_qdb { nullptr };
    QList<ManifestMetaFilter> m_manifestMetaContent {};
    QStringList m_exampleCategories {};

    template <typename F>
    void processManifestMetaContent(const QString &fullName, F matchFunc);
};

QT_END_NAMESPACE

#endif // MANIFESTWRITER_H
