// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FILEVERIFIER_H
#define FILEVERIFIER_H

#include <QString>

QT_BEGIN_NAMESPACE

class RecordDirectory;
class ConversionData;
class Translator;
class TranslatorMessage;

class FileVerifier
{
public:
    FileVerifier(const RecordDirectory &records, bool quiet);

    void verifySources(const QStringList &sources, ConversionData &cd);
    bool verifyTs(const QString &tsFile, QSet<QString> &transformedIds);

private:
    const RecordDirectory &m_records;
    bool m_quiet;
};

QT_END_NAMESPACE

#endif // FILEVERIFIER_H
