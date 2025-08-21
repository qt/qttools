// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "validator.h"
#include "translator.h"
#if defined(QT_FEATURE_widgets)
#  include "phrase.h"
#endif

QT_USE_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace {

static QString leadingWhitespace(const QString &str)
{
    int i = 0;
    for (; i < str.size(); i++) {
        if (!str[i].isSpace()) {
            break;
        }
    }
    return str.left(i);
}

static QString trailingWhitespace(const QString &str)
{
    int i = str.size();
    while (--i >= 0) {
        if (!str[i].isSpace()) {
            break;
        }
    }
    return str.mid(i + 1);
}

static Validator::Ending ending(QString str, QLocale::Language lang)
{
    str = str.simplified();
    if (str.isEmpty())
        return Validator::End_None;

    switch (str.at(str.size() - 1).unicode()) {
    case 0x002e: // full stop
        if (str.endsWith("..."_L1))
            return Validator::End_Ellipsis;
        else
            return Validator::End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
        return Validator::End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
    case 0xff01: // full width exclamation mark
    case 0xff1f: // full width question mark
        return Validator::End_Interrobang;
    case 0x003b: // greek 'compatibility' questionmark
        return lang == QLocale::Greek ? Validator::End_Interrobang : Validator::End_None;
    case 0x003a: // colon
    case 0xff1a: // full width colon
        return Validator::End_Colon;
    case 0x2026: // horizontal ellipsis
        return Validator::End_Ellipsis;
    default:
        return Validator::End_None;
    }
}

static bool haveMnemonic(const QString &str)
{
    for (const ushort *p = (ushort *)str.constData();;) { // Assume null-termination
        ushort c = *p++;
        if (!c)
            break;
        if (c == '&') {
            c = *p++;
            if (!c)
                return false;
            // Matches QKeySequence::mnemonic(), except for
            // '&#' - most likely the start of an NCR
            // '& ' - too many false positives
            if (c != '&' && c != ' ' && c != '#' && QChar(c).isPrint()) {
                const ushort *pp = p;
                for (; *p < 256 && isalpha(*p); p++)
                    ;
                if (pp == p || *p != ';')
                    return true;
                // This looks like a HTML &entity;, so ignore it. As a HTML string
                // won't contain accels anyway, we can stop scanning here.
                break;
            }
        }
    }
    return false;
}

static QHash<int, int> countPlaceMarkers(const QString &str)
{
    QHash<int, int> counts;
    const QChar *c = str.unicode();
    const QChar *cend = c + str.size();
    while (c < cend) {
        if (c->unicode() == '%') {
            const QChar *escape_start = ++c;
            while (c->isDigit())
                ++c;
            const QChar *escape_end = c;
            bool ok = true;
            int markerIndex =
                    QString::fromRawData(escape_start, escape_end - escape_start).toInt(&ok);
            if (ok)
                counts[markerIndex]++;
        } else {
            ++c;
        }
    }
    return counts;
}
} // namespace

Validator Validator::fromSource(const QString &source, const Checks &checks,
                                const QLocale::Language &locale,
                                const QHash<QString, QList<Phrase *>> &phrases)
{
    Q_UNUSED(phrases)
    Validator v;
    if (checks.accelerator)
        v.m_haveMnemonic.emplace(haveMnemonic(source));
    if (checks.punctuation)
        v.m_ending.emplace(ending(source, locale));
    if (checks.placeMarker)
        v.m_placeMarkerCounts.emplace(countPlaceMarkers(source));
    if (checks.surroundingWhiteSpace) {
        v.m_leadingWhiteSpace.emplace(leadingWhitespace(source));
        v.m_trailingWhiteSpace.emplace(trailingWhitespace(source));
    }
#if defined(QT_FEATURE_widgets)
    if (checks.phraseMatch) {
        v.m_matchingPhraseTargets.emplace();
        QString fsource = friendlyString(source);
        QStringList lookupWords = fsource.split(QLatin1Char(' '));

        for (const QString &s : std::as_const(lookupWords))
            if (auto wordPhrases = phrases.find(s); wordPhrases != phrases.constEnd())
                for (const Phrase *p : *wordPhrases)
                    if (fsource == friendlyString(p->source()))
                        v.m_matchingPhraseTargets.value()[s].append(friendlyString(p->target()));
    }
#endif
    return v;
}

QMap<Validator::ErrorType, QString> Validator::validate(QStringList translations,
                                                        const TranslatorMessage &msg,
                                                        const QLocale::Language &locale,
                                                        QList<bool> countRefNeeds)
{
    int i = 0;
    QMap<ErrorType, QString> errors;
    for (QStringView translation : std::as_const(translations)) {
        while (!translation.isEmpty()) {
            auto sep = translation.indexOf(Translator::BinaryVariantSeparator);
            if (sep < 0)
                sep = translation.size();
            const QString trans = translation.first(sep).toString();

            const bool needsRef = msg.isPlural() && countRefNeeds.at(i);
            errors.insert(validateTranslation(trans, locale, needsRef));
            translation.slice(std::min(sep + 1, translation.size()));
        }
        i++;
    }
    return errors;
}

QMap<Validator::ErrorType, QString> Validator::validateTranslation(const QString &translation,
                                                                   const QLocale::Language &locale,
                                                                   bool needsRef)
{
    QMap<ErrorType, QString> errors;
    if (m_haveMnemonic && *m_haveMnemonic != haveMnemonic(translation))
        errors.insert(*m_haveMnemonic ? MissingAccelerator : SuperfluousAccelerator, translation);
    if (m_placeMarkerCounts) {
        if (*m_placeMarkerCounts != countPlaceMarkers(translation))
            errors.insert(PlaceMarkersDiffer, translation);
        if (needsRef && !translation.contains(QLatin1String("%n"))
            && !translation.contains(QLatin1String("%Ln")))
            errors.insert(NumerusMarkerMissing, translation);
    }
    if (m_ending && *m_ending != ending(translation, locale))
        errors.insert(PunctuationDiffers, translation);

    if (m_leadingWhiteSpace
        && (*m_leadingWhiteSpace != leadingWhitespace(translation)
            || *m_trailingWhiteSpace != trailingWhitespace(translation)))
        errors.insert(SurroundingWhitespaceDiffers, translation);
#if defined(QT_FEATURE_widgets)
    if (m_matchingPhraseTargets) {
        const QString ftranslation = friendlyString(translation);
        for (auto itr = m_matchingPhraseTargets->cbegin(); itr != m_matchingPhraseTargets->cend();
             itr++) {
            bool found = false;
            for (const QString &target : itr.value()) {
                if (ftranslation.indexOf(target) >= 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
                errors.insert(IgnoredPhrasebook, itr.key());
        }
    }
#endif
    return errors;
}
