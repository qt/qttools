// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPGENERATOR_H
#define HELPGENERATOR_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QHelpProjectData;
class HelpGeneratorPrivate;

class HelpGenerator : public QObject
{
    Q_OBJECT

public:
    HelpGenerator(bool silent = false);
    bool generate(QHelpProjectData *helpData,
        const QString &outputFileName);
    bool checkLinks(const QHelpProjectData &helpData);
    QString error() const;

private slots:
    void printStatus(const QString &msg);
    void printWarning(const QString &msg);

private:
    HelpGeneratorPrivate *m_private;
};

QT_END_NAMESPACE

#endif
