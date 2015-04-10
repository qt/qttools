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

#ifndef ACTIVEQT_EXTRAINFO_H
#define ACTIVEQT_EXTRAINFO_H

#include <QtDesigner/QDesignerExtraInfoExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionFactory>

#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;

class QAxWidgetExtraInfo: public QObject, public QDesignerExtraInfoExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerExtraInfoExtension)
public:
    QAxWidgetExtraInfo(QDesignerAxWidget *widget, QDesignerFormEditorInterface *core, QObject *parent);

    QWidget *widget() const Q_DECL_OVERRIDE;
    QDesignerFormEditorInterface *core() const Q_DECL_OVERRIDE;

    bool saveUiExtraInfo(DomUI *ui) Q_DECL_OVERRIDE;
    bool loadUiExtraInfo(DomUI *ui) Q_DECL_OVERRIDE;

    bool saveWidgetExtraInfo(DomWidget *ui_widget) Q_DECL_OVERRIDE;
    bool loadWidgetExtraInfo(DomWidget *ui_widget) Q_DECL_OVERRIDE;

private:
    QPointer<QDesignerAxWidget> m_widget;
    QPointer<QDesignerFormEditorInterface> m_core;
};

class QAxWidgetExtraInfoFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    explicit QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const Q_DECL_OVERRIDE;

private:
    QDesignerFormEditorInterface *m_core;
};

QT_END_NAMESPACE

#endif // ACTIVEQT_EXTRAINFO_H
