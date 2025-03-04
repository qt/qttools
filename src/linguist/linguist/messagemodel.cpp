// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "messagemodel.h"

#include "globals.h"
#include "statistics.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QTextDocument>

#include <private/qtranslator_p.h>

#include <limits.h>

using namespace Qt::Literals::StringLiterals;

static QString resolveNcr(QStringView str)
{
    constexpr QStringView notation = u"&#";
    constexpr QChar cx = u'x';
    constexpr QChar ce = u';';

    QString result;
    result.reserve(str.size());
    qsizetype offset = str.indexOf(notation);
    while (offset >= 0) {

        qsizetype metaLen = 2;
        if (str.size() <= offset + metaLen)
            break;

        int base = 10;
        if (const QChar ch = str[offset + metaLen]; ch == cx) {
            metaLen++;
            base = 16;
        }
        offset += metaLen;

        const qsizetype end = str.sliced(offset).indexOf(ce);
        if (end > 0) {
            bool valid;
            if (const uint c = str.sliced(offset, end).toUInt(&valid, base);
                valid && c <= QChar::LastValidCodePoint) {
                if (QChar::requiresSurrogates(c))
                    result += str.sliced(0, offset - metaLen) + QChar(QChar::highSurrogate(c))
                            + QChar(QChar::lowSurrogate(c));
                else
                    result += str.sliced(0, offset - metaLen) + QChar(c);
                str.slice(offset + end + 1);
                offset = str.indexOf(notation);
                continue;
            }
        }
        result += str.sliced(0, offset);
        str.slice(offset);
        offset = str.indexOf(notation);
    }
    result += str;
    return result;
}

static QString showNcr(const QString &str)
{
    QString result;
    result.reserve(str.size());
    for (const QChar ch : str) {
        if (uint c = ch.unicode(); Q_UNLIKELY(!ch.isPrint() && c > 0x20))
            result += QString("&#x%1;"_L1).arg(c, 0, 16);
        else
            result += ch;
    }
    return result;
}

static QString adjustNcrVisibility(const QString &str, bool ncrMode)
{
    return ncrMode ? showNcr(str) : resolveNcr(str);
}

QT_BEGIN_NAMESPACE

/******************************************************************************
 *
 * MessageItem
 *
 *****************************************************************************/

MessageItem::MessageItem(const TranslatorMessage &message)
    : m_message(message), m_danger(false), m_ncrMode(false)
{
    if (m_message.translation().isEmpty())
        m_message.setTranslation(QString());
}


bool MessageItem::compare(const QString &findText, bool matchSubstring,
    Qt::CaseSensitivity cs) const
{
    return matchSubstring
        ? text().indexOf(findText, 0, cs) >= 0
        : text().compare(findText, cs) == 0;
}

void MessageItem::setTranslation(const QString &translation)
{
    m_message.setTranslation(resolveNcr(translation));
}

QString MessageItem::text() const
{
    return adjustNcrVisibility(m_message.sourceText(), m_ncrMode);
}

QString MessageItem::pluralText() const
{
    return adjustNcrVisibility(m_message.extra("po-msgid_plural"_L1), m_ncrMode);
}

QString MessageItem::translation() const
{
    return adjustNcrVisibility(m_message.translation(), m_ncrMode);
}

QStringList MessageItem::translations() const
{
    QStringList translations;
    translations.reserve(m_message.translations().size());
    for (QString &trans : m_message.translations())
        translations.append(adjustNcrVisibility(trans, m_ncrMode));
    return translations;
}

void MessageItem::setTranslations(const QStringList &translations)
{
    QStringList trans;
    trans.reserve(translations.size());
    for (const QString &t : translations)
        trans.append(resolveNcr(t));

    m_message.setTranslations(trans);
}

/******************************************************************************
 *
 * GroupItem
 *
 *****************************************************************************/

void GroupItem::appendToComment(const QString &str)
{
    if (!m_comment.isEmpty())
        m_comment += "\n\n"_L1;
    m_comment += str;
}

MessageItem *GroupItem::messageItem(int i) const
{
    if (i >= 0 && i < msgItemList.size())
        return const_cast<MessageItem *>(&msgItemList[i]);
    Q_ASSERT(i >= 0 && i < msgItemList.size());
    return 0;
}

MessageItem *GroupItem::findMessage(const QString &sourcetext, const QString &comment) const
{
    for (int i = 0; i < messageCount(); ++i) {
        MessageItem *mi = messageItem(i);
        if (mi->text() == sourcetext && mi->comment() == comment)
            return mi;
    }
    return 0;
}

MessageItem *GroupItem::findMessageById(const QString &msgid) const
{
    for (int i = 0, cnt = messageCount(); i < cnt; ++i) {
        MessageItem *m = messageItem(i);
        if (m->id() == msgid)
            return m;
    }
    return 0;
}

/******************************************************************************
 *
 * DataModel
 *
 *****************************************************************************/

DataModel::DataModel(QObject *parent)
  : QObject(parent),
    m_modified(false),
    m_numMessages(0),
    m_srcWords(0),
    m_srcChars(0),
    m_srcCharsSpc(0),
    m_language(QLocale::Language(-1)),
    m_sourceLanguage(QLocale::Language(-1)),
    m_territory(QLocale::Territory(-1)),
    m_sourceTerritory(QLocale::Territory(-1))
{}

QStringList DataModel::normalizedTranslations(const MessageItem &m) const
{
    QStringList translations =
            Translator::normalizedTranslations(m.message(), m_numerusForms.size());
    QStringList ncrTranslations;
    ncrTranslations.reserve(translations.size());
    for (const QString &translate : std::as_const(translations))
        ncrTranslations.append(adjustNcrVisibility(translate, m.ncrMode()));
    return ncrTranslations;
}

GroupItem *DataModel::groupItem(int groupId, TranslationType type) const
{
    const auto &list = type == IDBASED ? m_labelList : m_contextList;
    if (groupId >= 0 && groupId < list.size())
        return const_cast<GroupItem *>(&list[groupId]);
    return 0;
}

GroupItem *DataModel::groupItem(DataIndex index) const
{
    return groupItem(index.group(), index.translationType());
}

MessageItem *DataModel::messageItem(const DataIndex &index) const
{
    if (GroupItem *g = groupItem(index))
        return g->messageItem(index.message());
    return 0;
}

GroupItem *DataModel::findGroup(const QString &groupName, TranslationType type) const
{
    const auto &list = type == IDBASED ? m_labelList : m_contextList;
    for (int g = 0; g < list.size(); ++g) {
        GroupItem *gi = groupItem(g, type);
        if (gi->group() == groupName)
            return gi;
    }
    return 0;
}

MessageItem *DataModel::findMessage(const QString &context, const QString &label,
                                    const QString &sourcetext, const QString &comment) const
{
    if (context.isEmpty()) {
        if (GroupItem *gi = findGroup(label, IDBASED))
            return gi->findMessage(sourcetext, comment);
    } else {
        if (GroupItem *gi = findGroup(context, TEXTBASED))
            return gi->findMessage(sourcetext, comment);
    }
    return 0;
}

