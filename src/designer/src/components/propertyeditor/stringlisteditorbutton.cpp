// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "stringlisteditorbutton.h"
#include "stringlisteditor.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

StringListEditorButton::StringListEditorButton(
    const QStringList &stringList, QWidget *parent)
    : QToolButton(parent), m_stringList(stringList)
{
    setFocusPolicy(Qt::NoFocus);
    setText(tr("Change String List"));
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect(this, &QAbstractButton::clicked, this, &StringListEditorButton::showStringListEditor);
}

StringListEditorButton::~StringListEditorButton() = default;

void StringListEditorButton::setStringList(const QStringList &stringList)
{
    m_stringList = stringList;
}

void StringListEditorButton::showStringListEditor()
{
    int result;
    QStringList lst = StringListEditor::getStringList(nullptr, m_stringList, &result);
    if (result == QDialog::Accepted) {
        m_stringList = lst;
        emit stringListChanged(m_stringList);
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
