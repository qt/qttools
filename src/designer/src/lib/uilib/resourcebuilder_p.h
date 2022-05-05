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

#ifndef RESOURCEBUILDER_H
#define RESOURCEBUILDER_H

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

#include "uilib_global.h"
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QDir;
class QVariant;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class DomProperty;
class DomResourceIcon;

class QDESIGNER_UILIB_EXPORT QResourceBuilder
{
public:
    enum IconStateFlags {
        NormalOff = 0x1, NormalOn = 0x2, DisabledOff = 0x4, DisabledOn = 0x8,
        ActiveOff = 0x10, ActiveOn = 0x20, SelectedOff = 0x40, SelectedOn = 0x80
    };

    QResourceBuilder();
    virtual ~QResourceBuilder();

    virtual QVariant loadResource(const QDir &workingDirectory, const DomProperty *property) const;

    virtual QVariant toNativeValue(const QVariant &value) const;

    virtual DomProperty *saveResource(const QDir &workingDirectory, const QVariant &value) const;

    virtual bool isResourceProperty(const DomProperty *p) const;

    virtual bool isResourceType(const QVariant &value) const;

    static int iconStateFlags(const DomResourceIcon *resIcon);
};


#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // RESOURCEBUILDER_H