static int calcMergeScore(const DataModel *one, const DataModel *two)
{
    int inBoth = 0;

    auto countSameMessages = [two, one, &inBoth](int count, TranslationType type) {
        for (int i = 0; i < count; ++i) {
            GroupItem *gi = two->groupItem(i, type);
            if (GroupItem *g = one->findGroup(gi->group(), type)) {
                for (int j = 0; j < gi->messageCount(); ++j) {
                    MessageItem *m = gi->messageItem(j);
                    if (g->findMessage(m->text(), m->comment()))
                        ++inBoth;
                }
            }
        }
    };

    countSameMessages(two->contextCount(), TEXTBASED);
    countSameMessages(two->labelCount(), IDBASED);

    return inBoth * 100 / two->messageCount();
}

bool DataModel::isWellMergeable(const DataModel *other) const
{
    if (!other->messageCount() || !messageCount())
        return true;

    return calcMergeScore(this, other) + calcMergeScore(other, this) > 90;
}

bool DataModel::load(const QString &fileName, bool *langGuessed, QWidget *parent)
{
    Translator tor;
    ConversionData cd;
    bool ok = tor.load(fileName, cd, "auto"_L1);
    if (!ok) {
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"), cd.error());
        return false;
    }

    if (!tor.messageCount()) {
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"),
                             tr("The translation file '%1' will not be loaded because it is empty.")
                             .arg(fileName.toHtmlEscaped()));
        return false;
    }

    const Translator::Duplicates dupes = tor.resolveDuplicates();
    if (!dupes.byId.isEmpty() || !dupes.byContents.isEmpty()) {
        QString err = tr("<qt>Duplicate messages found in '%1':").arg(fileName.toHtmlEscaped());
        int numdups = 0;
        for (auto it = dupes.byId.begin(); it != dupes.byId.end(); ++it) {
            if (++numdups >= 5) {
                err += tr("<p>[more duplicates omitted]");
                goto doWarn;
            }
            err += tr("<p>* ID: %1").arg(tor.message(it.key()).id().toHtmlEscaped());
        }
        for (auto it = dupes.byContents.begin(); it != dupes.byContents.end(); ++it) {
            const TranslatorMessage &msg = tor.message(it.key());
            if (++numdups >= 5) {
                err += tr("<p>[more duplicates omitted]");
                break;
            }
            err += tr("<p>* Context: %1<br>* Source: %2")
                    .arg(msg.context().toHtmlEscaped(), msg.sourceText().toHtmlEscaped());
            if (!msg.comment().isEmpty())
                err += tr("<br>* Comment: %3").arg(msg.comment().toHtmlEscaped());
        }
      doWarn:
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"), err);
    }

    m_srcFileName = fileName;
    m_relativeLocations = (tor.locationsType() == Translator::RelativeLocations);
    m_extra = tor.extras();
    m_contextList.clear();
    m_labelList.clear();
    m_numMessages = 0;

    m_srcWords = 0;
    m_srcChars = 0;
    m_srcCharsSpc = 0;

    auto loadMessage = [this](const TranslatorMessage &msg, const QString &group,
                              QList<GroupItem> &list, QHash<QString, int> &groups,
                              TranslationType type) {
        if (!groups.contains(group)) {
            groups.insert(group, list.size());
            list.append(GroupItem(type, group));
        }
        GroupItem *gi = groupItem(groups.value(group), type);
        if (msg.sourceText() == QLatin1String(ContextComment)) {
            gi->appendToComment(msg.comment());
        } else {
            MessageItem tmp(msg);
            if (msg.type() == TranslatorMessage::Finished)
                gi->incrementFinishedCount();
            if (msg.type() == TranslatorMessage::Finished
                || msg.type() == TranslatorMessage::Unfinished) {
                doCharCounting(tmp.text(), m_srcWords, m_srcChars, m_srcCharsSpc);
                doCharCounting(tmp.pluralText(), m_srcWords, m_srcChars, m_srcCharsSpc);
                gi->incrementNonobsoleteCount();
            }
            gi->appendMessage(tmp);
            ++m_numMessages;
        }
    };

    QHash<QString, int> labels;
    QHash<QString, int> contexts;
    for (const TranslatorMessage &msg : tor.messages()) {
        if (const QString ctx = msg.context(); !ctx.isEmpty())
            loadMessage(msg, ctx, m_contextList, contexts, TEXTBASED);
        else
            loadMessage(msg, msg.label(), m_labelList, labels, IDBASED);
    }

    // Try to detect the correct language in the following order
    // 1. Look for the language attribute in the ts
    //   if that fails
    // 2. Guestimate the language from the filename
    //   (expecting the qt_{en,de}.ts convention)
    //   if that fails
    // 3. Retrieve the locale from the system.
    *langGuessed = false;
    QString lang = tor.languageCode();
    if (lang.isEmpty()) {
        lang = QFileInfo(fileName).baseName();
        int pos = lang.indexOf(u'_');
        if (pos != -1)
            lang.remove(0, pos + 1);
        else
            lang.clear();
        *langGuessed = true;
    }
    QLocale::Language l;
    QLocale::Territory c;
    Translator::languageAndTerritory(lang, &l, &c);
    if (l == QLocale::C) {
        QLocale sys;
        l = sys.language();
        c = sys.territory();
        *langGuessed = true;
    }
    if (!setLanguageAndTerritory(l, c))
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"),
                             tr("Linguist does not know the plural rules for '%1'.\n"
                                "Will assume a single universal form.")
                             .arg(m_localizedLanguage));
    // Try to detect the correct source language in the following order
    // 1. Look for the language attribute in the ts
    //   if that fails
    // 2. Assume English
    lang = tor.sourceLanguageCode();
    if (lang.isEmpty()) {
        l = QLocale::C;
        c = QLocale::AnyTerritory;
    } else {
        Translator::languageAndTerritory(lang, &l, &c);
    }
    setSourceLanguageAndTerritory(l, c);

    setModified(false);

    return true;
}

bool DataModel::save(const QString &fileName, QWidget *parent)
{
    Translator tor;
    for (DataModelIterator it(IDBASED, this); it.isValid(); ++it)
        tor.append(it.current()->message());
    for (DataModelIterator it(TEXTBASED, this); it.isValid(); ++it)
        tor.append(it.current()->message());

    tor.setLanguageCode(Translator::makeLanguageCode(m_language, m_territory));
    tor.setSourceLanguageCode(Translator::makeLanguageCode(m_sourceLanguage, m_sourceTerritory));
    tor.setLocationsType(m_relativeLocations ? Translator::RelativeLocations
                                             : Translator::AbsoluteLocations);
    tor.setExtras(m_extra);
    ConversionData cd;
    tor.normalizeTranslations(cd);
    bool ok = tor.save(fileName, cd, "auto"_L1);
    if (ok)
        setModified(false);
    if (!cd.error().isEmpty())
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"), cd.error());
    return ok;
}

