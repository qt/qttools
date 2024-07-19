// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef METATRANSLATOR_H
#define METATRANSLATOR_H

#include "translatormessage.h"
#include "fmt.h"

#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QLocale>
#include <QMultiHash>
#include <QRegularExpression>
#include <QString>
#include <QSet>
#include <QVector>

QT_BEGIN_NAMESPACE

class QIODevice;

// A struct of "interesting" data passed to and from the load and save routines
class ConversionData
{
public:
    ConversionData() :
        m_verbose(false),
        m_ignoreUnfinished(false),
        m_sortContexts(false),
        m_noUiLines(false),
        m_idBased(false),
        m_saveMode(SaveEverything)
    {}

    // tag manipulation
    const QStringList &dropTags() const { return m_dropTags; }
    QStringList &dropTags() { return m_dropTags; }
    const QDir &targetDir() const { return m_targetDir; }
    bool isVerbose() const { return m_verbose; }
    bool ignoreUnfinished() const { return m_ignoreUnfinished; }
    bool sortContexts() const { return m_sortContexts; }

    void appendError(const QString &error) { m_errors.append(error); }
    QString error() const { return m_errors.isEmpty() ? QString() : m_errors.join(QLatin1Char('\n')) + QLatin1Char('\n'); }
    QStringList errors() const { return  m_errors; }
    void clearErrors() { m_errors.clear(); }

public:
    QString m_defaultContext;
    bool m_sourceIsUtf16; // CPP & JAVA specific
    QString m_unTrPrefix; // QM specific
    QString m_sourceFileName;
    QString m_targetFileName;
    QString m_compilationDatabaseDir;
    QVector<QRegularExpression> m_excludes;
    QDir m_sourceDir;
    QDir m_targetDir; // FIXME: TS specific
    QSet<QString> m_projectRoots;
    QMultiHash<QString, QString> m_allCSources;
    QStringList m_includePath;
    QStringList m_dropTags;  // tags to be dropped
    QStringList m_errors;
    bool m_verbose;
    bool m_ignoreUnfinished;
    bool m_sortContexts;
    bool m_noUiLines;
    bool m_idBased;
    TranslatorSaveMode m_saveMode;
    QStringList m_rootDirs;
};

class TMMKey {
public:
    TMMKey(const TranslatorMessage &msg)
        { context = msg.context(); source = msg.sourceText(); comment = msg.comment(); }
    bool operator==(const TMMKey &o) const
        { return context == o.context && source == o.source && comment == o.comment; }
    QString context, source, comment;
};
Q_DECLARE_TYPEINFO(TMMKey, Q_RELOCATABLE_TYPE);
inline size_t qHash(const TMMKey &key)
{
    return qHash(key.context) ^ qHash(key.source) ^ qHash(key.comment);
}

class Translator
{
public:
    Translator();

    bool load(const QString &filename, ConversionData &err, const QString &format /* = "auto" */);
    bool save(const QString &filename, ConversionData &err, const QString &format /* = "auto" */) const;

    int find(const TranslatorMessage &msg) const;
    int find(const QString &context,
        const QString &comment, const TranslatorMessage::References &refs) const;

    int find(const QString &context) const;

    void replaceSorted(const TranslatorMessage &msg);
    void extend(const TranslatorMessage &msg, ConversionData &cd); // Only for single-location messages
    void append(const TranslatorMessage &msg);
    void appendSorted(const TranslatorMessage &msg);

    void stripObsoleteMessages();
    void stripFinishedMessages();
    void stripUntranslatedMessages();
    void stripEmptyContexts();
    void stripNonPluralForms();
    void stripIdenticalSourceTranslations();
    void dropTranslations();
    void dropUiLines();
    void makeFileNamesAbsolute(const QDir &originalPath);
    bool translationsExist() const;

    using DuplicateEntries = QHash<int, QVector<int>>;
    struct Duplicates
    {
        DuplicateEntries byId, byContents;
    };
    Duplicates resolveDuplicates();
    void reportDuplicates(const Duplicates &dupes, const QString &fileName, bool verbose);
    void reportDuplicatesLines(const TranslatorMessage &msg,
                               const DuplicateEntries::value_type &dups) const;

    QString languageCode() const { return m_language; }
    QString sourceLanguageCode() const { return m_sourceLanguage; }

    enum LocationsType { DefaultLocations, NoLocations, RelativeLocations, AbsoluteLocations };
    void setLocationsType(LocationsType lt) { m_locationsType = lt; }
    LocationsType locationsType() const { return m_locationsType; }

