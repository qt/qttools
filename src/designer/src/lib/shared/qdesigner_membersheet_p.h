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

#ifndef QDESIGNER_MEMBERSHEET_H
#define QDESIGNER_MEMBERSHEET_H

#include "shared_global_p.h"

#include <QtDesigner/membersheet.h>
#include <QtDesigner/default_extensionfactory.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class QDesignerMemberSheetPrivate;

class QDESIGNER_SHARED_EXPORT QDesignerMemberSheet: public QObject, public QDesignerMemberSheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerMemberSheetExtension)

public:
    explicit QDesignerMemberSheet(QObject *object, QObject *parent = nullptr);
    ~QDesignerMemberSheet() override;

    int indexOf(const QString &name) const override;

    int count() const override;
    QString memberName(int index) const override;

    QString memberGroup(int index) const override;
    void setMemberGroup(int index, const QString &group) override;

    bool isVisible(int index) const override;
    void setVisible(int index, bool b) override;

    bool isSignal(int index) const override;
    bool isSlot(int index) const override;

    bool inheritedFromWidget(int index) const override;

    static bool signalMatchesSlot(const QString &signal, const QString &slot);

    QString declaredInClass(int index) const override;

    QString signature(int index) const override;
    QList<QByteArray> parameterTypes(int index) const override;
    QList<QByteArray> parameterNames(int index) const override;

private:
    QDesignerMemberSheetPrivate *d;
};

class QDESIGNER_SHARED_EXPORT QDesignerMemberSheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)

public:
    QDesignerMemberSheetFactory(QExtensionManager *parent = nullptr);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;
};

QT_END_NAMESPACE

#endif // QDESIGNER_MEMBERSHEET_H
