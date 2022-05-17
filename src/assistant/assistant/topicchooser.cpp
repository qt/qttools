// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include "topicchooser.h"
#include "helpenginewrapper.h"

#include <QKeyEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QUrl>

#include <QtHelp/QHelpLink>

QT_BEGIN_NAMESPACE

TopicChooser::TopicChooser(QWidget *parent, const QString &keyword, const QList<QHelpLink> &docs)
    : QDialog(parent)
    , m_filterModel(new QSortFilterProxyModel(this))
{
    TRACE_OBJ
    ui.setupUi(this);

    setFocusProxy(ui.lineEdit);
    ui.lineEdit->installEventFilter(this);
    ui.lineEdit->setPlaceholderText(tr("Filter"));
    ui.label->setText(tr("Choose a topic for <b>%1</b>:").arg(keyword));

    QStandardItemModel *model = new QStandardItemModel(this);
    m_filterModel->setSourceModel(model);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    for (const auto &doc : docs) {
        m_links.append(doc.url);
        QStandardItem *item = new QStandardItem(doc.title);
        item->setToolTip(doc.url.toString());
        model->appendRow(item);
    }

    ui.listWidget->setModel(m_filterModel);
    ui.listWidget->setUniformItemSizes(true);
    ui.listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (m_filterModel->rowCount() != 0)
        ui.listWidget->setCurrentIndex(m_filterModel->index(0, 0));

    connect(ui.buttonCancel, &QAbstractButton::clicked,
            this, &QDialog::reject);
    connect(ui.buttonDisplay, &QAbstractButton::clicked,
            this, &TopicChooser::acceptDialog);
    connect(ui.lineEdit, &QLineEdit::textChanged,
            this, &TopicChooser::setFilter);
    connect(ui.listWidget, &QAbstractItemView::activated,
            this, &TopicChooser::activated);

    const QByteArray ba = HelpEngineWrapper::instance().topicChooserGeometry();
    if (!ba.isEmpty())
        restoreGeometry(ba);
}

TopicChooser::~TopicChooser()
{
    HelpEngineWrapper::instance().setTopicChooserGeometry(saveGeometry());
}

QUrl TopicChooser::link() const
{
    TRACE_OBJ
    if (m_activedIndex.isValid())
        return m_links.at(m_filterModel->mapToSource(m_activedIndex).row());
    return QUrl();
}

void TopicChooser::acceptDialog()
{
    TRACE_OBJ
    m_activedIndex = ui.listWidget->currentIndex();
    accept();
}

void TopicChooser::setFilter(const QString &pattern)
{
    TRACE_OBJ
    m_filterModel->setFilterFixedString(pattern);
    if (m_filterModel->rowCount() != 0 && !ui.listWidget->currentIndex().isValid())
        ui.listWidget->setCurrentIndex(m_filterModel->index(0, 0));
}

void TopicChooser::activated(const QModelIndex &index)
{
    TRACE_OBJ
    m_activedIndex = index;
    accept();
}

bool TopicChooser::eventFilter(QObject *object, QEvent *event)
{
    TRACE_OBJ
    if (object == ui.lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            QCoreApplication::sendEvent(ui.listWidget, event);
            break;
        }
    } else if (ui.lineEdit && event->type() == QEvent::FocusIn
        && static_cast<QFocusEvent *>(event)->reason() != Qt::MouseFocusReason) {
            ui.lineEdit->selectAll();
            ui.lineEdit->setFocus();
    }
    return QDialog::eventFilter(object, event);
}

QT_END_NAMESPACE
