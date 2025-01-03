// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "indexwindow.h"

#include "centralwidget.h"
#include "helpenginewrapper.h"
#include "helpviewer.h"
#include "openpagesmanager.h"
#include "topicchooser.h"
#include "tracer.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QListWidgetItem>

#include <QtHelp/QHelpIndexWidget>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpLink>

QT_BEGIN_NAMESPACE

IndexWindow::IndexWindow(QWidget *parent)
    : QWidget(parent)
    , m_searchLineEdit(new QLineEdit)
    , m_indexWidget(HelpEngineWrapper::instance().indexWidget())
{
    TRACE_OBJ
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *l = new QLabel(tr("&Look for:"));
    layout->addWidget(l);

    l->setBuddy(m_searchLineEdit);
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &QLineEdit::textChanged,
            this, &IndexWindow::filterIndices);
    m_searchLineEdit->installEventFilter(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(m_searchLineEdit);

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    m_indexWidget->installEventFilter(this);
    connect(helpEngine.indexModel(), &QHelpIndexModel::indexCreationStarted,
            this, &IndexWindow::disableSearchLineEdit);
    connect(helpEngine.indexModel(), &QHelpIndexModel::indexCreated,
            this, &IndexWindow::enableSearchLineEdit);
    connect(m_indexWidget, &QHelpIndexWidget::documentActivated,
            this, [this](const QHelpLink &link) {
        emit linkActivated(link.url);
    });
    connect(m_indexWidget, &QHelpIndexWidget::documentsActivated,
            this, &IndexWindow::documentsActivated);
    connect(m_searchLineEdit, &QLineEdit::returnPressed,
            m_indexWidget, &QHelpIndexWidget::activateCurrentItem);
    layout->addWidget(m_indexWidget);

    m_indexWidget->viewport()->installEventFilter(this);
}

IndexWindow::~IndexWindow()
{
    TRACE_OBJ
}

void IndexWindow::filterIndices(const QString &filter)
{
    TRACE_OBJ
    if (filter.contains(QLatin1Char('*')))
        m_indexWidget->filterIndices(filter, filter);
    else
        m_indexWidget->filterIndices(filter, QString());
}

bool IndexWindow::eventFilter(QObject *obj, QEvent *e)
{
    TRACE_OBJ
    if (obj == m_searchLineEdit && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        QModelIndex idx = m_indexWidget->currentIndex();
        switch (ke->key()) {
        case Qt::Key_Up:
            idx = m_indexWidget->model()->index(idx.row()-1,
                idx.column(), idx.parent());
            if (idx.isValid()) {
                m_indexWidget->setCurrentIndex(idx);
                return true;
            }
            break;
        case Qt::Key_Down:
            idx = m_indexWidget->model()->index(idx.row() + 1,
                idx.column(), idx.parent());
            if (idx.isValid()) {
                m_indexWidget->setCurrentIndex(idx);
                return true;
            }
            break;
        case Qt::Key_Escape:
            emit escapePressed();
            return true;
        default: ; // stop complaining
        }
    } else if (obj == m_indexWidget && e->type() == QEvent::ContextMenu) {
        QContextMenuEvent *ctxtEvent = static_cast<QContextMenuEvent*>(e);
        QModelIndex idx = m_indexWidget->indexAt(ctxtEvent->pos());
        if (idx.isValid()) {
            QMenu menu;
            QAction *curTab = menu.addAction(tr("Open Link"));
            QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
            menu.move(m_indexWidget->mapToGlobal(ctxtEvent->pos()));

            QAction *action = menu.exec();
            if (curTab == action)
                m_indexWidget->activateCurrentItem();
            else if (newTab == action) {
                open(m_indexWidget, idx);
            }
        }
    } else if (m_indexWidget && obj == m_indexWidget->viewport()
        && e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        QModelIndex idx = m_indexWidget->indexAt(mouseEvent->pos());
        if (idx.isValid()) {
            Qt::MouseButtons button = mouseEvent->button();
            if (((button == Qt::LeftButton) && (mouseEvent->modifiers() & Qt::ControlModifier))
                || (button == Qt::MiddleButton)) {
                open(m_indexWidget, idx);
            }
        }
    }
#ifdef Q_OS_MAC
    else if (obj == m_indexWidget && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
           m_indexWidget->activateCurrentItem();
    }
#endif
    return QWidget::eventFilter(obj, e);
}

void IndexWindow::enableSearchLineEdit()
{
    TRACE_OBJ
    m_searchLineEdit->setDisabled(false);
    filterIndices(m_searchLineEdit->text());
}

void IndexWindow::disableSearchLineEdit()
{
    TRACE_OBJ
    m_searchLineEdit->setDisabled(true);
}

void IndexWindow::setSearchLineEditText(const QString &text)
{
    TRACE_OBJ
    m_searchLineEdit->setText(text);
}

void IndexWindow::focusInEvent(QFocusEvent *e)
{
    TRACE_OBJ
    if (e->reason() != Qt::MouseFocusReason) {
        m_searchLineEdit->selectAll();
        m_searchLineEdit->setFocus();
    }
}

void IndexWindow::open(QHelpIndexWidget* indexWidget, const QModelIndex &index)
{
    TRACE_OBJ
    QHelpIndexModel *model = qobject_cast<QHelpIndexModel*>(indexWidget->model());
    if (model) {
        const QString keyword = model->data(index, Qt::DisplayRole).toString();
        const QList<QHelpLink> docs = model->helpEngine()->documentsForKeyword(keyword);

        QUrl url;
        if (docs.size() > 1) {
            TopicChooser tc(this, keyword, docs);
            if (tc.exec() == QDialog::Accepted)
                url = tc.link();
        } else if (!docs.isEmpty()) {
            url = docs.first().url;
        } else {
            return;
        }

        if (!HelpViewer::canOpenPage(url.path()))
            CentralWidget::instance()->setSource(url);
        else
            OpenPagesManager::instance()->createPage(url);
    }
}

QT_END_NAMESPACE
