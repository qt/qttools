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

#ifndef ABSTRACTRESOURCEBROWSER_H
#define ABSTRACTRESOURCEBROWSER_H

#include <QtDesigner/sdk_global.h>

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QWidget; // FIXME: fool syncqt

class QDESIGNER_SDK_EXPORT QDesignerResourceBrowserInterface : public QWidget
{
    Q_OBJECT
public:
    explicit QDesignerResourceBrowserInterface(QWidget *parent = nullptr);
    virtual ~QDesignerResourceBrowserInterface();

    virtual void setCurrentPath(const QString &filePath) = 0;
    virtual QString currentPath() const = 0;

Q_SIGNALS:
    void currentPathChanged(const QString &filePath);
    void pathActivated(const QString &filePath);
};

QT_END_NAMESPACE

#endif // ABSTRACTFORMEDITOR_H

