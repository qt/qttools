// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translator.h"

#include "simtexth.h"

#include <iostream>

#include <stdio.h>
#ifdef Q_OS_WIN
// required for _setmode, to avoid _O_TEXT streams...
#  include <io.h> // for _setmode
#  include <fcntl.h> // for _O_BINARY
#endif

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QTextStream>

#include <private/qtranslator_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

Translator::Translator() :
    m_locationsType(AbsoluteLocations),
    m_indexOk(true)
{
}

void Translator::registerFileFormat(const FileFormat &format)
{
    //qDebug() << "Translator: Registering format " << format.extension;
    QList<Translator::FileFormat> &formats = registeredFileFormats();
    for (int i = 0; i < formats.size(); ++i)
        if (format.fileType == formats[i].fileType && format.priority < formats[i].priority) {
            formats.insert(i, format);
            return;
        }
    formats.append(format);
}

QList<Translator::FileFormat> &Translator::registeredFileFormats()
{
    static QList<Translator::FileFormat> theFormats;
    return theFormats;
}

void Translator::addIndex(int idx, const TranslatorMessage &msg) const
{

    m_msgIdx[TMMKey(msg)] = idx;
    if (!msg.id().isEmpty())
        m_idMsgIdx[msg.id()] = idx;

}

void Translator::delIndex(int idx) const
{
    const TranslatorMessage &msg = m_messages.at(idx);

    m_msgIdx.remove(TMMKey(msg));
    if (!msg.id().isEmpty())
        m_idMsgIdx.remove(msg.id());

}

void Translator::ensureIndexed() const
{
    if (!m_indexOk) {
        m_indexOk = true;
        m_idMsgIdx.clear();
        m_msgIdx.clear();
        for (int i = 0; i < m_messages.size(); i++)
            addIndex(i, m_messages.at(i));
    }
}

void Translator::replaceSorted(const TranslatorMessage &msg)
{
    int index = find(msg);
    if (index == -1) {
        appendSorted(msg);
    } else {
        delIndex(index);
        m_messages[index] = msg;
        addIndex(index, msg);
    }
}

static QString elidedId(const QString &id, int len)
{
    return id.size() <= len ? id : id.left(len - 5) + "[...]"_L1;
}

static QString makeMsgId(const TranslatorMessage &msg)
{
    QString id = msg.context() + "//"_L1 + elidedId(msg.sourceText(), 100);
    if (!msg.comment().isEmpty())
        id += "//"_L1 + elidedId(msg.comment(), 30);
    return id;
}

void Translator::extend(const TranslatorMessage &msg, ConversionData &cd)
{
    int index = find(msg);
    if (index == -1) {
        append(msg);
    } else {
        TranslatorMessage &emsg = m_messages[index];
        if (emsg.sourceText().isEmpty()) {
            delIndex(index);
            emsg.setSourceText(msg.sourceText());
            addIndex(index, msg);
        } else if (!msg.sourceText().isEmpty() && emsg.sourceText() != msg.sourceText()) {
            cd.appendError(
                    "Contradicting source strings for message with id '%1'."_L1.arg(emsg.id()));
            return;
        }
        if (emsg.extras().isEmpty()) {
            emsg.setExtras(msg.extras());
        } else if (!msg.extras().isEmpty() && emsg.extras() != msg.extras()) {
            cd.appendError("Contradicting meta data for %1."_L1.arg(
                    !emsg.id().isEmpty() ? "message with id '%1'"_L1.arg(emsg.id())
                                         : "message '%1'"_L1.arg(makeMsgId(msg))));
            return;
        }
        emsg.addReferenceUniq(msg.fileName(), msg.lineNumber());
        if (!msg.extraComment().isEmpty()) {
            QString cmt = emsg.extraComment();
            if (!cmt.isEmpty()) {
                QStringList cmts = cmt.split("\n----------\n"_L1);
                if (!cmts.contains(msg.extraComment())) {
                    cmts.append(msg.extraComment());
                    cmt = cmts.join("\n----------\n"_L1);
                }
            } else {
                cmt = msg.extraComment();
            }
            emsg.setExtraComment(cmt);
        }
        if (emsg.label().isEmpty())
            emsg.setLabel(msg.label());
        else if (!msg.label().isEmpty() && emsg.label() != msg.label())
            cd.appendError("Contradicting label for message with id %1"_L1.arg(emsg.id()));
    }
}