bool DataModel::saveAs(const QString &newFileName, QWidget *parent)
{
    if (!save(newFileName, parent))
        return false;
    m_srcFileName = newFileName;
    return true;
}

bool DataModel::release(const QString &fileName, bool verbose, bool ignoreUnfinished,
    TranslatorSaveMode mode, QWidget *parent)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"),
            tr("Cannot create '%2': %1").arg(file.errorString()).arg(fileName));
        return false;
    }
    Translator tor;
    QLocale locale(m_language, m_territory);
    tor.setLanguageCode(locale.name());
    for (DataModelIterator it(IDBASED, this); it.isValid(); ++it)
        tor.append(it.current()->message());
    for (DataModelIterator it(TEXTBASED, this); it.isValid(); ++it)
        tor.append(it.current()->message());
    ConversionData cd;
    cd.m_verbose = verbose;
    cd.m_ignoreUnfinished = ignoreUnfinished;
    cd.m_saveMode = mode;
    bool ok = saveQM(tor, file, cd);
    if (!ok)
        QMessageBox::warning(parent, QObject::tr("Qt Linguist"), cd.error());
    return ok;
}

void DataModel::doCharCounting(const QString &text, int &trW, int &trC, int &trCS)
{
    trCS += text.size();
    bool inWord = false;
    for (int i = 0; i < text.size(); ++i) {
        if (text[i].isLetterOrNumber() || text[i] == u'_') {
            if (!inWord) {
                ++trW;
                inWord = true;
            }
        } else {
            inWord = false;
        }
        if (!text[i].isSpace())
            trC++;
    }
}

bool DataModel::setLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory)
{
    if (m_language == lang && m_territory == territory)
        return true;
    m_language = lang;
    m_territory = territory;

    if (lang == QLocale::C || uint(lang) > uint(QLocale::LastLanguage)) // XXX does this make any sense?
        lang = QLocale::English;
    QByteArray rules;
    bool ok = getNumerusInfo(lang, territory, &rules, &m_numerusForms, 0);
    QLocale loc(lang, territory);
    // Add territory name if we couldn't match the (lang, territory) combination,
    // or if the language is used in more than one territory.
    const bool mentionTerritory = (loc.territory() != territory) || [lang, territory]() {
        const auto locales = QLocale::matchingLocales(lang, QLocale::AnyScript,
                                                      QLocale::AnyTerritory);
        return std::any_of(locales.cbegin(), locales.cend(), [territory](const QLocale &locale) {
            return locale.territory() != territory;
        });
    }();
    m_localizedLanguage = mentionTerritory
            //: <language> (<territory>)
            ? tr("%1 (%2)").arg(loc.nativeLanguageName(), loc.nativeTerritoryName())
            : loc.nativeLanguageName();
    m_countRefNeeds.clear();
    for (int i = 0; i < rules.size(); ++i) {
        m_countRefNeeds.append(!(rules.at(i) == Q_EQ && (i == (rules.size() - 2) || rules.at(i + 2) == (char)Q_NEWRULE)));
        while (++i < rules.size() && rules.at(i) != (char)Q_NEWRULE) {}
    }
    m_countRefNeeds.append(true);
    if (!ok) {
        m_numerusForms.clear();
        m_numerusForms << tr("Universal Form");
    }
    emit languageChanged();
    setModified(true);
    return ok;
}

void DataModel::setSourceLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory)
{
    if (m_sourceLanguage == lang && m_sourceTerritory == territory)
        return;
    m_sourceLanguage = lang;
    m_sourceTerritory = territory;
    setModified(true);
}

void DataModel::updateStatistics()
{

    StatisticalData stats {};

    auto updateMessageStatistics = [&stats, this](const MessageItem *mi) {
        if (mi->isObsolete()) {
            stats.obsoleteMsg++;
        } else if (mi->isFinished()) {
            bool hasDanger = false;
            for (const QString &trnsl : mi->translations()) {
                doCharCounting(trnsl, stats.wordsFinished, stats.charsFinished, stats.charsSpacesFinished);
                hasDanger |= mi->danger();
            }
            if (hasDanger)
                stats.translatedMsgDanger++;
            else
                stats.translatedMsgNoDanger++;
        } else if (mi->isUnfinished()) {
            bool hasDanger = false;
            for (const QString &trnsl : mi->translations()) {
                doCharCounting(trnsl, stats.wordsUnfinished, stats.charsUnfinished, stats.charsSpacesUnfinished);
                hasDanger |= mi->danger();
            }
            if (hasDanger)
                stats.unfinishedMsgDanger++;
            else
                stats.unfinishedMsgNoDanger++;
        }
    };

    for (DataModelIterator it(IDBASED, this); it.isValid(); ++it)
        updateMessageStatistics(it.current());
    for (DataModelIterator it(TEXTBASED, this); it.isValid(); ++it)
        updateMessageStatistics(it.current());

    stats.wordsSource = m_srcWords;
    stats.charsSource = m_srcChars;
    stats.charsSpacesSource = m_srcCharsSpc;
    emit statsChanged(stats);
}

void DataModel::setModified(bool isModified)
{
    if (m_modified == isModified)
        return;
    m_modified = isModified;
    emit modifiedChanged();
}

QString DataModel::prettifyPlainFileName(const QString &fn)
{
    static QString workdir = QDir::currentPath() + u'/';

    return QDir::toNativeSeparators(fn.startsWith(workdir) ? fn.mid(workdir.size()) : fn);
}

QString DataModel::prettifyFileName(const QString &fn)
{
    if (fn.startsWith(u'='))
        return u'=' + prettifyPlainFileName(fn.mid(1));
    else
        return prettifyPlainFileName(fn);
}

/******************************************************************************
 *
 * DataModelIterator
 *
 *****************************************************************************/

DataModelIterator::DataModelIterator(TranslationType type, DataModel *model, int group, int message)
    : DataIndex(type, group, message), m_model(model)
{
}

bool DataModelIterator::isValid() const
{
    const qsizetype size =
            isIdBased() ? m_model->m_labelList.size() : m_model->m_contextList.size();
    return m_group < size;
}

void DataModelIterator::operator++()
{
    ++m_message;
    const qsizetype size = isIdBased() ? m_model->m_labelList.at(m_group).messageCount()
                                       : m_model->m_contextList.at(m_group).messageCount();
    if (m_message >= size) {
        ++m_group;
        m_message = 0;
    }
}

MessageItem *DataModelIterator::current() const
{
    return m_model->messageItem(*this);
}

/******************************************************************************
 *
 * MultiGroupItem
 *
 *****************************************************************************/

