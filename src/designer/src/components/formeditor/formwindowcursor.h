/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include "formeditor_global.h"
#include "formwindow.h"
#include <QtDesigner/QDesignerFormWindowCursorInterface>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowCursor: public QObject, public QDesignerFormWindowCursorInterface
{
    Q_OBJECT
public:
    explicit FormWindowCursor(FormWindow *fw, QObject *parent = 0);
    virtual ~FormWindowCursor();

    QDesignerFormWindowInterface *formWindow() const Q_DECL_OVERRIDE;

    bool movePosition(MoveOperation op, MoveMode mode) Q_DECL_OVERRIDE;

    int position() const Q_DECL_OVERRIDE;
    void setPosition(int pos, MoveMode mode) Q_DECL_OVERRIDE;

    QWidget *current() const Q_DECL_OVERRIDE;

    int widgetCount() const Q_DECL_OVERRIDE;
    QWidget *widget(int index) const Q_DECL_OVERRIDE;

    bool hasSelection() const Q_DECL_OVERRIDE;
    int selectedWidgetCount() const Q_DECL_OVERRIDE;
    QWidget *selectedWidget(int index) const Q_DECL_OVERRIDE;

    void setProperty(const QString &name, const QVariant &value) Q_DECL_OVERRIDE;
    void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) Q_DECL_OVERRIDE;
    void resetWidgetProperty(QWidget *widget, const QString &name) Q_DECL_OVERRIDE;

public slots:
    void update();

private:
    FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOWCURSOR_H
