// Copyright (C) 2016 Tasuku Suzuki <stasuku@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QtWidgets/QTextBrowser>

class LogViewer : public QTextBrowser
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // LOGVIEWER_H
