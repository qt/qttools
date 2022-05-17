// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTUTILS_P_H
#define SIGNALSLOTUTILS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

enum MemberType { SignalMember, SlotMember };

// member to class name
QMap<QString, QString> getSignals(QDesignerFormEditorInterface *core, QObject *object, bool showAll);
QMap<QString, QString> getMatchingSlots(QDesignerFormEditorInterface *core, QObject *object,
            const QString &signalSignature, bool showAll);

bool memberFunctionListContains(QDesignerFormEditorInterface *core, QObject *object, MemberType type, const QString &signature);

// Members functions listed by class they were inherited from
struct ClassMemberFunctions
{
    ClassMemberFunctions() = default;
    ClassMemberFunctions(const QString &_class_name);

    QString m_className;
    QStringList m_memberList;
};

using ClassesMemberFunctions = QList<ClassMemberFunctions>;

// Return classes and members in reverse class order to
// populate of the combo of the ToolWindow.

ClassesMemberFunctions reverseClassesMemberFunctions(const QString &obj_name, MemberType member_type,
                                                     const QString &peer, QDesignerFormWindowInterface *form);

bool signalMatchesSlot(QDesignerFormEditorInterface *core, const QString &signal, const QString &slot);

QString realObjectName(QDesignerFormEditorInterface *core, QObject *object);

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTUTILS_P_H
