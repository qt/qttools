// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QFile>
#include <QHelpContentItem>
#include <QHelpEngineCore>
#include <QHelpLink>
#include <QTemporaryDir>

#include <algorithm>

// Registers the input as a documentation file (.qch) in an empty collection file and reads
// everything back. Documentation files are SQLite databases, which may come from third
// parties, so a seed corpus of valid files, e.g. generated with qhelpgenerator, is needed
// to get past the SQLite file format.

// silence warnings
static QtMessageHandler mh = qInstallMessageHandler([](QtMsgType, const QMessageLogContext &,
                                                       const QString &) {});

// Bounds the work done for a single input.
static constexpr qsizetype MaxItems = 100;

static bool writeFile(const QString &fileName, const QByteArray &contents)
{
    QFile file(fileName);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate)
            && file.write(contents) == contents.size();
}

static void visit(const QHelpContentItem *item, int depth)
{
    if (!item || depth > MaxItems)
        return;
    const int count = std::min(item->childCount(), int(MaxItems));
    for (int i = 0; i < count; ++i)
        visit(item->child(i), depth + 1);
}

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size) {
    static int argc = 1;
    static char arg1[] = "fuzzer";
    static char *argv[] = {arg1, nullptr};
    static QCoreApplication app(argc, argv);
    static QTemporaryDir dir;
    static const QString qchFile = dir.filePath(QStringLiteral("fuzz.qch"));
    static const QString colFile = dir.filePath(QStringLiteral("fuzz.qhc"));
    static const QByteArray emptyCollection = [] {
        {
            QHelpEngineCore c(colFile);
            c.setReadOnly(false);
            c.setupData();
        }
        QFile file(colFile);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }();

    if (!dir.isValid() || emptyCollection.isEmpty()
            || !writeFile(qchFile, QByteArray::fromRawData(Data, Size))
            || !writeFile(colFile, emptyCollection)) {
        return 0;
    }

    QHelpEngineCore::namespaceName(qchFile);
    QHelpEngineCore::metaData(qchFile, QStringLiteral("author"));

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    c.setUsesFilterEngine(true);
    if (!c.setupData() || !c.registerDocumentation(qchFile))
        return 0;

    for (const QString &ns : c.registeredDocumentations()) {
        c.filterAttributeSets(ns);
        const QList<QUrl> files = c.files(ns, QString());
        for (qsizetype i = 0; i < std::min(files.size(), MaxItems); ++i)
            c.fileData(c.findFile(files.at(i)));
    }

    const QStringList keywords = c.requestIndex(QString()).result();
    for (qsizetype i = 0; i < std::min(keywords.size(), MaxItems); ++i)
        c.documentsForKeyword(keywords.at(i));

    const std::shared_ptr<QHelpContentItem> root = c.requestContent(QString()).result();
    visit(root.get(), 0);
    return 0;
}