void Translator::insert(int idx, const TranslatorMessage &msg)
{
    if (m_indexOk) {
        if (idx == m_messages.size())
            addIndex(idx, msg);
        else
            m_indexOk = false;
    }
    m_messages.insert(idx, msg);
}

void Translator::append(const TranslatorMessage &msg)
{
    insert(m_messages.size(), msg);
}

void Translator::appendSorted(const TranslatorMessage &msg)
{
    int msgLine = msg.lineNumber();
    if (msgLine < 0) {
        append(msg);
        return;
    }

    int bestIdx = 0; // Best insertion point found so far
    int bestScore = 0; // Its category: 0 = no hit, 1 = pre or post, 2 = middle
    int bestSize = 0; // The length of the region. Longer is better within one category.

    // The insertion point to use should this region turn out to be the best one so far
    int thisIdx = 0;
    int thisScore = 0;
    int thisSize = 0;
    // Working vars
    int prevLine = 0;
    int curIdx = 0;
    for (const TranslatorMessage &mit : std::as_const(m_messages)) {
        bool sameFile = mit.fileName() == msg.fileName() && mit.context() == msg.context();
        int curLine;
        if (sameFile && (curLine = mit.lineNumber()) >= prevLine) {
            if (msgLine >= prevLine && msgLine < curLine) {
                thisIdx = curIdx;
                thisScore = thisSize ? 2 : 1;
            }
            ++thisSize;
            prevLine = curLine;
        } else {
            if (thisSize) {
                if (!thisScore) {
                    thisIdx = curIdx;
                    thisScore = 1;
                }
                if (thisScore > bestScore || (thisScore == bestScore && thisSize > bestSize)) {
                    bestIdx = thisIdx;
                    bestScore = thisScore;
                    bestSize = thisSize;
                }
                thisScore = 0;
                thisSize = sameFile ? 1 : 0;
                prevLine = 0;
            }
        }
        ++curIdx;
    }
    if (thisSize && !thisScore) {
        thisIdx = curIdx;
        thisScore = 1;
    }
    if (thisScore > bestScore || (thisScore == bestScore && thisSize > bestSize))
        insert(thisIdx, msg);
    else if (bestScore)
        insert(bestIdx, msg);
    else
        append(msg);
}

static QString guessFormat(const QString &filename, const QString &format)
{
    if (format != "auto"_L1)
        return format;

    for (const Translator::FileFormat &fmt : std::as_const(Translator::registeredFileFormats())) {
        if (filename.endsWith(u'.' + fmt.extension, Qt::CaseInsensitive))
            return fmt.extension;
    }

    // the default format.
    // FIXME: change to something more widely distributed later.
    return "ts"_L1;
}

static QString getDependencyName(const QString &filename, const QString &format)
{
    const QString file = QFileInfo(filename).fileName();
    const QString fmt = guessFormat(file, format);

    if (file.endsWith(u'.' + fmt))
        return file.chopped(fmt.size() + 1);

    // no extension in the file name
    return file;
}

bool Translator::load(const QString &filename, ConversionData &cd, const QString &format)
{
    cd.m_sourceDir = QFileInfo(filename).absoluteDir();
    cd.m_sourceFileName = filename;

    QFile file;
    if (filename.isEmpty() || filename == "-"_L1) {
#ifdef Q_OS_WIN
        // QFile is broken for text files
        ::_setmode(0, _O_BINARY);
#endif
        if (!file.open(stdin, QIODevice::ReadOnly)) {
            cd.appendError(QString::fromLatin1("Cannot open stdin!? (%1)")
                .arg(file.errorString()));
            return false;
        }
    } else {
        file.setFileName(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            cd.appendError(QString::fromLatin1("Cannot open %1: %2")
                .arg(filename, file.errorString()));
            return false;
        }
    }

    QString fmt = guessFormat(filename, format);

    for (const FileFormat &format : std::as_const(registeredFileFormats())) {
        if (fmt == format.extension) {
            if (format.loader)
                return (*format.loader)(*this, file, cd);
            cd.appendError(QString("No loader for format %1 found"_L1).arg(fmt));
            return false;
        }
    }

    cd.appendError(QString("Unknown format %1 for file %2"_L1).arg(format, filename));
    return false;
}


