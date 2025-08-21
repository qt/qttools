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
    QMap<Validator::ErrorType, QString> errors;

    tor.languageAndTerritory(tor.sourceLanguageCode(), &sourceLang, nullptr);
    tor.languageAndTerritory(tor.languageCode(), &targetLang, &targetTerritory);

    if (checks.placeMarker && !getCountNeed(targetLang, targetTerritory, countRefNeeds, nullptr)) {
        printErr("Could not get numerus info");
        ok = false;
    }

    for (const TranslatorMessage &msg : tor.messages()) {
        if (msg.isTranslated() && (checkFinished || msg.type() != TranslatorMessage::Finished)) {
            Validator validator = Validator::fromSource(msg.sourceText(), checks, sourceLang, {});
            errors.insert(validator.validate(msg.translations(), msg, targetLang, countRefNeeds));
        }
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

    for (const QString &trs : errors)
        stream << "Validation error for translation '%1'\n"_L1.arg(trs);

    printOut("Finished batch checks.");

    if (!errors.empty())
        ok = false;

    return ok ? 0 : 1;
}
