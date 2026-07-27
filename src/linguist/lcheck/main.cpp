// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QString>

#include "translator.h"
#include "validator.h"

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

static void printErr(const QString &out)
{
    QTextStream stream(stderr);
    stream << out;
}

static void printOut(const QString &out)
{
    QTextStream stream(stdout);
    stream << out;
}

// These descriptions intentionally mirror the (translated) warning texts in
// WarningModel::addWarning() in the Linguist GUI. The GUI needs translatable
// strings while this console tool does not, so the two sets are kept separate.
static QString checkDescription(Validator::ErrorType type)
{
    switch (type) {
    case Validator::SuperfluousAccelerator:
        return u"Accelerator possibly superfluous in translation"_s;
    case Validator::MissingAccelerator:
        return u"Accelerator possibly missing in translation"_s;
    case Validator::SurroundingWhitespaceDiffers:
        return u"Translation does not have the same surrounding whitespace as the source text"_s;
    case Validator::PunctuationDiffers:
        return u"Translation does not end with the same punctuation as the source text"_s;
    case Validator::PlaceMarkersDiffer:
        return u"Translation does not refer to the same place markers as the source text"_s;
    case Validator::NumerusMarkerMissing:
        return u"Translation does not contain the necessary %n/%Ln place marker"_s;
    case Validator::IgnoredPhrasebook:
        return u"A phrase book suggestion was ignored"_s;
    }
    return u"Unknown error"_s;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(u"lcheck"_s);
    QCoreApplication::setApplicationVersion(QLatin1StringView(QT_VERSION_STR));

    QCommandLineParser parser;
    parser.setApplicationDescription(
            u"lcheck is part of Qt's Linguist tool chain. It can be used as a\n"
            "stand-alone tool to perform batch checks on the translations of\n"
            "TS files. By default, lcheck performs the following checks and\n"
            "fails if at least one check fails:\n"
            "    Validity check of accelerators:\n"
            "        Whether the number of ampersands in the source\n"
            "        and translation text is the same.\n"
            "    Validity check of surrounding whitespaces:\n"
            "        Whether the source and translation texts have the\n"
            "        same surrounding whitespaces.\n"
            "    Validity check of ending punctuation:\n"
            "        Whether the source and translation texts have the\n"
            "        same ending punctuation.\n"
            "    Validity check of place markers:\n"
            "        Whether %1, %2, ... are used consistently in the\n"
            "        source text and translation text.\n"
            "To get more details regarding the checks refer to Qt Linguist help.\n"
            "Each check can be disabled using the arguments as explained below."_s);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption noAcceleratorOption(u"no-accelerator"_s, u"Disable the accelerator check"_s);
    parser.addOption(noAcceleratorOption);

    QCommandLineOption noPunctuationOption(u"no-punctuation"_s, u"Disable the punctuation check"_s);
    parser.addOption(noPunctuationOption);

    QCommandLineOption noPlaceMarkerOption(u"no-place-marker"_s,
                                           u"Disable the place marker check"_s);
    parser.addOption(noPlaceMarkerOption);

    QCommandLineOption noWhitespacesOption(u"no-whitespaces"_s,
                                           u"Disable the check for surrounding white spaces"_s);
    parser.addOption(noWhitespacesOption);

    QCommandLineOption checkFinishedOption(
            u"check-finished"_s,
            u"Enable check for translations marked as finished.\n"
            "By default, the finished translations are not checked."_s);
    parser.addOption(checkFinishedOption);

    QCommandLineOption outputOption(QStringList() << u"o"_s << u"output"_s,
                                    u"The output file to generate the report to. If\n"
                                    "nothing is specified, the report is written to\n"
                                    "the standard error stream."_s,
                                    u"file"_s);
    parser.addOption(outputOption);

    parser.addPositionalArgument(u"ts-file"_s, u"TS file to check"_s);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    const QString tsFile = args.first();

    Validator::Checks checks;
    checks.accelerator = !parser.isSet(noAcceleratorOption);
    checks.punctuation = !parser.isSet(noPunctuationOption);
    checks.placeMarker = !parser.isSet(noPlaceMarkerOption);
    checks.surroundingWhiteSpace = !parser.isSet(noWhitespacesOption);

    bool checkFinished = parser.isSet(checkFinishedOption);
    std::optional<QString> output;
    if (parser.isSet(outputOption))
        output = parser.value(outputOption);

    Translator tor;
    ConversionData cd;
    bool ok = tor.load(tsFile, cd, "auto"_L1);
    if (!ok) {
        printErr("lcheck error: %1"_L1.arg(cd.error()));
        return 1;
    }

    if (!cd.errors().isEmpty())
        ok = false;

    QLocale::Language sourceLang;
    QLocale::Language targetLang;
    QLocale::Territory targetTerritory;
    QList<bool> countRefNeeds;

    tor.languageAndTerritory(tor.sourceLanguageCode(), &sourceLang, nullptr);
    tor.languageAndTerritory(tor.languageCode(), &targetLang, &targetTerritory);

    if (checks.placeMarker && !getCountNeed(targetLang, targetTerritory, countRefNeeds, nullptr)) {
        printErr("Could not get numerus info");
        ok = false;
    }

    QTextStream stream(stderr);
    QFile f;
    if (output) {
        f.setFileName(*output);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
            stream.setDevice(&f);
        else {
            printErr("Could not open the output file %1 for writing."_L1.arg(*output));
            return 1;
        }
    }

    int errorCount = 0;
    for (const TranslatorMessage &msg : tor.messages()) {
        if (!msg.isTranslated() || (!checkFinished && msg.type() == TranslatorMessage::Finished))
            continue;

        Validator validator = Validator::fromSource(msg.sourceText(), checks, sourceLang, {});
        const QList<Validator::Error> errors =
                validator.validate(msg.translations(), msg, targetLang, countRefNeeds);
        if (errors.isEmpty())
            continue;

        QString location = msg.fileName();
        if (!location.isEmpty()) {
            if (msg.lineNumber() >= 0)
                location += ":"_L1 + QString::number(msg.lineNumber());
            location += ": "_L1;
        }

        for (const Validator::Error &error : errors) {
            stream << location << checkDescription(error.type) << u'\n';
            if (!msg.context().isEmpty())
                stream << "    Context:     "_L1 << msg.context() << u'\n';
            stream << "    Source:      "_L1 << msg.sourceText() << u'\n';
            stream << "    Translation: "_L1 << error.message << u'\n';
            ++errorCount;
        }
    }

    if (errorCount > 0)
        ok = false;

    printOut(u"Finished batch checks. Found %1 validation error(s).\n"_s.arg(errorCount));

    return ok ? 0 : 1;
}