bool Translator::save(const QString &filename, ConversionData &cd, const QString &format) const
{
    QFile file;
    if (filename.isEmpty() || filename == "-"_L1) {
#ifdef Q_OS_WIN
        // QFile is broken for text files
        ::_setmode(1, _O_BINARY);
#endif
        if (!file.open(stdout, QIODevice::WriteOnly)) {
            cd.appendError(QString::fromLatin1("Cannot open stdout!? (%1)")
                .arg(file.errorString()));
            return false;
        }
    } else {
        file.setFileName(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            cd.appendError(QString::fromLatin1("Cannot create %1: %2")
                .arg(filename, file.errorString()));
            return false;
        }
    }

    QString fmt = guessFormat(filename, format);
    cd.m_targetDir = QFileInfo(filename).absoluteDir();

    for (const FileFormat &format : std::as_const(registeredFileFormats())) {
        if (fmt == format.extension) {
            if (format.saver) {
                if (fmt != u"ts" && m_locationsType == RelativeLocations)
                    std::cerr << "Warning: relative locations are not supported for non TS files. "
                                 "File "
                              << qPrintable(filename)
                              << " will be generated with the "
                                 "default location type."
                              << std::endl;
                return (*format.saver)(*this, file, cd);
            }
            cd.appendError(QString("Cannot save %1 files"_L1).arg(fmt));
            return false;
        }
    }

    cd.appendError(QString("Unknown format %1 for file %2"_L1).arg(format).arg(filename));
    return false;
}

QString Translator::makeLanguageCode(QLocale::Language language, QLocale::Territory territory)
{
    QString result = QLocale::languageToCode(language);
    if (language != QLocale::C && territory != QLocale::AnyTerritory) {
        result.append(u'_');
        result.append(QLocale::territoryToCode(territory));
    }
    return result;
}

void Translator::languageAndTerritory(QStringView languageCode, QLocale::Language *langPtr,
                                    QLocale::Territory *territoryPtr)
{
    QLocale::Language language = QLocale::AnyLanguage;
    QLocale::Territory territory = QLocale::AnyTerritory;
    auto separator = languageCode.indexOf(u'_'); // "de_DE"
    if (separator == -1) {
        // compatibility with older .ts files
        separator = languageCode.indexOf(u'-'); // "de-DE"
    }
    if (separator != -1) {
        language = QLocale::codeToLanguage(languageCode.left(separator));
        territory = QLocale::codeToTerritory(languageCode.mid(separator + 1));
    } else {
        language = QLocale::codeToLanguage(languageCode);
        territory = QLocale(language).territory();
    }

    if (langPtr)
        *langPtr = language;
    if (territoryPtr)
        *territoryPtr = territory;
}

int Translator::find(const TranslatorMessage &msg) const
{
    ensureIndexed();
    if (msg.id().isEmpty())
        return m_msgIdx.value(TMMKey(msg), -1);
    int i = m_idMsgIdx.value(msg.id(), -1);
    if (i >= 0)
        return i;
    i = m_msgIdx.value(TMMKey(msg), -1);
    // If both have an id, then find only by id.
    return i >= 0 && m_messages.at(i).id().isEmpty() ? i : -1;
}

int Translator::find(const QString &context,
    const QString &comment, const TranslatorMessage::References &refs) const
{
    if (!refs.isEmpty()) {
        for (auto it = m_messages.cbegin(), end = m_messages.cend(); it != end; ++it) {
            if (it->context() == context && it->comment() == comment) {
                for (const auto &itref : it->allReferences()) {
                    for (const auto &ref : refs) {
                        if (itref == ref)
                            return it - m_messages.cbegin();
                    }
                }
            }
        }
    }
    return -1;
}

