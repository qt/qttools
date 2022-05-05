/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "statistics.h"

QT_BEGIN_NAMESPACE

Statistics::Statistics(QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent, fl)
{
    setupUi(this);
}

void Statistics::languageChange()
{
    retranslateUi(this);
}

void Statistics::updateStats(const StatisticalData& newStats)
{
    int totals = newStats.translatedMsgDanger + newStats.translatedMsgNoDanger
               + newStats.unfinishedMsgNoDanger + newStats.unfinishedMsgDanger;
    int totalsWithObsolete = totals + newStats.obsoleteMsg;
    int unfinished = newStats.unfinishedMsgDanger + newStats.unfinishedMsgNoDanger;
    int finished = newStats.translatedMsgNoDanger + newStats.translatedMsgDanger;

    wordsSourceTextbox->setText(QString::number(newStats.wordsSource));
    charsSourceTextbox->setText(QString::number(newStats.charsSource));
    charsSpacesSourceTextbox->setText(QString::number(newStats.charsSpacesSource));
    wordsFinishedTextbox->setText(QString::number(newStats.wordsFinished));
    charsFinishedTextbox->setText(QString::number(newStats.charsFinished));
    charsSpacesFinishedTextbox->setText(QString::number(newStats.charsSpacesFinished));
    wordsUnfinishedTextbox->setText(QString::number(newStats.wordsUnfinished));
    charsUnfinishedTextbox->setText(QString::number(newStats.charsUnfinished));
    charsSpacesUnfinishedTextbox->setText(QString::number(newStats.charsSpacesUnfinished));
    totalMessagesTextbox->setText(QString::number(totals));
    totalWithObsoleteTextbox->setText(QString::number(totalsWithObsolete));
    totalFinishedTextbox->setText(QString::number(finished));
    finishedWithoutWarningsTextbox->setText(QString::number(newStats.translatedMsgNoDanger));
    finishedWithWarningsTextbox->setText(QString::number(newStats.translatedMsgDanger));
    unfinishedNoObsTextbox->setText(QString::number(unfinished));
    unfinishedNoWarningsTextbox->setText(QString::number(newStats.unfinishedMsgNoDanger));
    unfinishedWithWarningsTextbox->setText(QString::number(newStats.unfinishedMsgDanger));
}

QT_END_NAMESPACE
