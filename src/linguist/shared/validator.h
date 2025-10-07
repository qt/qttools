// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QString>
#include <QLocale>

QT_BEGIN_NAMESPACE

class Phrase;
class TranslatorMessage;

namespace Ui {
class MainWindow;
}

struct Validator
{

    struct Checks
    {
        bool accelerator = true;
        bool punctuation = true;
        bool placeMarker = true;
        bool surroundingWhiteSpace = true;
#ifndef LINGUIST_CONSOLE_APPLICATION
        bool phraseMatch = true;
#endif
    };

    enum ErrorType {
        SuperfluousAccelerator,
        MissingAccelerator,
        SurroundingWhitespaceDiffers,
        PunctuationDiffers,
        IgnoredPhrasebook,
        PlaceMarkersDiffer,
        NumerusMarkerMissing
    };

    enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon, End_Ellipsis };

    static Validator fromSource(const QString &source, const Checks &checks,
                                const QLocale::Language &locale,
                                const QHash<QString, QList<Phrase *>> &phrases);

    QMap<ErrorType, QString> validate(QStringList translations, const TranslatorMessage &msg,
                                      const QLocale::Language &locale, QList<bool> countRefNeeds);

private:
    Validator() = default;
    QMap<ErrorType, QString> validateTranslation(const QString &translation,
                                                 const QLocale::Language &locale, bool needsRef);
    std::optional<bool> m_haveMnemonic;
    std::optional<QString> m_leadingWhiteSpace;
    std::optional<QString> m_trailingWhiteSpace;
    std::optional<Ending> m_ending;
    std::optional<QHash<int, int>> m_placeMarkerCounts;
#ifndef LINGUIST_CONSOLE_APPLICATION
    std::optional<QHash<QString, QStringList>> m_matchingPhraseTargets;
#endif
};

QT_END_NAMESPACE

#endif // VALIDATOR_H
