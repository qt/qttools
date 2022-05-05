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

#ifndef DESIGNERINTROSPECTION
#define DESIGNERINTROSPECTION

#include "shared_global_p.h"
#include <abstractintrospection_p.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

struct QMetaObject;
class QWidget;

namespace qdesigner_internal {
    // Qt C++ introspection with helpers to find core and meta object for an object
    class QDESIGNER_SHARED_EXPORT QDesignerIntrospection : public QDesignerIntrospectionInterface {
    public:
        QDesignerIntrospection();
        ~QDesignerIntrospection() override;

        const QDesignerMetaObjectInterface* metaObject(const QObject *object) const override;

        const QDesignerMetaObjectInterface* metaObjectForQMetaObject(const QMetaObject *metaObject) const;
    private:
        using MetaObjectMap = QMap<const QMetaObject*, QDesignerMetaObjectInterface*>;
        mutable MetaObjectMap m_metaObjectMap;

    };
}

QT_END_NAMESPACE

#endif // DESIGNERINTROSPECTION
