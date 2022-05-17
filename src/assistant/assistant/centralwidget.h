// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtCore/QUrl>
#include <QtCore/QHash>

#include <QtWidgets/QTabBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class FindWidget;
class HelpViewer;
class QStackedWidget;
class QPrinter;

class TabBar : public QTabBar
{
    Q_OBJECT
    friend class CentralWidget;

public:
    ~TabBar();

    int addNewTab(const QString &title);
    void setCurrent(HelpViewer *viewer);
    void removeTabAt(HelpViewer *viewer);

public slots:
    void titleChanged();

signals:
    void currentTabChanged(HelpViewer *viewer);
    void addBookmark(const QString &title, const QString &url);

private:
    TabBar(QWidget *parent = nullptr);

private slots:
    void slotCurrentChanged(int index);
    void slotTabCloseRequested(int index);
    void slotCustomContextMenuRequested(const QPoint &pos);
};

class CentralWidget : public QWidget
{
    Q_OBJECT
    friend class OpenPagesManager;

public:
    CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget() override;

    static CentralWidget *instance();

    QUrl currentSource() const;
    QString currentTitle() const;

    bool hasSelection() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;

    HelpViewer *viewerAt(int index) const;
    HelpViewer *currentHelpViewer() const;
    int currentIndex() const;

    void connectTabBar();

public slots:
#if QT_CONFIG(clipboard)
    void copy();
#endif
    void home();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void forward();
    void nextPage();

    void backward();
    void previousPage();

    void print();
    void pageSetup();
    void printPreview();

    void setSource(const QUrl &url);
    void setSourceFromSearch(const QUrl &url);

    void findNext();
    void findPrevious();
    void find(const QString &text, bool forward, bool incremental);

    void activateTab();
    void showTextSearch();
    void updateBrowserFont();
    void updateUserInterface();

signals:
    void currentViewerChanged();
#if QT_CONFIG(clipboard)
    void copyAvailable(bool yes);
#endif
    void sourceChanged(const QUrl &url);
    void highlighted(const QUrl &link);
    void forwardAvailable(bool available);
    void backwardAvailable(bool available);
    void addBookmark(const QString &title, const QString &url);

protected:
    void keyPressEvent(QKeyEvent *) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void highlightSearchTerms();
    void printPreviewToPrinter(QPrinter *printer);
    void handleSourceChanged(const QUrl &url);
    void slotHighlighted(const QUrl& link);

private:
    void initPrinter();
    void connectSignals(HelpViewer *page);
    bool eventFilter(QObject *object, QEvent *e) override;

    void removePage(int index);
    void setCurrentPage(HelpViewer *page);
    void addPage(HelpViewer *page, bool fromSearch = false);

private:
#ifndef QT_NO_PRINTER
    QPrinter *m_printer;
#endif
    FindWidget *m_findWidget;
    QStackedWidget *m_stackedWidget;
    TabBar *m_tabBar;
    QHash<QUrl, QUrl> m_resolvedLinks;
};

QT_END_NAMESPACE

#endif  // CENTRALWIDGET_H
