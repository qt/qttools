/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "qtgradientmanager.h"
#include <QtGui/QPixmap>
#include <QtCore/QMetaEnum>

QT_BEGIN_NAMESPACE

QtGradientManager::QtGradientManager(QObject *parent)
    : QObject(parent)
{
}

QMap<QString, QGradient> QtGradientManager::gradients() const
{
    return m_idToGradient;
}

QString QtGradientManager::uniqueId(const QString &id) const
{
    if (!m_idToGradient.contains(id))
        return id;

    QString base = id;
    while (base.count() > 0 && base.at(base.count() - 1).isDigit())
        base = base.left(base.count() - 1);
    QString newId = base;
    int counter = 0;
    while (m_idToGradient.contains(newId)) {
        ++counter;
        newId = base + QString::number(counter);
    }
    return newId;
}

QString QtGradientManager::addGradient(const QString &id, const QGradient &gradient)
{
    QString newId = uniqueId(id);

    m_idToGradient[newId] = gradient;

    emit gradientAdded(newId, gradient);

    return newId;
}

void QtGradientManager::removeGradient(const QString &id)
{
    if (!m_idToGradient.contains(id))
        return;

    emit gradientRemoved(id);

    m_idToGradient.remove(id);
}

void QtGradientManager::renameGradient(const QString &id, const QString &newId)
{
    if (!m_idToGradient.contains(id))
        return;

    if (newId == id)
        return;

    QString changedId = uniqueId(newId);
    QGradient gradient = m_idToGradient.value(id);

    emit gradientRenamed(id, changedId);

    m_idToGradient.remove(id);
    m_idToGradient[changedId] = gradient;
}

void QtGradientManager::changeGradient(const QString &id, const QGradient &newGradient)
{
    if (!m_idToGradient.contains(id))
        return;

    if (m_idToGradient.value(id) == newGradient)
        return;

    emit gradientChanged(id, newGradient);

    m_idToGradient[id] = newGradient;
}

void QtGradientManager::clear()
{
    const QMap<QString, QGradient> grads = gradients();
    for (auto it = grads.cbegin(), end = grads.cend(); it != end; ++it)
        removeGradient(it.key());
}

QT_END_NAMESPACE