MultiGroupItem::MultiGroupItem(int oldCount, GroupItem *groupItem, bool writable)
    : m_group(groupItem->group()),
      m_comment(groupItem->comment()),
      m_translationType(groupItem->translationType())
{
    QList<MessageItem *> mList;
    QList<MessageItem *> eList;
    for (int j = 0; j < groupItem->messageCount(); ++j) {
        MessageItem *m = groupItem->messageItem(j);
        mList.append(m);
        eList.append(0);
        m_multiMessageList.append(MultiMessageItem(m));
    }
    for (int i = 0; i < oldCount; ++i) {
        m_messageLists.append(eList);
        m_writableMessageLists.append(0);
        m_groupList.append(0);
    }
    m_messageLists.append(mList);
    m_writableMessageLists.append(writable ? &m_messageLists.last() : 0);
    m_groupList.append(groupItem);
}

void MultiGroupItem::appendEmptyModel()
{
    QList<MessageItem *> eList;
    for (int j = 0; j < messageCount(); ++j)
        eList.append(0);
    m_messageLists.append(eList);
    m_writableMessageLists.append(0);
    m_groupList.append(0);
}

void MultiGroupItem::assignLastModel(GroupItem *g, bool writable)
{
    if (writable)
        m_writableMessageLists.last() = &m_messageLists.last();
    m_groupList.last() = g;
}

// XXX this is not needed, yet
void MultiGroupItem::moveModel(int oldPos, int newPos)
{
    m_groupList.insert(newPos, m_groupList[oldPos]);
    m_messageLists.insert(newPos, m_messageLists[oldPos]);
    m_writableMessageLists.insert(newPos, m_writableMessageLists[oldPos]);
    removeModel(oldPos < newPos ? oldPos : oldPos + 1);
}

void MultiGroupItem::removeModel(int pos)
{
    m_groupList.removeAt(pos);
    m_messageLists.removeAt(pos);
    m_writableMessageLists.removeAt(pos);
}

void MultiGroupItem::putMessageItem(int pos, MessageItem *m)
{
    m_messageLists.last()[pos] = m;
}

void MultiGroupItem::appendMessageItems(const QList<MessageItem *> &m)
{
    QList<MessageItem *> nullItems = m; // Basically, just a reservation
    for (int i = 0; i < nullItems.size(); ++i)
        nullItems[i] = 0;
    for (int i = 0; i < m_messageLists.size() - 1; ++i)
        m_messageLists[i] += nullItems;
    m_messageLists.last() += m;
    for (MessageItem *mi : m)
        m_multiMessageList.append(MultiMessageItem(mi));
}

void MultiGroupItem::removeMultiMessageItem(int pos)
{
    for (int i = 0; i < m_messageLists.size(); ++i)
        m_messageLists[i].removeAt(pos);
    m_multiMessageList.removeAt(pos);
}

int MultiGroupItem::firstNonobsoleteMessageIndex(int msgIdx) const
{
    for (int i = 0; i < m_messageLists.size(); ++i)
        if (m_messageLists[i][msgIdx] && !m_messageLists[i][msgIdx]->isObsolete())
            return i;
    return -1;
}

int MultiGroupItem::findMessage(const QString &sourcetext, const QString &comment) const
{
    for (int i = 0, cnt = messageCount(); i < cnt; ++i) {
        MultiMessageItem *m = multiMessageItem(i);
        if (m->text() == sourcetext && m->comment() == comment)
            return i;
    }
    return -1;
}

int MultiGroupItem::findMessageById(const QString &id) const
{
    for (int i = 0, cnt = messageCount(); i < cnt; ++i) {
        MultiMessageItem *m = multiMessageItem(i);
        if (m->id() == id)
            return i;
    }
    return -1;
}

/******************************************************************************
 *
 * MultiDataModel
 *
 *****************************************************************************/

static const QColor lightPaletteColors[7]{
    QColor(210, 235, 250), // blue
    QColor(210, 250, 220), // green
    QColor(250, 240, 210), // yellow
    QColor(210, 250, 250), // cyan
    QColor(250, 230, 200), // orange
    QColor(250, 210, 210), // red
    QColor(235, 210, 250), // purple
};

static const QColor darkPaletteColors[7] = {
    QColor(60, 80, 100), // blue
    QColor(50, 90, 70), // green
    QColor(100, 90, 50), // yellow
    QColor(50, 90, 90), // cyan
    QColor(100, 70, 50), // orange
    QColor(90, 50, 50), // red
    QColor(70, 50, 90), // purple
};

MultiDataModel::MultiDataModel(QObject *parent) :
    QObject(parent),
    m_numFinished(0),
    m_numEditable(0),
    m_numMessages(0),
    m_modified(false)
{
    updateColors();

    m_bitmap = QBitmap(8, 8);
    m_bitmap.clear();
    QPainter p(&m_bitmap);
    for (int j = 0; j < 8; ++j)
        for (int k = 0; k < 8; ++k)
            if ((j + k) & 4)
                p.drawPoint(j, k);
}

MultiDataModel::~MultiDataModel()
{
    qDeleteAll(m_dataModels);
}

QBrush MultiDataModel::brushForModel(int model) const
{
    QBrush brush(m_colors[model % 7]);
    if (!isModelWritable(model))
        brush.setTexture(m_bitmap);
    return brush;
}

void MultiDataModel::updateColors()
{
    m_colors = isDarkMode() ? darkPaletteColors : lightPaletteColors;
}

bool MultiDataModel::isWellMergeable(const DataModel *dm) const
{
    if (!dm->messageCount() || !messageCount())
        return true;

    int inBothNew = 0;

    auto countInBothNew = [dm, &inBothNew, this](int count, TranslationType type) {
        for (int i = 0; i < count; ++i) {
            GroupItem *g = dm->groupItem(i, type);
            if (MultiGroupItem *mgi = findGroup(g->group(), type)) {
                for (int j = 0; j < g->messageCount(); ++j) {
                    MessageItem *m = g->messageItem(j);
                    // During merging, when calculating the well-mergeability ratio,
                    // we reward ID-based messages with the same IDs for having the same label.
                    // This is not a strict requirement, since linguists can still represent
                    // merging of the messages with the same ID and different labels reasonably.
                    // However, if too many messages share the same ID but have different labels,
                    // merging them provides little value and may reduce user convenience.
                    if ((type == TEXTBASED && mgi->findMessage(m->text(), m->comment()) >= 0)
                        || (type == IDBASED && mgi->findMessageById(m->id()) >= 0))
                        ++inBothNew;
                }
            }
        }
    };

    countInBothNew(dm->contextCount(), TEXTBASED);
    countInBothNew(dm->labelCount(), IDBASED);

    int newRatio = inBothNew * 100 / dm->messageCount();
    int inBothOld = 0;

    auto countInBothOld = [this, dm, &inBothOld](int count, TranslationType type) {
        for (int k = 0; k < count; ++k) {
            MultiGroupItem *mgi = multiGroupItem(k, type);
            if (GroupItem *g = dm->findGroup(mgi->group(), type)) {
                for (int j = 0; j < mgi->messageCount(); ++j) {
                    MultiMessageItem *m = mgi->multiMessageItem(j);
                    if ((type == TEXTBASED && g->findMessage(m->text(), m->comment()))
                        || (type == IDBASED && g->findMessageById(m->id())))
                        ++inBothOld;
                }
            }
        }
    };

    countInBothOld(contextCount(), TEXTBASED);
    countInBothOld(labelCount(), IDBASED);

    int oldRatio = inBothOld * 100 / messageCount();

    return newRatio + oldRatio > 90;
}

