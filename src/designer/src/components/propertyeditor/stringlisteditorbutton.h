/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef STRINGLISTEDITORBUTTON_H
#define STRINGLISTEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtCore/qstringlist.h>
#include <QtWidgets/qtoolbutton.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT StringListEditorButton: public QToolButton
{
    Q_OBJECT
public:
    explicit StringListEditorButton(const QStringList &stringList, QWidget *parent = nullptr);
    ~StringListEditorButton() override;

    inline QStringList stringList() const
    { return m_stringList; }

signals:
    void stringListChanged(const QStringList &stringList);

public slots:
    void setStringList(const QStringList &stringList);

private slots:
    void showStringListEditor();

private:
    QStringList m_stringList;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // STRINGLISTEDITORBUTTON_H