void Translator::stripObsoleteMessages()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
        if (it->type() == TranslatorMessage::Obsolete || it->type() == TranslatorMessage::Vanished)
            it = m_messages.erase(it);
        else
            ++it;
    m_indexOk = false;
}

void Translator::stripFinishedMessages()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
        if (it->type() == TranslatorMessage::Finished)
            it = m_messages.erase(it);
        else
            ++it;
    m_indexOk = false;
}

void Translator::stripUntranslatedMessages()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
        if (!it->isTranslated())
            it = m_messages.erase(it);
        else
            ++it;
    m_indexOk = false;
}

bool Translator::translationsExist() const
{
    return std::any_of(m_messages.cbegin(), m_messages.cend(),
                       [](const auto &m) { return m.isTranslated(); });
}

bool Translator::unfinishedTranslationsExist() const
{
    return std::any_of(m_messages.cbegin(), m_messages.cend(),
                       [](const auto &m) { return m.type() == TranslatorMessage::Unfinished; });
}

void Translator::stripEmptyContexts()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
        if (it->sourceText() == QLatin1String(ContextComment))
            it = m_messages.erase(it);
        else
            ++it;
    m_indexOk = false;
}

void Translator::stripNonPluralForms()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
        if (!it->isPlural())
            it = m_messages.erase(it);
        else
            ++it;
    m_indexOk = false;
}

void Translator::stripIdenticalSourceTranslations()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); ) {
        // we need to have just one translation, and it be equal to the source
        if (it->translations().size() == 1 && it->translation() == it->sourceText())
            it = m_messages.erase(it);
        else
            ++it;
    }
    m_indexOk = false;
}

void Translator::dropTranslations()
{
    for (auto &message : m_messages) {
        if (message.type() == TranslatorMessage::Finished)
            message.setType(TranslatorMessage::Unfinished);
        message.setTranslation(QString());
    }
}

void Translator::dropUiLines()
{
    const QString uiXt = ".ui"_L1;
    const QString juiXt = ".jui"_L1;
    for (auto &message : m_messages) {
        QHash<QString, int> have;
        QList<TranslatorMessage::Reference> refs;
        for (const auto &itref : message.allReferences()) {
            const QString &fn = itref.fileName();
            if (fn.endsWith(uiXt) || fn.endsWith(juiXt)) {
                if (++have[fn] == 1)
                    refs.append(TranslatorMessage::Reference(fn, -1));
            } else {
                refs.append(itref);
            }
        }
        message.setReferences(refs);
    }
}

class TranslatorMessagePtrBase
{
public:
    explicit TranslatorMessagePtrBase(const Translator *tor, int messageIndex)
        : tor(tor), messageIndex(messageIndex)
    {
    }

    inline const TranslatorMessage *operator->() const
    {
        return &tor->message(messageIndex);
    }

    const Translator *tor;
    const int messageIndex;
};

class TranslatorMessageIdPtr : public TranslatorMessagePtrBase
{
public:
    using TranslatorMessagePtrBase::TranslatorMessagePtrBase;
};

Q_DECLARE_TYPEINFO(TranslatorMessageIdPtr, Q_RELOCATABLE_TYPE);

inline size_t qHash(TranslatorMessageIdPtr tmp)
{
    return qHash(tmp->id());
}

inline bool operator==(TranslatorMessageIdPtr tmp1, TranslatorMessageIdPtr tmp2)
{
    return tmp1->id() == tmp2->id();
}

class TranslatorMessageContentPtr : public TranslatorMessagePtrBase
{
public:
    using TranslatorMessagePtrBase::TranslatorMessagePtrBase;
};

Q_DECLARE_TYPEINFO(TranslatorMessageContentPtr, Q_RELOCATABLE_TYPE);

inline size_t qHash(TranslatorMessageContentPtr tmp)
{
    size_t hash = qHash(tmp->context()) ^ qHash(tmp->sourceText());
    if (!tmp->sourceText().isEmpty())
        // Special treatment for context comments (empty source).
        hash ^= qHash(tmp->comment());
    return hash;
}