void MultiDataModel::append(DataModel *dm, bool readWrite)
{
    int insCol = modelCount() + 1;
    m_dataModels.append(dm);

    auto appendGroups = [this, dm, readWrite, insCol](TranslationType type, MessageModel *msgModel,
                                                      QList<MultiGroupItem> &multiGroupList) {
        qsizetype count = type == IDBASED ? labelCount() : contextCount();
        msgModel->beginInsertColumns(QModelIndex(), insCol, insCol);
        for (int j = 0; j < count; ++j) {
            msgModel->beginInsertColumns(msgModel->createIndex(j, 0), insCol, insCol);
            multiGroupList[j].appendEmptyModel();
            msgModel->endInsertColumns();
        }
        msgModel->endInsertColumns();
        count = type == IDBASED ? dm->labelCount() : dm->contextCount();
        int appendedGroups = 0;
        for (int i = 0; i < count; ++i) {
            GroupItem *g = dm->groupItem(i, type);
            int gidx = findGroupIndex(g->group(), type);
            if (gidx >= 0) {
                MultiGroupItem *mgi = multiGroupItem(gidx, type);
                mgi->assignLastModel(g, readWrite);
                QList<MessageItem *> appendItems;
                for (int j = 0; j < g->messageCount(); ++j) {
                    MessageItem *m = g->messageItem(j);

                    int msgIdx = type == IDBASED ? mgi->findMessageById(m->id())
                                                 : mgi->findMessage(m->text(), m->comment());

                    if (msgIdx >= 0)
                        mgi->putMessageItem(msgIdx, m);
                    else
                        appendItems << m;
                }
                if (!appendItems.isEmpty()) {
                    int msgCnt = mgi->messageCount();
                    msgModel->beginInsertRows(msgModel->createIndex(gidx, 0), msgCnt,
                                              msgCnt + appendItems.size() - 1);
                    mgi->appendMessageItems(appendItems);
                    msgModel->endInsertRows();
                    m_numMessages += appendItems.size();
                }
            } else {
                multiGroupList << MultiGroupItem(modelCount() - 1, g, readWrite);
                m_numMessages += g->messageCount();
                ++appendedGroups;
            }
        }
        if (appendedGroups) {
            // Do that en block to avoid itemview inefficiency. It doesn't hurt that we
            // announce the availability of the data "long" after it was actually added.
            const qsizetype groupCount = type == IDBASED ? labelCount() : contextCount();
            msgModel->beginInsertRows(QModelIndex(), groupCount - appendedGroups, groupCount - 1);
            msgModel->endInsertRows();
        }
    };

    appendGroups(TEXTBASED, m_textBasedMsgModel, m_multiContextList);
    appendGroups(IDBASED, m_idBasedMsgModel, m_multiLabelList);

    dm->setWritable(readWrite);
    updateCountsOnAdd(modelCount() - 1, readWrite);
    connect(dm, &DataModel::modifiedChanged,
            this, &MultiDataModel::onModifiedChanged);
    connect(dm, &DataModel::languageChanged,
            this, &MultiDataModel::onLanguageChanged);
    connect(dm, &DataModel::statsChanged,
            this, &MultiDataModel::statsChanged);
    emit modelAppended();
}

void MultiDataModel::close(int model)
{
    if (m_dataModels.size() == 1) {
        closeAll();
    } else {
        int delCol = model + 1;
        auto removeModel = [delCol, model](auto *msgModel, auto &list) {
            msgModel->beginRemoveColumns(QModelIndex(), delCol, delCol);
            for (int i = list.size(); --i >= 0;) {
                msgModel->beginRemoveColumns(msgModel->createIndex(i, 0), delCol, delCol);
                list[i].removeModel(model);
                msgModel->endRemoveColumns();
            }
            msgModel->endRemoveColumns();
        };

        updateCountsOnRemove(model, isModelWritable(model));
        removeModel(m_idBasedMsgModel, m_multiLabelList);
        removeModel(m_textBasedMsgModel, m_multiContextList);
        delete m_dataModels.takeAt(model);
        emit modelDeleted(model);

        auto removeMessages = [this](auto *msgModel, auto &list) {
            for (int i = list.size(); --i >= 0;) {
                auto &mi = list[i];
                QModelIndex idx = msgModel->createIndex(i, 0);
                for (int j = mi.messageCount(); --j >= 0;)
                    if (mi.multiMessageItem(j)->isEmpty()) {
                        msgModel->beginRemoveRows(idx, j, j);
                        mi.removeMultiMessageItem(j);
                        msgModel->endRemoveRows();
                        --m_numMessages;
                    }
                if (!mi.messageCount()) {
                    msgModel->beginRemoveRows(QModelIndex(), i, i);
                    list.removeAt(i);
                    msgModel->endRemoveRows();
                }
            }
        };

        removeMessages(m_idBasedMsgModel, m_multiLabelList);
        removeMessages(m_textBasedMsgModel, m_multiContextList);
        onModifiedChanged();
    }
}

void MultiDataModel::closeAll()
{
    m_idBasedMsgModel->beginResetModel();
    m_textBasedMsgModel->beginResetModel();
    m_numFinished = 0;
    m_numEditable = 0;
    m_numMessages = 0;
    qDeleteAll(m_dataModels);
    m_dataModels.clear();
    m_multiContextList.clear();
    m_multiLabelList.clear();
    m_textBasedMsgModel->endResetModel();
    m_idBasedMsgModel->endResetModel();
    emit allModelsDeleted();
    onModifiedChanged();
}

// XXX this is not needed, yet
void MultiDataModel::moveModel(int oldPos, int newPos)
{
    int delPos = oldPos < newPos ? oldPos : oldPos + 1;
    m_dataModels.insert(newPos, m_dataModels[oldPos]);
    m_dataModels.removeAt(delPos);
    for (int i = 0; i < m_multiContextList.size(); ++i)
        m_multiContextList[i].moveModel(oldPos, newPos);
    for (int i = 0; i < m_multiLabelList.size(); ++i)
        m_multiLabelList[i].moveModel(oldPos, newPos);
}

QStringList MultiDataModel::prettifyFileNames(const QStringList &names)
{
    QStringList out;

    for (const QString &name : names)
        out << DataModel::prettifyFileName(name);
    return out;
}

