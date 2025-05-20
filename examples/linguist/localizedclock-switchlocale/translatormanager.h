// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include <QQmlEngine>
#include <QTranslator>

class TranslatorManager : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
public:
    explicit TranslatorManager(QQmlEngine &engine);

    static TranslatorManager *create(QQmlEngine *engine, QJSEngine *);
    Q_INVOKABLE void switchLanguage(const QString &lang);

private:
    QTranslator m_translator;
    QQmlEngine &m_engine;
};
#endif // TRANSLATORMANAGER_H
