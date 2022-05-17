// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GLOBALACTIONS_H
#define GLOBALACTIONS_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtGui/qtguiglobal.h>

QT_BEGIN_NAMESPACE

class QAction;
class QMenu;

class GlobalActions : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalActions)
public:
    static GlobalActions *instance(QObject *parent = nullptr);

    QList<QAction *> actionList() const { return m_actionList; }
    QAction *backAction() const { return m_backAction; }
    QAction *nextAction() const { return m_nextAction; }
    QAction *homeAction() const { return m_homeAction; }
    QAction *zoomInAction() const { return m_zoomInAction; }
    QAction *zoomOutAction() const { return m_zoomOutAction; }
#if QT_CONFIG(clipboard)
    QAction *copyAction() const { return m_copyAction; }
#endif
    QAction *printAction() const { return m_printAction; }
    QAction *findAction() const { return m_findAction; }

public slots:
#if QT_CONFIG(clipboard)
    void setCopyAvailable(bool available);
#endif
    void updateActions();

#if defined(BROWSER_QTWEBKIT)
private slots:
    void slotAboutToShowBackMenu();
    void slotAboutToShowNextMenu();
    void slotOpenActionUrl(QAction *action);
#endif // BROWSER_QTWEBKIT

private:
    void setupNavigationMenus(QAction *back, QAction *next, QWidget *parent);

private:
    GlobalActions(QObject *parent);

    static GlobalActions *m_instance;

    QAction *m_backAction;
    QAction *m_nextAction;
    QAction *m_homeAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
#if QT_CONFIG(clipboard)
    QAction *m_copyAction;
#endif
    QAction *m_printAction;
    QAction *m_findAction;

    QList<QAction *> m_actionList;

    QMenu *m_backMenu;
    QMenu *m_nextMenu;
};

QT_END_NAMESPACE

#endif // GLOBALACTIONS_H
