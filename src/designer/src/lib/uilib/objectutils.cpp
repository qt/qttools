// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "objectutils_p.h"

#include <QtWidgets/qlayout.h>
#include <QtWidgets/qstackedlayout.h>
#if QT_CONFIG(formlayout)
#  include <QtWidgets/qformlayout.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

template <typename ...T>
struct TypeList {};

using Layouts = TypeList<
        QGridLayout
      , QHBoxLayout
      , QStackedLayout
      , QVBoxLayout
#if QT_CONFIG(formlayout)
      , QFormLayout
#endif
    >;

template <typename ...T>
static QStringList objectNamesImpl(TypeList<T...>)
{
    QStringList result;
    // Fold expression: executes the lambda for each type in the list
    ( [&result] { result.append(QString::fromUtf8(T::staticMetaObject.className())); }() , ... );
    return result;
}

QStringList layoutNames()
{
    return objectNamesImpl(Layouts());
}

template <typename Base, typename T>
static Base *createObject(const QString &className, QWidget *parent)
{
    return className == QLatin1StringView(T::staticMetaObject.className()) ? new T(parent) : nullptr;
}

template <typename Base, typename ...T>
static Base *createObjectImpl(const QString &className, QWidget *parent, TypeList<T...>)
{
    Base *result = nullptr;
    // Left fold over the comma operator
    // The expression inside the parenthesis runs for every type sequentially
    ( (result == nullptr ? (result = createObject<Base, T>(className, parent)) : nullptr), ... );
    return result;
}

QLayout *createLayoutInstance(const QString &className, QWidget *parent)
{
    return createObjectImpl<QLayout>(className, parent, Layouts());
}

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE
