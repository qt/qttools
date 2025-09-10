// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BRUSHPROPERTYMANAGER_H
#define BRUSHPROPERTYMANAGER_H

#include <QtCore/qhash.h>
#include <QtGui/qbrush.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

class QtProperty;
class QtVariantPropertyManager;

class QString;
class QVariant;

namespace qdesigner_internal {

// BrushPropertyManager: A mixin for DesignerPropertyManager that manages brush properties.

class BrushPropertyManager {
public:
    Q_DISABLE_COPY_MOVE(BrushPropertyManager);

    BrushPropertyManager();
    ~BrushPropertyManager();

    void initializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int enumTypeId);
    bool uninitializeProperty(QtProperty *property);

    // Call from slotValueChanged().
    int valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);
    int setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

    bool valueText(const QtProperty *property, QString *text) const;
    bool valueIcon(const QtProperty *property, QIcon *icon) const;
    bool value(const QtProperty *property, QVariant *v) const;

    // Call from  QtPropertyManager's propertyDestroyed signal
    void slotPropertyDestroyed(QtProperty *property);

private:
    static int brushStyleToIndex(Qt::BrushStyle st);
    static Qt::BrushStyle brushStyleIndexToStyle(int brushStyleIndex);
    static QString brushStyleIndexToString(int brushStyleIndex);

    static const QMap<int, QIcon> &brushStyleIcons();

    using PropertyToPropertyMap = QHash<const QtProperty *, QtProperty *>;
    PropertyToPropertyMap m_brushPropertyToStyleSubProperty;
    PropertyToPropertyMap m_brushPropertyToColorSubProperty;
    PropertyToPropertyMap m_brushStyleSubPropertyToProperty;
    PropertyToPropertyMap m_brushColorSubPropertyToProperty;

    QHash<const QtProperty *, QBrush> m_brushValues;
};

}

QT_END_NAMESPACE

#endif // BRUSHPROPERTYMANAGER_H
