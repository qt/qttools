// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globalactions.h"

#include "centralwidget.h"
#include "helpviewer.h"
#include "tracer.h"

#include <QtWidgets/QMenu>

#include <QtGui/QAction>

#if defined(BROWSER_QTWEBKIT)
#  include <QWebHistory>
#endif

using namespace Qt::StringLiterals;

GlobalActions *GlobalActions::instance(QObject *parent)
{
    Q_ASSERT(!m_instance != !parent);
    if (!m_instance)
        m_instance = new GlobalActions(parent);
    return m_instance;
}

GlobalActions::GlobalActions(QObject *parent) : QObject(parent)
{
    TRACE_OBJ

    // TODO: Put resource path in misc class
    QString resourcePath = ":/qt-project.org/assistant/images/"_L1;
#ifdef Q_OS_MAC
    resourcePath.append("mac"_L1);
#else
    resourcePath.append("win"_L1);
#endif
    CentralWidget *centralWidget = CentralWidget::instance();

    m_backAction = new QAction(tr("&Back"), parent);
    m_backAction->setEnabled(false);
    m_backAction->setShortcuts(QKeySequence::Back);
    m_backAction->setIcon(QIcon(resourcePath + "/previous.png"_L1));
    connect(m_backAction, &QAction::triggered, centralWidget, &CentralWidget::backward);
    m_actionList << m_backAction;

    m_nextAction = new QAction(tr("&Forward"), parent);
    m_nextAction->setPriority(QAction::LowPriority);
    m_nextAction->setEnabled(false);
    m_nextAction->setShortcuts(QKeySequence::Forward);
    m_nextAction->setIcon(QIcon(resourcePath + "/next.png"_L1));
    connect(m_nextAction, &QAction::triggered, centralWidget, &CentralWidget::forward);
    m_actionList << m_nextAction;

    setupNavigationMenus(m_backAction, m_nextAction, centralWidget);

    m_homeAction = new QAction(tr("&Home"), parent);
    m_homeAction->setShortcut(tr("ALT+Home"));
    m_homeAction->setIcon(QIcon(resourcePath + "/home.png"_L1));
    connect(m_homeAction, &QAction::triggered, centralWidget, &CentralWidget::home);
    m_actionList << m_homeAction;

    QAction *separator = new QAction(parent);
    separator->setSeparator(true);
    m_actionList << separator;

    m_zoomInAction = new QAction(tr("Zoom &in"), parent);
    m_zoomInAction->setPriority(QAction::LowPriority);
    m_zoomInAction->setIcon(QIcon(resourcePath + "/zoomin.png"_L1));
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(m_zoomInAction, &QAction::triggered, centralWidget, &CentralWidget::zoomIn);
    m_actionList << m_zoomInAction;

    m_zoomOutAction = new QAction(tr("Zoom &out"), parent);
    m_zoomOutAction->setPriority(QAction::LowPriority);
    m_zoomOutAction->setIcon(QIcon(resourcePath + "/zoomout.png"_L1));
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(m_zoomOutAction, &QAction::triggered, centralWidget, &CentralWidget::zoomOut);
    m_actionList << m_zoomOutAction;

    separator = new QAction(parent);
    separator->setSeparator(true);
    m_actionList << separator;

#if QT_CONFIG(clipboard)
    m_copyAction = new QAction(tr("&Copy selected Text"), parent);
    m_copyAction->setPriority(QAction::LowPriority);
    m_copyAction->setIconText("&Copy");
    m_copyAction->setIcon(QIcon(resourcePath + "/editcopy.png"_L1));
    m_copyAction->setShortcuts(QKeySequence::Copy);
    m_copyAction->setEnabled(false);
    connect(m_copyAction, &QAction::triggered, centralWidget, &CentralWidget::copy);
    m_actionList << m_copyAction;
#endif

    m_printAction = new QAction(tr("&Print..."), parent);
    m_printAction->setPriority(QAction::LowPriority);
    m_printAction->setIcon(QIcon(resourcePath + "/print.png"_L1));
    m_printAction->setShortcut(QKeySequence::Print);
    connect(m_printAction, &QAction::triggered, centralWidget, &CentralWidget::print);
    m_actionList << m_printAction;

    m_findAction = new QAction(tr("&Find in Text..."), parent);
    m_findAction->setIconText(tr("&Find"));
    m_findAction->setIcon(QIcon(resourcePath + "/find.png"_L1));
    m_findAction->setShortcuts(QKeySequence::Find);
    connect(m_findAction, &QAction::triggered, centralWidget, &CentralWidget::showTextSearch);
    m_actionList << m_findAction;

#if defined (Q_OS_UNIX) && !defined(Q_OS_MAC)
    m_backAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious,
                                           m_backAction->icon()));
    m_nextAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext,
                                           m_nextAction->icon()));
    m_zoomInAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn,
                                             m_zoomInAction->icon()));
    m_zoomOutAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut,
                                              m_zoomOutAction->icon()));
