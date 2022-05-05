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
// of the Qt tools.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef GRIDPANEL_H
#define GRIDPANEL_H

#include "shared_global_p.h"

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class Grid;

namespace Ui {
    class GridPanel;
}

class  QDESIGNER_SHARED_EXPORT GridPanel : public QWidget
{
    Q_OBJECT
public:
    GridPanel(QWidget *parent = nullptr);
    ~GridPanel();

    void setTitle(const QString &title);

    void setGrid(const Grid &g);
    Grid grid() const;

    void setCheckable (bool c);
    bool isCheckable () const;

    bool isChecked () const;
    void setChecked(bool c);

    void setResetButtonVisible(bool v);

private slots:
    void reset();

private:
    Ui::GridPanel *m_ui;
};

} // qdesigner_internal

QT_END_NAMESPACE

#endif // GRIDPANEL_H
