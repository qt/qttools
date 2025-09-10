// Copyright (C) 2016 Tasuku Suzuki <stasuku@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "logviewer.h"

#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMenu>

LogViewer::LogViewer(QWidget *parent)
    : QTextBrowser(parent)
{
}

void LogViewer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    QAction *action = menu->addAction(tr("Clear"));
    connect(action, &QAction::triggered, this, &QTextEdit::clear);
    menu->exec(event->globalPos());
    delete menu;
}