#if QT_CONFIG(clipboard)
    m_copyAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditCopy,
                                           m_copyAction->icon()));
#endif
    m_findAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind,
                                           m_findAction->icon()));
    m_homeAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoHome,
                                           m_homeAction->icon()));
    m_printAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrint,
                                            m_printAction->icon()));
#endif
}

void GlobalActions::updateActions()
{
    TRACE_OBJ
    CentralWidget *centralWidget = CentralWidget::instance();
#if QT_CONFIG(clipboard)
    m_copyAction->setEnabled(centralWidget->hasSelection());
#endif
    m_nextAction->setEnabled(centralWidget->isForwardAvailable());
    m_backAction->setEnabled(centralWidget->isBackwardAvailable());
}

#if QT_CONFIG(clipboard)
void GlobalActions::setCopyAvailable(bool available)
{
    TRACE_OBJ
    m_copyAction->setEnabled(available);
}
#endif

#if defined(BROWSER_QTWEBKIT)

void GlobalActions::slotAboutToShowBackMenu()
{
    TRACE_OBJ
    m_backMenu->clear();
    if (QWebHistory *history = CentralWidget::instance()->currentHelpViewer()->history()) {
        const int currentItemIndex = history->currentItemIndex();
        QList<QWebHistoryItem> items = history->backItems(history->count());
        for (int i = items.count() - 1; i >= 0; --i) {
            QAction *action = new QAction(this);
            action->setText(items.at(i).title());
            action->setData(-1 * (currentItemIndex - i));
            m_backMenu->addAction(action);
        }
    }
}

void GlobalActions::slotAboutToShowNextMenu()
{
    TRACE_OBJ
    m_nextMenu->clear();
    if (QWebHistory *history = CentralWidget::instance()->currentHelpViewer()->history()) {
        const int count = history->count();
        QList<QWebHistoryItem> items = history->forwardItems(count);
        for (int i = 0; i < items.count(); ++i) {
            QAction *action = new QAction(this);
            action->setData(count - i);
            action->setText(items.at(i).title());
            m_nextMenu->addAction(action);
        }
    }
}

void GlobalActions::slotOpenActionUrl(QAction *action)
{
    TRACE_OBJ
    if (HelpViewer* viewer = CentralWidget::instance()->currentHelpViewer()) {
        const int offset = action->data().toInt();
        QWebHistory *history = viewer->history();
        if (offset > 0) {
            history->goToItem(history->forwardItems(history->count()
                - offset + 1).back());  // forward
        } else if (offset < 0) {
            history->goToItem(history->backItems(-1 * offset).first()); // back
        }
    }
}

#endif //  BROWSER_QTWEBKIT

void GlobalActions::setupNavigationMenus(QAction *back, QAction *next,
    QWidget *parent)
{
#if defined(BROWSER_QTWEBKIT)
    m_backMenu = new QMenu(parent);
    connect(m_backMenu, &QMenu::aboutToShow,
            this, &GlobalActions::slotAboutToShowBackMenu);
    connect(m_backMenu, &QMenu::triggered,
            this, &GlobalActions::slotOpenActionUrl);
    back->setMenu(m_backMenu);

    m_nextMenu = new QMenu(parent);
    connect(m_nextMenu, &QMenu::aboutToShow,
            this, &GlobalActions::slotAboutToShowNextMenu);
    connect(m_nextMenu, &QMenu::triggered,
            this, &GlobalActions::slotOpenActionUrl);
    next->setMenu(m_nextMenu);
#else
    Q_UNUSED(back);
    Q_UNUSED(next);
    Q_UNUSED(parent);
#endif
}

GlobalActions *GlobalActions::m_instance = nullptr;
