/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QTBUTTONPROPERTYBROWSER_H
#define QTBUTTONPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

QT_BEGIN_NAMESPACE

class QtButtonPropertyBrowserPrivate;

class QtButtonPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:

    QtButtonPropertyBrowser(QWidget *parent = 0);
    ~QtButtonPropertyBrowser();

    void setExpanded(QtBrowserItem *item, bool expanded);
    bool isExpanded(QtBrowserItem *item) const;

Q_SIGNALS:

    void collapsed(QtBrowserItem *item);
    void expanded(QtBrowserItem *item);

protected:
    void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) override;
    void itemRemoved(QtBrowserItem *item) override;
    void itemChanged(QtBrowserItem *item) override;

private:

    QScopedPointer<QtButtonPropertyBrowserPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtButtonPropertyBrowser)
    Q_DISABLE_COPY_MOVE(QtButtonPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotUpdate())
    Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed())
    Q_PRIVATE_SLOT(d_func(), void slotToggled(bool))

};

QT_END_NAMESPACE

#endif
