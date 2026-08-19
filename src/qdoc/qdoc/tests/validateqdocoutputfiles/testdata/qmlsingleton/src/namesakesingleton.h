// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef NAMESAKESINGLETON_H
#define NAMESAKESINGLETON_H

#include <QtCore/QObject>
#include <QtQml/qqml.h>

// Backs the QML singleton type "Namesake". Its *class* name deliberately equals
// the name of an unrelated, non-singleton QML type (see namesaketest.qdoc). This
// mirrors QtQml, where the QML type "Qt" is a singleton backed by a C++ class
// named QtObject, while a separate QML type is also named QtObject and is backed
// by plain QObject.
class NamesakeObject : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Namesake)
    QML_SINGLETON

public:
    explicit NamesakeObject(QObject *parent = nullptr);

    Q_INVOKABLE QString getNamesakeMessage() const;
};

// The native type of the QML type "NamesakeObject". Carries no QML_SINGLETON.
class PlainObject : public QObject
{
    Q_OBJECT

public:
    explicit PlainObject(QObject *parent = nullptr);

    Q_INVOKABLE QString getPlainMessage() const;
};

#endif // NAMESAKESINGLETON_H
