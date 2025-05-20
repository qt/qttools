// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "translatormanager.h"

#include <QGuiApplication>

using namespace Qt::Literals::StringLiterals;

TranslatorManager::TranslatorManager(QQmlEngine &engine) : m_engine(engine) { }

TranslatorManager *TranslatorManager::create(QQmlEngine *engine, QJSEngine *)
{
    return new TranslatorManager(*engine);
}

void TranslatorManager::switchLanguage(const QString &lang)
{
    QLocale locale(lang);
    QLocale::setDefault(locale);
    qApp->removeTranslator(&m_translator); // not necessary from Qt 6.10
    if (m_translator.load(locale, "clock"_L1, "_"_L1, ":/i18n/"_L1)
        && qApp->installTranslator(&m_translator)) {
        m_engine.retranslate();
    } else {
        qWarning("Could not load translation to %s!", qPrintable(locale.name()));
    }
}