QString MultiDataModel::condenseFileNames(const QStringList &names)
{
    if (names.isEmpty())
        return QString();

    if (names.size() < 2)
        return names.first();

    QString prefix = names.first();
    if (prefix.startsWith(u'='))
        prefix.remove(0, 1);
    QString suffix = prefix;
    for (int i = 1; i < names.size(); ++i) {
        QString fn = names[i];
        if (fn.startsWith(u'='))
            fn.remove(0, 1);
        for (int j = 0; j < prefix.size(); ++j)
            if (fn[j] != prefix[j]) {
                if (j < prefix.size()) {
                    while (j > 0 && prefix[j - 1].isLetterOrNumber())
                        --j;
                    prefix.truncate(j);
                }
                break;
            }
        int fnl = fn.size() - 1;
        int sxl = suffix.size() - 1;
        for (int k = 0; k <= sxl; ++k)
            if (fn[fnl - k] != suffix[sxl - k]) {
                if (k < sxl) {
                    while (k > 0 && suffix[sxl - k + 1].isLetterOrNumber())
                        --k;
                    if (prefix.size() + k > fnl)
                        --k;
                    suffix.remove(0, sxl - k + 1);
                }
                break;
            }
    }
    QString ret = prefix + u'{';
    int pxl = prefix.size();
    int sxl = suffix.size();
    for (int j = 0; j < names.size(); ++j) {
        if (j)
            ret += u',';
        int off = pxl;
        QString fn = names[j];
        if (fn.startsWith(u'=')) {
            ret += u'=';
            ++off;
        }
        ret += fn.mid(off, fn.size() - sxl - off);
    }
    ret += u'}' + suffix;
    return ret;
}

QStringList MultiDataModel::srcFileNames(bool pretty) const
{
    QStringList names;
    for (DataModel *dm : m_dataModels)
        names << (dm->isWritable() ? QString() : QString::fromLatin1("=")) + dm->srcFileName(pretty);
    return names;
}

QString MultiDataModel::condensedSrcFileNames(bool pretty) const
{
    return condenseFileNames(srcFileNames(pretty));
}

MultiMessageItem *MultiDataModel::multiMessageItem(const MultiDataIndex &index) const
{
    return multiGroupItem(index)->multiMessageItem(index.message());
}

bool MultiDataModel::isModified() const
{
    for (const DataModel *mdl : m_dataModels)
        if (mdl->isModified())
            return true;
    return false;
}

void MultiDataModel::onModifiedChanged()
{
    bool modified = isModified();
    if (modified != m_modified) {
        emit modifiedChanged(modified);
        m_modified = modified;
    }
}

void MultiDataModel::onLanguageChanged()
{
    int i = 0;
    while (sender() != m_dataModels[i])
        ++i;
    emit languageChanged(i);
}

GroupItem *MultiDataModel::groupItem(const MultiDataIndex &index) const
{
    return multiGroupItem(index)->groupItem(index.model());
}

int MultiDataModel::isFileLoaded(const QString &name) const
{
    for (int i = 0; i < m_dataModels.size(); ++i)
        if (m_dataModels[i]->srcFileName() == name)
            return i;
    return -1;
}

int MultiDataModel::findGroupIndex(const QString &group, TranslationType type) const
{
    const auto &list = type == IDBASED ? m_multiLabelList : m_multiContextList;
    for (int i = 0; i < list.size(); ++i) {
        const MultiGroupItem &mg = list[i];
        if (mg.group() == group)
            return i;
    }
    return -1;
}

MultiGroupItem *MultiDataModel::findGroup(const QString &group, TranslationType type) const
{
    const auto &list = type == IDBASED ? m_multiLabelList : m_multiContextList;
    for (int i = 0; i < list.size(); ++i) {
        const MultiGroupItem &mgi = list[i];
        if (mgi.group() == group)
            return const_cast<MultiGroupItem *>(&mgi);
    }
    return 0;
}

MultiGroupItem *MultiDataModel::multiGroupItem(const MultiDataIndex &index) const
{
    const auto &list = index.isIdBased() ? m_multiLabelList : m_multiContextList;
    return const_cast<MultiGroupItem *>(&list[index.group()]);
}

MessageItem *MultiDataModel::messageItem(const MultiDataIndex &index, int model) const
{
    qsizetype groupCount = index.isIdBased() ? labelCount() : contextCount();
    if (index.group() < groupCount && index.group() >= 0 && model >= 0 && model < modelCount()) {
        MultiGroupItem *mgi = multiGroupItem(index);
        if (index.message() < mgi->messageCount())
            return mgi->messageItem(model, index.message());
    }
    Q_ASSERT(model >= 0 && model < modelCount());
    Q_ASSERT(index.group() < groupCount);
    return 0;
}

void MultiDataModel::setTranslation(const MultiDataIndex &index, const QString &translation)
{
    MessageItem *m = messageItem(index);
    if (translation == m->translation())
        return;
    m->setTranslation(translation);
    setModified(index.model(), true);
    emit translationChanged(index);
}

void MultiDataModel::setFinished(const MultiDataIndex &index, bool finished)
{
    MultiGroupItem *mgi = multiGroupItem(index);
    MultiMessageItem *mm = mgi->multiMessageItem(index.message());
    GroupItem *gi = groupItem(index);
    MessageItem *m = messageItem(index);
    TranslatorMessage::Type type = m->type();
    if (type == TranslatorMessage::Unfinished && finished) {
        m->setType(TranslatorMessage::Finished);
        mm->decrementUnfinishedCount();
        if (!mm->countUnfinished()) {
            incrementFinishedCount();
            mgi->incrementFinishedCount();
            emit multiGroupDataChanged(index);
        }
        gi->incrementFinishedCount();
        if (m->danger()) {
            gi->incrementFinishedDangerCount();
            gi->decrementUnfinishedDangerCount();
            if (!gi->unfinishedDangerCount() || gi->finishedCount() == gi->nonobsoleteCount())
                emit groupDataChanged(index);
        } else if (gi->finishedCount() == gi->nonobsoleteCount()) {
            emit groupDataChanged(index);
        }
        emit messageDataChanged(index);
        setModified(index.model(), true);
    } else if (type == TranslatorMessage::Finished && !finished) {
        m->setType(TranslatorMessage::Unfinished);
        mm->incrementUnfinishedCount();
        if (mm->countUnfinished() == 1) {
            decrementFinishedCount();
            mgi->decrementFinishedCount();
            emit multiGroupDataChanged(index);
        }
        gi->decrementFinishedCount();
        if (m->danger()) {
            gi->decrementFinishedDangerCount();
            gi->incrementUnfinishedDangerCount();
            if (gi->unfinishedDangerCount() == 1
                || gi->finishedCount() + 1 == gi->nonobsoleteCount())
                emit groupDataChanged(index);
        } else if (gi->finishedCount() + 1 == gi->nonobsoleteCount()) {
            emit groupDataChanged(index);
        }
        emit messageDataChanged(index);
        setModified(index.model(), true);
    }
}

