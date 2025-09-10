// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OPENPAGESMANAGER_H
#define OPENPAGESMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QAbstractItemView;
class QModelIndex;
class QUrl;

class HelpViewer;
class OpenPagesModel;
class OpenPagesSwitcher;
class OpenPagesWidget;

class OpenPagesManager : public QObject
{
    Q_OBJECT
public:
    static OpenPagesManager *createInstance(QObject *parent,
        bool defaultCollection, const QUrl &cmdLineUrl);
    static OpenPagesManager *instance();

    bool pagesOpenForNamespace(const QString &nameSpace) const;
    void closePages(const QString &nameSpace);
    void reloadPages(const QString &nameSpace);

    QAbstractItemView* openPagesWidget() const;

    int pageCount() const;
    void setCurrentPage(int index);

    void resetHelpPage();

public slots:
    HelpViewer *createPage(const QUrl &url, bool fromSearch = false);
    HelpViewer *createNewPageFromSearch(const QUrl &url);
    HelpViewer *createBlankPage();
    void closeCurrentPage();

    void nextPage();
    void nextPageWithSwitcher();
    void previousPage();
    void previousPageWithSwitcher();

    void closePage(HelpViewer *page);
    void setCurrentPage(HelpViewer *page);

signals:
    void aboutToAddPage();
    void pageAdded(int index);

    void pageClosed();
    void aboutToClosePage(int index);

private slots:
    void setCurrentPage(const QModelIndex &index);
    void closePage(const QModelIndex &index);
    void closePagesExcept(const QModelIndex &index);

private:
    OpenPagesManager(QObject *parent, bool defaultCollection,
        const QUrl &cmdLineUrl);
    ~OpenPagesManager();

    void setupInitialPages(bool defaultCollection, const QUrl &cmdLineUrl);
    void closeOrReloadPages(const QString &nameSpace, bool tryReload);
    void removePage(int index);

    void nextOrPreviousPage(int offset);
    void showSwitcherOrSelectPage() const;

    OpenPagesModel *m_model;
    OpenPagesWidget *m_openPagesWidget = nullptr;
    OpenPagesSwitcher *m_openPagesSwitcher = nullptr;

    QPointer<HelpViewer> m_helpPageViewer;

    static OpenPagesManager *m_instance;
};

QT_END_NAMESPACE

#endif // OPENPAGESMANAGER_H