    static QString makeLanguageCode(QLocale::Language language, QLocale::Territory territory);
    static void languageAndTerritory(QStringView languageCode, QLocale::Language *langPtr,
                                     QLocale::Territory *territoryPtr);
    void setLanguageCode(const QString &languageCode) { m_language = languageCode; }
    void setSourceLanguageCode(const QString &languageCode) { m_sourceLanguage = languageCode; }
    static QString guessLanguageCodeFromFileName(const QString &fileName);
    const QList<TranslatorMessage> &messages() const;
    static QStringList normalizedTranslations(const TranslatorMessage &m, int numPlurals);
    void normalizeTranslations(ConversionData &cd);
    QStringList normalizedTranslations(const TranslatorMessage &m, ConversionData &cd, bool *ok) const;

    int messageCount() const { return m_messages.size(); }
    TranslatorMessage &message(int i) { return m_messages[i]; }
    const TranslatorMessage &message(int i) const { return m_messages.at(i); }
    const TranslatorMessage &constMessage(int i) const { return m_messages.at(i); }
    void dump() const;

    void setDependencies(const QStringList &dependencies) { m_dependencies = dependencies; }
    QStringList dependencies() const { return m_dependencies; }

    // additional file format specific data
    // note: use '<fileformat>:' as prefix for file format specific members,
    // e.g. "po-flags", "po-msgid_plural"
    typedef TranslatorMessage::ExtraData ExtraData;
    QString extra(const QString &ba) const;
    void setExtra(const QString &ba, const QString &var);
    bool hasExtra(const QString &ba) const;
    const ExtraData &extras() const { return m_extra; }
    void setExtras(const ExtraData &extras) { m_extra = extras; }

    // registration of file formats
    typedef bool (*SaveFunction)(const Translator &, QIODevice &out, ConversionData &data);
    typedef bool (*LoadFunction)(Translator &, QIODevice &in, ConversionData &data);
    struct FileFormat {
        FileFormat() : untranslatedDescription(nullptr), loader(0), saver(0), priority(-1) {}
        QString extension; // such as "ts", "xlf", ...
        const char *untranslatedDescription;
        // human-readable description
        QString description() const { return FMT::tr(untranslatedDescription); }
        LoadFunction loader;
        SaveFunction saver;
        enum FileType { TranslationSource, TranslationBinary } fileType;
        int priority; // 0 = highest, -1 = invisible
    };
    static void registerFileFormat(const FileFormat &format);
    static QList<FileFormat> &registeredFileFormats();

    enum {
        TextVariantSeparator = 0x2762, // some weird character nobody ever heard of :-D
        BinaryVariantSeparator = 0x9c // unicode "STRING TERMINATOR"
    };

private:
    void insert(int idx, const TranslatorMessage &msg);
    void addIndex(int idx, const TranslatorMessage &msg) const;
    void delIndex(int idx) const;
    void ensureIndexed() const;

    typedef QList<TranslatorMessage> TMM;       // int stores the sequence position.

    TMM m_messages;
    LocationsType m_locationsType;

    // A string beginning with a 2 or 3 letter language code (ISO 639-1
    // or ISO-639-2), followed by the optional territory variant to distinguish
    // between territory-specific variations of the language. The language code
    // and territory code are always separated by '_'
    // Note that the language part can also be a 3-letter ISO 639-2 code.
    // Legal examples:
    // 'pt'         portuguese, assumes portuguese from portugal
    // 'pt_BR'      Brazilian portuguese (ISO 639-1 language code)
    // 'por_BR'     Brazilian portuguese (ISO 639-2 language code)
    QString m_language;
    QString m_sourceLanguage;
    QStringList m_dependencies;
    ExtraData m_extra;

    mutable bool m_indexOk;
    mutable QHash<QString, int> m_ctxCmtIdx;
    mutable QHash<QString, int> m_idMsgIdx;
    mutable QHash<TMMKey, int> m_msgIdx;
};

bool getNumerusInfo(QLocale::Language language, QLocale::Territory territory, QByteArray *rules,
                    QStringList *forms, const char **gettextRules);

QString getNumerusInfoString();

bool saveQM(const Translator &translator, QIODevice &dev, ConversionData &cd);

/*
  This is a quick hack. The proper way to handle this would be
  to extend Translator's interface.
*/
#define ContextComment "QT_LINGUIST_INTERNAL_CONTEXT_COMMENT"

QT_END_NAMESPACE

#endif