void MultiDataModel::setDanger(const MultiDataIndex &index, bool danger)
{
    GroupItem *gi = groupItem(index);
    MessageItem *m = messageItem(index);
    if (!m->danger() && danger) {
        if (m->isFinished()) {
            gi->incrementFinishedDangerCount();
            if (gi->finishedDangerCount() == 1)
                emit groupDataChanged(index);
        } else {
            gi->incrementUnfinishedDangerCount();
            if (gi->unfinishedDangerCount() == 1)
                emit groupDataChanged(index);
        }
        emit messageDataChanged(index);
        m->setDanger(danger);
    } else if (m->danger() && !danger) {
        if (m->isFinished()) {
            gi->decrementFinishedDangerCount();
            if (!gi->finishedDangerCount())
                emit groupDataChanged(index);
        } else {
            gi->decrementUnfinishedDangerCount();
            if (!gi->unfinishedDangerCount())
                emit groupDataChanged(index);
        }
        emit messageDataChanged(index);
        m->setDanger(danger);
    }
}

void MultiDataModel::updateCountsOnAdd(int model, bool writable)
{
    auto updateCount = [model, writable, this](auto &mg) {
        for (int j = 0; j < mg.messageCount(); ++j)
            if (MessageItem *m = mg.messageItem(model, j)) {
                MultiMessageItem *mm = mg.multiMessageItem(j);
                mm->incrementNonnullCount();
                if (!m->isObsolete()) {
                    if (writable) {
                        if (!mm->countEditable()) {
                            mg.incrementEditableCount();
                            incrementEditableCount();
                            if (m->isFinished()) {
                                mg.incrementFinishedCount();
                                incrementFinishedCount();
                            } else {
                                mm->incrementUnfinishedCount();
                            }
                        } else if (!m->isFinished()) {
                            if (!mm->isUnfinished()) {
                                mg.decrementFinishedCount();
                                decrementFinishedCount();
                            }
                            mm->incrementUnfinishedCount();
                        }
                        mm->incrementEditableCount();
                    }
                    mg.incrementNonobsoleteCount();
                    mm->incrementNonobsoleteCount();
                }
            }
    };
    for (auto &mg : m_multiContextList)
        updateCount(mg);
    for (auto &mg : m_multiLabelList)
        updateCount(mg);
}

void MultiDataModel::updateCountsOnRemove(int model, bool writable)
{
    auto updateCount = [model, writable, this](auto &mg) {
        for (int j = 0; j < mg.messageCount(); ++j)
            if (MessageItem *m = mg.messageItem(model, j)) {
                MultiMessageItem *mm = mg.multiMessageItem(j);
                mm->decrementNonnullCount();
                if (!m->isObsolete()) {
                    mm->decrementNonobsoleteCount();
                    mg.decrementNonobsoleteCount();
                    if (writable) {
                        mm->decrementEditableCount();
                        if (!mm->countEditable()) {
                            mg.decrementEditableCount();
                            decrementEditableCount();
                            if (m->isFinished()) {
                                mg.decrementFinishedCount();
                                decrementFinishedCount();
                            } else {
                                mm->decrementUnfinishedCount();
                            }
                        } else if (!m->isFinished()) {
                            mm->decrementUnfinishedCount();
                            if (!mm->isUnfinished()) {
                                mg.incrementFinishedCount();
                                incrementFinishedCount();
                            }
                        }
                    }
                }
            }
    };
    for (auto &mg : m_multiContextList)
        updateCount(mg);
    for (auto &mg : m_multiLabelList)
        updateCount(mg);
}

/******************************************************************************
 *
 * MultiDataModelIterator
 *
 *****************************************************************************/

MultiDataModelIterator::MultiDataModelIterator(TranslationType type, MultiDataModel *dataModel,
                                               int model, int group, int message)
    : MultiDataIndex(type, model, group, message), m_dataModel(dataModel)
{
}

void MultiDataModelIterator::operator++()
{
    Q_ASSERT(isValid());
    ++m_message;
    const qsizetype count = isIdBased()
            ? m_dataModel->m_multiLabelList.at(m_group).messageCount()
            : m_dataModel->m_multiContextList.at(m_group).messageCount();
    if (m_message >= count) {
        ++m_group;
        m_message = 0;
    }
}

bool MultiDataModelIterator::isValid() const
{
    const qsizetype size = isIdBased() ? m_dataModel->m_multiLabelList.size()
                                       : m_dataModel->m_multiContextList.size();
    return m_group < size;
}

MessageItem *MultiDataModelIterator::current() const
{
    return m_dataModel->messageItem(*this);
}

/******************************************************************************
 *
 * MessageModel
 *
 *****************************************************************************/

MessageModel::MessageModel(TranslationType translationType, QObject *parent, MultiDataModel *data)
    : QAbstractItemModel(parent), m_data(data), m_translationType(translationType)
{
    if (translationType == IDBASED)
        data->m_idBasedMsgModel = this;
    else
        data->m_textBasedMsgModel = this;
    connect(m_data, &MultiDataModel::multiGroupDataChanged, this,
            &MessageModel::multiGroupItemChanged);
    connect(m_data, &MultiDataModel::groupDataChanged, this, &MessageModel::groupItemChanged);
    connect(m_data, &MultiDataModel::messageDataChanged, this, &MessageModel::messageItemChanged);
}

QModelIndex MessageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column);
    if (!parent.internalId())
        return createIndex(row, column, parent.row() + 1);
    return QModelIndex();
}

QModelIndex MessageModel::parent(const QModelIndex& index) const
{
    if (index.internalId())
        return createIndex(index.internalId() - 1, 0);
    return QModelIndex();
}

void MessageModel::multiGroupItemChanged(const MultiDataIndex &index)
{
    if (index.translationType() != m_translationType)
        return;
    QModelIndex idx = createIndex(index.group(), m_data->modelCount() + 2);
    emit dataChanged(idx, idx);
}

void MessageModel::groupItemChanged(const MultiDataIndex &index)
{
    if (index.translationType() != m_translationType)
        return;
    QModelIndex idx = createIndex(index.group(), index.model() + 1);
    emit dataChanged(idx, idx);
}

void MessageModel::messageItemChanged(const MultiDataIndex &index)
{
    if (index.translationType() != m_translationType)
        return;
    QModelIndex idx = createIndex(index.message(), index.model() + 1, index.group() + 1);
    emit dataChanged(idx, idx);
}

QModelIndex MessageModel::modelIndex(const MultiDataIndex &index)
{
    if (index.message() < 0) // Should be unused case
        return createIndex(index.group(), index.model() + 1);
    return createIndex(index.message(), index.model() + 1, index.group() + 1);
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_translationType == IDBASED ? m_data->labelCount()
                                            : m_data->contextCount(); // contexts
    if (!parent.internalId()) // messages
        return m_data->multiGroupItem(parent.row(), m_translationType)->messageCount();
    return 0;
}

