// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WORLDTIMECLOCK_H
#define WORLDTIMECLOCK_H

#include <QTime>
#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

//! [0] //! [1]
class QDESIGNER_WIDGET_EXPORT WorldTimeClock : public QWidget
{
    Q_OBJECT
//! [0]

public:
    explicit WorldTimeClock(QWidget *parent = nullptr);

public slots:
    void setTimeZone(int hourOffset);

signals:
    void updated(QTime currentTime);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int timeZoneOffset = 0;
//! [2]
};
//! [1] //! [2]

#endif
