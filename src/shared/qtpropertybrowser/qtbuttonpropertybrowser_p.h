// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef QTBUTTONPROPERTYBROWSER_H
#define QTBUTTONPROPERTYBROWSER_H

#include "qtpropertybrowser_p.h"

QT_BEGIN_NAMESPACE

class QtButtonPropertyBrowserPrivate;

class QtButtonPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:
    QtButtonPropertyBrowser(QWidget *parent = nullptr);
    ~QtButtonPropertyBrowser() override;

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
};

QT_END_NAMESPACE

#endif
