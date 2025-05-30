// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTGROUPBOXPROPERTYBROWSER_H
#define QTGROUPBOXPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

QT_BEGIN_NAMESPACE

class QtGroupBoxPropertyBrowserPrivate;

class QtGroupBoxPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:
    QtGroupBoxPropertyBrowser(QWidget *parent = 0);
    ~QtGroupBoxPropertyBrowser();

protected:
    void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) override;
    void itemRemoved(QtBrowserItem *item) override;
    void itemChanged(QtBrowserItem *item) override;

private:
    QScopedPointer<QtGroupBoxPropertyBrowserPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGroupBoxPropertyBrowser)
    Q_DISABLE_COPY_MOVE(QtGroupBoxPropertyBrowser)
};

QT_END_NAMESPACE

#endif
