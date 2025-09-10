// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