int MessageModel::columnCount(const QModelIndex &) const
{
    return m_data->modelCount() + 3;
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    static QVariant pxOn;
    static QVariant pxOff;
    static QVariant pxObsolete;
    static QVariant pxDanger;
    static QVariant pxWarning;
    static QVariant pxEmpty;

    static Qt::ColorScheme mode = Qt::ColorScheme::Unknown; // to prevent creating new QPixmaps
                                                            // every time the method is called

    if (bool dark = isDarkMode();
        (dark && mode != Qt::ColorScheme::Dark) || (!dark && mode != Qt::ColorScheme::Light)) {
        pxOn = createMarkIcon(TranslationMarks::OnMark, dark);
        pxOff = createMarkIcon(TranslationMarks::OffMark, dark);
        pxObsolete = createMarkIcon(TranslationMarks::ObsoleteMark, dark);
        pxDanger = createMarkIcon(TranslationMarks::DangerMark, dark);
        pxWarning = createMarkIcon(TranslationMarks::WarningMark, dark);
        pxEmpty = createMarkIcon(TranslationMarks::EmptyMark, dark);
        mode = dark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light;
    }

    int row = index.row();
    int column = index.column() - 1;

    if (column < 0)
        return QVariant();

    int numLangs = m_data->modelCount();

    if (role == Qt::ToolTipRole && column < numLangs) {
        return tr("Completion status for %1").arg(m_data->model(column)->localizedLanguage());
    } else if (index.internalId()) {
        // this is a message
        int crow = index.internalId() - 1;
        MultiGroupItem *mgi = m_data->multiGroupItem(crow, m_translationType);
        if (row >= mgi->messageCount() || !index.isValid())
            return QVariant();

        if (role == Qt::DisplayRole || (role == Qt::ToolTipRole && column == numLangs)) {
            switch (column - numLangs) {
            case 0: // Source text
            {
                if (m_translationType == IDBASED)
                    return mgi->multiMessageItem(row)->id();
                else if (const QString text = mgi->multiMessageItem(row)->text(); !text.isEmpty())
                    return text.simplified();
                else
                    return tr("<context comment>");
            }
            case 1:
                if (m_translationType == IDBASED)
                    return mgi->multiMessageItem(row)->text();
            default: // Status or dummy column => no text
                return QVariant();
            }
        }
        else if (role == Qt::DecorationRole && column < numLangs) {
            if (MessageItem *msgItem = mgi->messageItem(column, row)) {
                switch (msgItem->message().type()) {
                case TranslatorMessage::Unfinished:
                    if (msgItem->translation().isEmpty())
                        return pxEmpty;
                    if (msgItem->danger())
                        return pxDanger;
                    return pxOff;
                case TranslatorMessage::Finished:
                    if (msgItem->danger())
                        return pxWarning;
                    return pxOn;
                default:
                    return pxObsolete;
                }
            }
            return QVariant();
        }
        else if (role == SortRole) {
            switch (column - numLangs) {
            case 0: // Source text
                return mgi->multiMessageItem(row)->text().simplified().remove(u'&');
            case 1: // Dummy column
                return QVariant();
            default:
                if (MessageItem *msgItem = mgi->messageItem(column, row)) {
                    int rslt = !msgItem->translation().isEmpty();
                    if (!msgItem->danger())
                        rslt |= 2;
                    if (msgItem->isObsolete())
                        rslt |= 8;
                    else if (msgItem->isFinished())
                        rslt |= 4;
                    return rslt;
                }
                return INT_MAX;
            }
        } else if (role == Qt::ForegroundRole && column > 0
                   && mgi->multiMessageItem(row)->isObsolete()) {
            return QBrush(Qt::darkGray);
        } else if (role == Qt::ForegroundRole && column == numLangs
                   && mgi->multiMessageItem(row)->text().isEmpty()) {
            return QBrush(QColor(0, 0xa0, 0xa0));
        } else if (role == Qt::BackgroundRole) {
            if (column < numLangs && numLangs != 1)
                return m_data->brushForModel(column);
        }
    } else {
        // this is a context or a label
        const qsizetype groupCount =
                m_translationType == IDBASED ? m_data->labelCount() : m_data->contextCount();
        if (row >= groupCount || !index.isValid())
            return QVariant();

        MultiGroupItem *mgi = m_data->multiGroupItem(row, m_translationType);
        if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            switch (column - numLangs) {
            case 0: // Context/Label
            {
                const QString groupName = mgi->group().simplified();
                if (m_translationType == IDBASED and groupName.isEmpty()) {
                    return tr("<unnamed label>");
                }
                return groupName;
            }
            case 1: {
                if (role == Qt::ToolTipRole) {
                    return tr("%n unfinished message(s) left.", 0,
                              mgi->getNumEditable() - mgi->getNumFinished());
                }
                return QString::asprintf("%d/%d", mgi->getNumFinished(), mgi->getNumEditable());
            }
            default:
                return QVariant(); // Status => no text
            }
        }
        else if (role == Qt::FontRole && column == m_data->modelCount()) {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        else if (role == Qt::DecorationRole && column < numLangs) {
            if (GroupItem *groupItem = mgi->groupItem(column)) {
                if (groupItem->isObsolete())
                    return pxObsolete;
                if (groupItem->isFinished())
                    return groupItem->finishedDangerCount() > 0 ? pxWarning : pxOn;
                return groupItem->unfinishedDangerCount() > 0 ? pxDanger : pxOff;
            }
            return QVariant();
        }
        else if (role == SortRole) {
            switch (column - numLangs) {
            case 0: // Context/Label (same as display role)
                return mgi->group().simplified();
            case 1: // Items
                return mgi->getNumEditable();
            default: // Percent
                if (GroupItem *groupItem = mgi->groupItem(column)) {
                    int totalItems = groupItem->nonobsoleteCount();
                    int percent =
                            totalItems ? (100 * groupItem->finishedCount()) / totalItems : 100;
                    int rslt = percent * (((1 << 28) - 1) / 100) + totalItems;
                    if (groupItem->isObsolete()) {
                        rslt |= (1 << 30);
                    } else if (groupItem->isFinished()) {
                        rslt |= (1 << 29);
                        if (!groupItem->finishedDangerCount())
                            rslt |= (1 << 28);
                    } else {
                        if (!groupItem->unfinishedDangerCount())
                            rslt |= (1 << 28);
                    }
                    return rslt;
                }
                return INT_MAX;
            }
        } else if (role == Qt::ForegroundRole && column >= numLangs && mgi->isObsolete()) {
            return QBrush(Qt::darkGray);
        } else if (role == Qt::ForegroundRole && column == numLangs
                   && m_translationType == IDBASED) {
            return QBrush(QColor(0, 0xa0, 0xa0));
        } else if (role == Qt::BackgroundRole) {
            if (column < numLangs && numLangs != 1) {
                QBrush brush = m_data->brushForModel(column);
                if (row & 1) {
                    brush.setColor(brush.color().darker(108));
                }
                return brush;
            }
        }
    }
    return QVariant();
}

MultiDataIndex MessageModel::dataIndex(const QModelIndex &index, int model) const
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalId());
    return MultiDataIndex(m_translationType, model, index.internalId() - 1, index.row());
}

QT_END_NAMESPACE
