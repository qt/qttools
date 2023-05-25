// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_membersheet_p.h"
#include "qdesigner_propertysheet_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <abstractintrospection_p.h>

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static QByteArrayList stringListToByteArray(const QStringList &l)
{
    QByteArrayList rc;
    for (const auto &s : l)
        rc += s.toUtf8();
    return rc;
}

// ------------ QDesignerMemberSheetPrivate
class QDesignerMemberSheetPrivate {
public:
    explicit QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent);

    QDesignerFormEditorInterface *m_core;
    const QDesignerMetaObjectInterface *m_meta;

    class Info {
    public:
        QString group;
        bool visible{true};
    };

    Info &ensureInfo(int index);

    QHash<int, Info> m_info;
};

QDesignerMemberSheetPrivate::QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent) :
    m_core(QDesignerPropertySheet::formEditorForObject(sheetParent)),
    m_meta(m_core->introspection()->metaObject(object))
{
}

QDesignerMemberSheetPrivate::Info &QDesignerMemberSheetPrivate::ensureInfo(int index)
{
    auto it = m_info.find(index);
    if (it == m_info.end())
        it = m_info.insert(index, Info());
    return it.value();
}

// --------- QDesignerMemberSheet

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent) :
    QObject(parent),
    d(new QDesignerMemberSheetPrivate(object, parent))
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
    delete d;
}

int QDesignerMemberSheet::count() const
{
    return d->m_meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    return d->m_meta->indexOfMethod(name);
}

QString QDesignerMemberSheet::memberName(int index) const
{
    return d->m_meta->method(index)->tag();
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
    const QString member = d->m_meta->method(index)->signature();

    // Find class whose superclass does not contain the method.
    const QDesignerMetaObjectInterface *meta_obj = d->m_meta;

    for (;;) {
        const QDesignerMetaObjectInterface *tmp = meta_obj->superClass();
        if (tmp == nullptr)
            break;
        if (tmp->indexOfMethod(member) == -1)
            break;
        meta_obj = tmp;
    }
    return meta_obj->className();
}

QString QDesignerMemberSheet::memberGroup(int index) const
{
    return d->m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
    d->ensureInfo(index).group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
    return d->m_meta->method(index)->normalizedSignature();
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    const auto it = d->m_info.constFind(index);
    if (it != d->m_info.constEnd())
        return it.value().visible;

   return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal
           || d->m_meta->method(index)->access() == QDesignerMetaMethodInterface::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    d->ensureInfo(index).visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
    return declaredInClass(index) == "QWidget"_L1 || declaredInClass(index) == "QObject"_L1;
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
    return stringListToByteArray(d->m_meta->method(index)->parameterTypes());
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    return stringListToByteArray(d->m_meta->method(index)->parameterNames());
}

bool QDesignerMemberSheet::signalMatchesSlot(const QString &signal, const QString &slot)
{
    bool result = true;

    do {
        qsizetype signal_idx = signal.indexOf(u'(');
        qsizetype slot_idx = slot.indexOf(u'(');
        if (signal_idx == -1 || slot_idx == -1)
            break;

        ++signal_idx; ++slot_idx;

        if (slot.at(slot_idx) == u')')
            break;

        while (signal_idx < signal.size() && slot_idx < slot.size()) {
            const QChar signal_c = signal.at(signal_idx);
            const QChar slot_c = slot.at(slot_idx);

            if (signal_c == u',' && slot_c == u')')
                break;

            if (signal_c == u')' && slot_c == u')')
                break;

            if (signal_c != slot_c) {
                result = false;
                break;
            }

            ++signal_idx; ++slot_idx;
        }
    } while (false);

    return result;
}

// ------------ QDesignerMemberSheetFactory

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerMemberSheetExtension)) {
        return new QDesignerMemberSheet(object, parent);
    }

    return nullptr;
}

QT_END_NAMESPACE
