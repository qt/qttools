// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef STATISTICS_H
#define STATISTICS_H

#include "ui_statistics.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

struct StatisticalData
{
    int wordsSource;
    int charsSource;
    int charsSpacesSource;
    int wordsFinished;
    int charsFinished;
    int charsSpacesFinished;
    int wordsUnfinished;
    int charsUnfinished;
    int charsSpacesUnfinished;
    int translatedMsgNoDanger;
    int translatedMsgDanger;
    int obsoleteMsg;
    int unfinishedMsgNoDanger;
    int unfinishedMsgDanger;
};

class Statistics : public QDialog, public Ui::Statistics
{
    Q_OBJECT

public:
    Statistics(QWidget *parent = 0, Qt::WindowFlags fl = {});
    ~Statistics() {}

public slots:
    virtual void updateStats(const StatisticalData &newStats);

protected slots:
    virtual void languageChange();
};

QT_END_NAMESPACE

#endif // STATISTICS_H
