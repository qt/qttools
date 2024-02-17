// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"
#include "findwidget.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtGui/QHideEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , appPalette(qApp->palette())
{
    TRACE_OBJ
    installEventFilter(this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);
    QString resourcePath = ":/qt-project.org/assistant/images/"_L1;

#ifndef Q_OS_MAC
    hboxLayout->setContentsMargins({});
    hboxLayout->setSpacing(6);
    resourcePath.append("win"_L1);
#else
    resourcePath.append("mac"_L1);
#endif

    toolClose = setupToolButton({}, resourcePath + "/closetab.png"_L1);
    hboxLayout->addWidget(toolClose);
    connect(toolClose, &QAbstractButton::clicked, this, &QWidget::hide);

    editFind = new QLineEdit(this);
    hboxLayout->addWidget(editFind);
    editFind->setMinimumSize(QSize(150, 0));
    connect(editFind, &QLineEdit::textChanged, this, &FindWidget::textChanged);
    connect(editFind, &QLineEdit::returnPressed, this, &FindWidget::findNext);
    connect(editFind, &QLineEdit::textChanged, this, &FindWidget::updateButtons);

    toolPrevious = setupToolButton(tr("Previous"), resourcePath + "/previous.png"_L1);
    connect(toolPrevious, &QAbstractButton::clicked, this, &FindWidget::findPrevious);

    hboxLayout->addWidget(toolPrevious);

    toolNext = setupToolButton(tr("Next"), resourcePath + "/next.png"_L1);
    hboxLayout->addWidget(toolNext);
    connect(toolNext, &QAbstractButton::clicked, this, &FindWidget::findNext);

    checkCase = new QCheckBox(tr("Case Sensitive"), this);
    hboxLayout->addWidget(checkCase);

    labelWrapped = new QLabel(this);
    labelWrapped->setScaledContents(true);
    labelWrapped->setTextFormat(Qt::RichText);
    labelWrapped->setMinimumSize(QSize(0, 20));
    labelWrapped->setMaximumSize(QSize(105, 20));
    labelWrapped->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
    labelWrapped->setText(tr("<img src=\":/qt-project.org/assistant/images/wrap.png\""
        ">&nbsp;Search wrapped"));
    hboxLayout->addWidget(labelWrapped);

    QSpacerItem *spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding,
        QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem);
    setMinimumWidth(minimumSizeHint().width());
    labelWrapped->hide();

    updateButtons();
}

FindWidget::~FindWidget()
{
    TRACE_OBJ
}

void FindWidget::show()
{
    TRACE_OBJ
    QWidget::show();
    editFind->selectAll();
    editFind->setFocus(Qt::ShortcutFocusReason);
}

void FindWidget::showAndClear()
{
    TRACE_OBJ
    show();
    editFind->clear();
}

QString FindWidget::text() const
{
    TRACE_OBJ
    return editFind->text();
}

bool FindWidget::caseSensitive() const
{
    TRACE_OBJ
    return checkCase->isChecked();
}

void FindWidget::setPalette(bool found)
{
    TRACE_OBJ
    QPalette palette = editFind->palette();
    palette.setColor(QPalette::Active, QPalette::Base, found ? Qt::white
        : QColor(255, 102, 102));
    editFind->setPalette(palette);
}

void FindWidget::setTextWrappedVisible(bool visible)
{
    TRACE_OBJ
    labelWrapped->setVisible(visible);
}

void FindWidget::hideEvent(QHideEvent* event)
{
    TRACE_OBJ
#if defined(BROWSER_QTWEBKIT)
    // TODO: remove this once webkit supports setting the palette
    if (!event->spontaneous())
        qApp->setPalette(appPalette);
#else // BROWSER_QTWEBKIT
    Q_UNUSED(event);
#endif
}

void FindWidget::showEvent(QShowEvent* event)
{
    TRACE_OBJ
#if defined(BROWSER_QTWEBKIT)
    // TODO: remove this once webkit supports setting the palette
    if (!event->spontaneous()) {
        QPalette p = appPalette;
        p.setColor(QPalette::Inactive, QPalette::Highlight,
            p.color(QPalette::Active, QPalette::Highlight));
        p.setColor(QPalette::Inactive, QPalette::HighlightedText,
            p.color(QPalette::Active, QPalette::HighlightedText));
        qApp->setPalette(p);
    }
#else // BROWSER_QTWEBKIT
    Q_UNUSED(event);
#endif
}

void FindWidget::updateButtons()
{
    TRACE_OBJ
    const bool enable = !editFind->text().isEmpty();
    toolNext->setEnabled(enable);
    toolPrevious->setEnabled(enable);
}

void FindWidget::textChanged(const QString &text)
{
    TRACE_OBJ
    emit find(text, true, true);
}

bool FindWidget::eventFilter(QObject *object, QEvent *e)
{
    TRACE_OBJ
    if (e->type() == QEvent::KeyPress) {
        if ((static_cast<QKeyEvent*>(e))->key() == Qt::Key_Escape) {
            hide();
            emit escapePressed();
        }
    }
    return QWidget::eventFilter(object, e);
}

QToolButton* FindWidget::setupToolButton(const QString &text, const QString &icon)
{
    TRACE_OBJ
    QToolButton *toolButton = new QToolButton(this);

    toolButton->setText(text);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon(icon));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    return toolButton;
}

QT_END_NAMESPACE
