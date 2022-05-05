/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt UI Tools library of the Qt Toolkit.
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

#ifndef QUILOADER_P_H
#define QUILOADER_P_H

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

#include <QtUiTools/qtuitoolsglobal.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>

QT_FORWARD_DECLARE_CLASS(QDataStream)

// This file is here for use by the form preview in Linguist. If you change anything
// here or in the code which uses it, remember to adapt Linguist accordingly.

#define PROP_GENERIC_PREFIX "_q_notr_"
#define PROP_TOOLITEMTEXT "_q_toolItemText_notr"
#define PROP_TOOLITEMTOOLTIP "_q_toolItemToolTip_notr"
#define PROP_TABPAGETEXT "_q_tabPageText_notr"
#define PROP_TABPAGETOOLTIP "_q_tabPageToolTip_notr"
#define PROP_TABPAGEWHATSTHIS "_q_tabPageWhatsThis_notr"

QT_BEGIN_NAMESPACE

class Q_UITOOLS_EXPORT QUiTranslatableStringValue
{
public:
    QByteArray value() const { return m_value; }
    void setValue(const QByteArray &value) { m_value = value; }
    QByteArray qualifier() const { return m_qualifier; }
    void setQualifier(const QByteArray &qualifier) { m_qualifier = qualifier; }

    QString translate(const QByteArray &className, bool idBased) const;

private:
    QByteArray m_value;
    QByteArray m_qualifier; // Comment or ID for id-based tr().
};

#ifndef QT_NO_DATASTREAM
Q_UITOOLS_EXPORT QDataStream &operator<<(QDataStream &out, const QUiTranslatableStringValue &s);
Q_UITOOLS_EXPORT QDataStream &operator>>(QDataStream &in, QUiTranslatableStringValue &s);
#endif // QT_NO_DATASTREAM

struct QUiItemRolePair {
    int realRole;
    int shadowRole;
};

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

extern const Q_UITOOLS_EXPORT QUiItemRolePair qUiItemRoles[];

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QUiTranslatableStringValue)


#endif // QUILOADER_P_H