inline bool operator==(TranslatorMessageContentPtr tmp1, TranslatorMessageContentPtr tmp2)
{
    if (tmp1->context() != tmp2->context() || tmp1->sourceText() != tmp2->sourceText())
        return false;
    // Special treatment for context comments (empty source).
    if (tmp1->sourceText().isEmpty())
        return true;
    return tmp1->comment() == tmp2->comment();
}

Translator::Duplicates Translator::resolveDuplicates()
{
    Duplicates dups;
    QSet<TranslatorMessageIdPtr> idRefs;
    QSet<TranslatorMessageContentPtr> contentRefs;
    for (int i = 0; i < m_messages.size();) {
        const TranslatorMessage &msg = m_messages.at(i);
        TranslatorMessage *omsg;
        int oi;
        DuplicateEntries *pDup;
        if (!msg.id().isEmpty()) {
            const auto it = idRefs.constFind(TranslatorMessageIdPtr(this, i));
            if (it != idRefs.constEnd()) {
                oi = it->messageIndex;
                omsg = &m_messages[oi];
                pDup = &dups.byId;
                goto gotDupe;
            }
        }
        {
            const auto it = contentRefs.constFind(TranslatorMessageContentPtr(this, i));
            if (it != contentRefs.constEnd()) {
                oi = it->messageIndex;
                omsg = &m_messages[oi];
                if (msg.id().isEmpty() || omsg->id().isEmpty()) {
                    if (!msg.id().isEmpty() && omsg->id().isEmpty()) {
                        omsg->setId(msg.id());
                        idRefs.insert(TranslatorMessageIdPtr(this, oi));
                    }
                    pDup = &dups.byContents;
                    goto gotDupe;
                }
                // This is really a content dupe, but with two distinct IDs.
            }
        }
        if (!msg.id().isEmpty())
            idRefs.insert(TranslatorMessageIdPtr(this, i));
        contentRefs.insert(TranslatorMessageContentPtr(this, i));
        ++i;
        continue;
      gotDupe:
        (*pDup)[oi].append(msg.tsLineNumber());
        if (!omsg->isTranslated() && msg.isTranslated())
            omsg->setTranslations(msg.translations());
        m_indexOk = false;
        m_messages.removeAt(i);
    }
    return dups;
}

void Translator::reportDuplicates(const Duplicates &dupes,
                                  const QString &fileName, bool verbose)
{
    if (!dupes.byId.isEmpty() || !dupes.byContents.isEmpty()) {
        std::cerr << "Warning: dropping duplicate messages in '" << qPrintable(fileName);
        if (!verbose) {
            std::cerr << "'\n(try -verbose for more info).\n";
        } else {
            std::cerr << "':\n";
            for (auto it = dupes.byId.begin(); it != dupes.byId.end(); ++it) {
                const TranslatorMessage &msg = message(it.key());
                std::cerr << "\n* ID: " << qPrintable(msg.id()) << std::endl;
                reportDuplicatesLines(msg, it.value());
            }
            for (auto it = dupes.byContents.begin(); it != dupes.byContents.end(); ++it) {
                const TranslatorMessage &msg = message(it.key());
                std::cerr << "\n* Context: " << qPrintable(msg.context())
                          << "\n* Source: " << qPrintable(msg.sourceText()) << std::endl;
                if (!msg.comment().isEmpty())
                    std::cerr << "* Comment: " << qPrintable(msg.comment()) << std::endl;
                reportDuplicatesLines(msg, it.value());
            }
            std::cerr << std::endl;
        }
    }
}

void Translator::reportDuplicatesLines(const TranslatorMessage &msg,
                                       const DuplicateEntries::value_type &dups) const
{
    if (msg.tsLineNumber() >= 0) {
        std::cerr << "* Line in .ts file: " << msg.tsLineNumber() << std::endl;
        for (int tsLineNumber : dups) {
            if (tsLineNumber >= 0)
                std::cerr << "* Duplicate at line: " << tsLineNumber << std::endl;
        }
    }
}

