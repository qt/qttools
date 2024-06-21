// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDBUSVIEWER_H
#define QDBUSVIEWER_H

#include <QtWidgets/QWidget>
#include <QtDBus/QDBusConnection>
#include <QtCore/QRegularExpression>

class ServicesProxyModel;

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QTreeView)
QT_FORWARD_DECLARE_CLASS(QTreeWidget)
QT_FORWARD_DECLARE_CLASS(QStringListModel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QTextBrowser)
QT_FORWARD_DECLARE_CLASS(QDomDocument)
QT_FORWARD_DECLARE_CLASS(QDomElement)
QT_FORWARD_DECLARE_CLASS(QSplitter)
QT_FORWARD_DECLARE_CLASS(QSettings)

struct BusSignature
{
    QString mService, mPath, mInterface, mName;
    QString mTypeSig;
};

class QDBusViewer: public QWidget
{
    Q_OBJECT
public:
    QDBusViewer(const QDBusConnection &connection, QWidget *parent = 0);

    void saveState(QSettings *settings) const;
    void restoreState(const QSettings *settings);

public slots:
    void refresh();

private slots:
    void serviceChanged(const QModelIndex &index);
    void showContextMenu(const QPoint &);
    void connectionRequested(const BusSignature &sig);
    void callMethod(const BusSignature &sig);
    void getProperty(const BusSignature &sig);
    void setProperty(const BusSignature &sig);
    void dumpMessage(const QDBusMessage &msg);
    void dumpError(const QDBusError &error);
    void refreshChildren();

    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

    void serviceFilterReturnPressed();
    void activate(const QModelIndex &item);

    void logError(const QString &msg);
    void anchorClicked(const QUrl &url);

private:
    void serviceRegistered(const QString &service);
    void logMessage(const QString &msg);
    void showEvent(QShowEvent *) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    QDBusConnection c;
    QString currentService;
    QTreeView *tree;
    QAction *refreshAction;
    QStringListModel *servicesModel;
    ServicesProxyModel *servicesProxyModel;
    QLineEdit *serviceFilterLine;
    QTableView *servicesView;
    QTextBrowser *log;
    QSplitter *topSplitter;
    QSplitter *splitter;
    QRegularExpression objectPathRegExp;
};

#endif
