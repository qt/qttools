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
******************************************************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QtCore/QUrl>
#include <QtCore/QPoint>

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QMouseEvent;
class QHelpSearchEngine;
class QHelpSearchResultWidget;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QHelpSearchEngine *engine, QWidget *parent = nullptr);
    ~SearchWidget() override;

    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void requestShowLink(const QUrl &url);
    void requestShowLinkInNewTab(const QUrl &url);

private slots:
    void search() const;
    void searchingStarted();
    void searchingFinished(int searchResultCount);

private:
    bool eventFilter(QObject* o, QEvent *e) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;
    void contextMenuEvent(QContextMenuEvent *contextMenuEvent) override;

private:
    int zoomCount;
    QHelpSearchEngine *searchEngine;
    QHelpSearchResultWidget *resultWidget;
};

QT_END_NAMESPACE

#endif  // SEARCHWIDGET_H