// Used by lupdate to be able to search using absolute paths during merging
void Translator::makeFileNamesAbsolute(const QDir &originalPath)
{
    for (auto &msg : m_messages) {
        const TranslatorMessage::References refs = msg.allReferences();
        msg.setReferences(TranslatorMessage::References());
        for (const TranslatorMessage::Reference &ref : refs) {
            QString fileName = ref.fileName();
            QFileInfo fi (fileName);
            if (fi.isRelative())
                fileName = originalPath.absoluteFilePath(fileName);
            msg.addReference(fileName, ref.lineNumber());
        }
    }
}

const QList<TranslatorMessage> &Translator::messages() const
{
    return m_messages;
}

QStringList Translator::normalizedTranslations(const TranslatorMessage &msg, int numPlurals)
{
    QStringList translations = msg.translations();
    int numTranslations = msg.isPlural() ? numPlurals : 1;

    // make sure that the stringlist always have the size of the
    // language's current numerus, or 1 if its not plural
    if (translations.size() > numTranslations) {
        for (int i = translations.size(); i > numTranslations; --i)
            translations.removeLast();
    } else if (translations.size() < numTranslations) {
        for (int i = translations.size(); i < numTranslations; ++i)
            translations.append(QString());
    }
    return translations;
}

void Translator::normalizeTranslations(ConversionData &cd)
{
    bool truncated = false;
    QLocale::Language l;
    QLocale::Territory c;
    languageAndTerritory(languageCode(), &l, &c);
    int numPlurals = 1;
    if (l != QLocale::C) {
        QStringList forms;
        if (getNumerusInfo(l, c, 0, &forms, 0))
            numPlurals = forms.size(); // includes singular
    }
    for (int i = 0; i < m_messages.size(); ++i) {
        const TranslatorMessage &msg = m_messages.at(i);
        QStringList tlns = msg.translations();
        int ccnt = msg.isPlural() ? numPlurals : 1;
        if (tlns.size() != ccnt) {
            while (tlns.size() < ccnt)
                tlns.append(QString());
            while (tlns.size() > ccnt) {
                tlns.removeLast();
                truncated = true;
            }
            m_messages[i].setTranslations(tlns);
        }
    }
    if (truncated)
        cd.appendError(QLatin1String(
            "Removed plural forms as the target language has less "
            "forms.\nIf this sounds wrong, possibly the target language is "
            "not set or recognized."));
}

QString Translator::guessLanguageCodeFromFileName(const QString &filename)
{
    QString str = filename;
    for (const FileFormat &format : std::as_const(registeredFileFormats())) {
        if (str.endsWith(format.extension)) {
            str = str.left(str.size() - format.extension.size() - 1);
            break;
        }
    }
    static QRegularExpression re("[\\._]"_L1);
    while (true) {
        QLocale locale(str);
        //qDebug() << "LANGUAGE FROM " << str << "LANG: " << locale.language();
        if (locale.language() != QLocale::C) {
            //qDebug() << "FOUND " << locale.name();
            return locale.name();
        }
        int pos = str.indexOf(re);
        if (pos == -1)
            break;
        str = str.mid(pos + 1);
    }
    //qDebug() << "LANGUAGE GUESSING UNSUCCESSFUL";
    return QString();
}

void Translator::appendDependencies(const QStringList &dependencies)
{
    QStringList mergeDeps;
    for (const QString &dep : dependencies) {
        if (const auto it = std::find(m_dependencies.cbegin(), m_dependencies.cend(), dep);
            it == m_dependencies.cend()) {
            mergeDeps.append(dep);
        }
    }
    m_dependencies.append(mergeDeps);
}

void Translator::satisfyDependency(const QString &file, const QString &format)
{
    const auto dep = getDependencyName(file, format);
    if (const auto it = std::find(m_dependencies.cbegin(), m_dependencies.cend(), dep);
        it != m_dependencies.cend()) {
        m_dependencies.erase(it);
    }
}

bool Translator::hasExtra(const QString &key) const
{
    return m_extra.contains(key);
}

QString Translator::extra(const QString &key) const
{
    return m_extra[key];
}

void Translator::setExtra(const QString &key, const QString &value)
{
    m_extra[key] = value;
}

void Translator::dump() const
{
    for (int i = 0; i != messageCount(); ++i)
        message(i).dump();
}

QT_END_NAMESPACE
