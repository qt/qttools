/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
