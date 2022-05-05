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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROPERTYLINEEDIT_H
#define PROPERTYLINEEDIT_H

#include "shared_global_p.h"

#include <QtWidgets/qlineedit.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    // A line edit with a special context menu allowing for adding (escaped) new  lines
    class PropertyLineEdit : public QLineEdit {
        Q_OBJECT
    public:
        explicit PropertyLineEdit(QWidget *parent);
        void setWantNewLine(bool nl) {  m_wantNewLine = nl; }
        bool wantNewLine() const { return m_wantNewLine; }

        bool event(QEvent *e) override;
    protected:
        void contextMenuEvent (QContextMenuEvent *event ) override;
    private slots:
        void insertNewLine();
    private:
        void insertText(const QString &);
        bool m_wantNewLine;
    };
}

QT_END_NAMESPACE

#endif // PROPERTYLINEEDIT_H
