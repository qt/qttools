// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include "translator.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QLocale>
#include <QtGui/QColor>
#include <QtGui/QBitmap>

QT_BEGIN_NAMESPACE

class DataModel;
class MultiDataModel;
struct StatisticalData;

class MessageItem
{
public:
    MessageItem(const TranslatorMessage &message);

    bool danger() const { return m_danger; }
    void setDanger(bool danger) { m_danger = danger; }
    bool ncrMode() const { return m_ncrMode; }
    void setNcrMode(bool mode) { m_ncrMode = mode; }

    void setTranslation(const QString &translation);

    QString id() const { return m_message.id(); }
    QString context() const { return m_message.context(); }
    QString label() const { return m_message.label(); }
    QString text() const;
    QString pluralText() const;
    QString comment() const { return m_message.comment(); }
    QString fileName() const { return m_message.fileName(); }
    QString extraComment() const { return m_message.extraComment(); }
    QString translatorComment() const { return m_message.translatorComment(); }
    void setTranslatorComment(const QString &cmt) { m_message.setTranslatorComment(cmt); }
    int lineNumber() const { return m_message.lineNumber(); }
    QString translation() const;
    QStringList translations() const;
    void setTranslations(const QStringList &translations);

    TranslatorMessage::Type type() const { return m_message.type(); }
    void setType(TranslatorMessage::Type type) { m_message.setType(type); }

    bool isFinished() const { return type() == TranslatorMessage::Finished; }
    bool isUnfinished() const { return type() == TranslatorMessage::Unfinished; }
    bool isObsolete() const
        { return type() == TranslatorMessage::Obsolete || type() == TranslatorMessage::Vanished; }
    const TranslatorMessage &message() const { return m_message; }

    bool compare(const QString &findText, bool matchSubstring,
        Qt::CaseSensitivity cs) const;

private:
    TranslatorMessage m_message;
    bool m_danger;
    bool m_ncrMode;
};

enum TranslationType { IDBASED, TEXTBASED };

class GroupItem
{
public:
    GroupItem(TranslationType type, const QString &group) : m_group(group), m_translationType(type)
    {
    }

    int finishedDangerCount() const { return m_finishedDangerCount; }
    int unfinishedDangerCount() const { return m_unfinishedDangerCount; }

    int finishedCount() const { return m_finishedCount; }
    int unfinishedCount() const { return m_nonobsoleteCount - m_finishedCount; }
    int nonobsoleteCount() const { return m_nonobsoleteCount; }

    QString group() const { return m_group; }
    TranslationType translationType() const { return m_translationType; }
    QString comment() const { return m_comment; }
    QString fullContext() const { return m_comment.trimmed(); }

    // For item status in context list
    bool isObsolete() const { return !nonobsoleteCount(); }
    bool isFinished() const { return unfinishedCount() == 0; }

    MessageItem *messageItem(int i) const;
    int messageCount() const { return msgItemList.size(); }

    MessageItem *findMessage(const QString &sourcetext, const QString &comment) const;
    MessageItem *findMessageById(const QString &msgid) const;

private:
    friend class DataModel;
    friend class MultiDataModel;
    void appendMessage(const MessageItem &msg) { msgItemList.append(msg); }
    void appendToComment(const QString &x);
    void incrementFinishedCount() { ++m_finishedCount; }
    void decrementFinishedCount() { --m_finishedCount; }
    void incrementFinishedDangerCount() { ++m_finishedDangerCount; }
    void decrementFinishedDangerCount() { --m_finishedDangerCount; }
    void incrementUnfinishedDangerCount() { ++m_unfinishedDangerCount; }
    void decrementUnfinishedDangerCount() { --m_unfinishedDangerCount; }
    void incrementNonobsoleteCount() { ++m_nonobsoleteCount; }

    QString m_comment;
    QString m_group;
    TranslationType m_translationType;
    int m_finishedCount = 0;
    int m_finishedDangerCount = 0;
    int m_unfinishedDangerCount = 0;
    int m_nonobsoleteCount = 0;
    QList<MessageItem> msgItemList;
};

class DataIndex
{
public:
    DataIndex(TranslationType type, int group = -1, int message = -1)
        : m_group(group), m_message(message), m_type(type)
    {
    }
    int group() const { return m_group; }
    int message() const { return m_message; }
    bool isValid() const { return m_group >= 0; }
    bool isIdBased() const { return m_type == IDBASED; }
    TranslationType translationType() const { return m_type; }

protected:
    int m_group;
    int m_message;
    TranslationType m_type;
};

class DataModelIterator : public DataIndex
{
public:
    DataModelIterator(TranslationType type, const DataModel *model = 0, int groupNo = 0,
                      int messageNo = 0);
    MessageItem *current() const;
    bool isValid() const;
    void operator++();

private:
    const DataModel *m_model; // not owned
};

class DataModel : public QObject
{
    Q_OBJECT
public:
    DataModel(QObject *parent = 0);

    enum FindLocation { NoLocation = 0, SourceText = 0x1, Translations = 0x2, Comments = 0x4 };

    // Specializations
    int contextCount() const { return m_contextList.size(); }
    int labelCount() const { return m_labelList.size(); }
    GroupItem *findGroup(const QString &group, TranslationType type) const;
    MessageItem *findMessage(const QString &context, const QString &label,
                             const QString &sourcetext, const QString &comment) const;

    GroupItem *groupItem(int index, TranslationType type) const;
    GroupItem *groupItem(DataIndex) const;
    MessageItem *messageItem(const DataIndex &index) const;

    int messageCount() const { return m_numMessages; }
    bool isEmpty() const { return m_numMessages == 0; }
    bool isModified() const { return m_modified; }
    void setModified(bool dirty);
    bool isWritable() const { return m_writable; }
    void setWritable(bool writable) { m_writable = writable; }

    bool isWellMergeable(const DataModel *other) const;
    bool load(const QString &fileName, bool *langGuessed, QWidget *parent);
    bool save(QWidget *parent) { return save(m_srcFileName, parent); }
    bool saveAs(const QString &newFileName, QWidget *parent);
    bool release(const QString &fileName, bool verbose,
        bool ignoreUnfinished, TranslatorSaveMode mode, QWidget *parent);
    QString srcFileName(bool pretty = false) const
        { return pretty ? prettifyPlainFileName(m_srcFileName) : m_srcFileName; }

    static QString prettifyPlainFileName(const QString &fn);
    static QString prettifyFileName(const QString &fn);

    bool setLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory);
    QLocale::Language language() const { return m_language; }
    QLocale::Territory territory() const { return m_territory; }
    void setSourceLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory);
    QLocale::Language sourceLanguage() const { return m_sourceLanguage; }
    QLocale::Territory sourceTerritory() const { return m_sourceTerritory; }

    const QString &localizedLanguage() const { return m_localizedLanguage; }
    const QStringList &numerusForms() const { return m_numerusForms; }
    const QList<bool> &countRefNeeds() const { return m_countRefNeeds; }

    QStringList normalizedTranslations(const MessageItem &m) const;
    void doCharCounting(const QString& text, int& trW, int& trC, int& trCS);
    void updateStatistics();

    int getSrcWords() const { return m_srcWords; }
    int getSrcChars() const { return m_srcChars; }
    int getSrcCharsSpc() const { return m_srcCharsSpc; }

signals:
    void statsChanged(const StatisticalData &newStats);
    void progressChanged(int finishedCount, int oldFinishedCount);
    void languageChanged();
    void modifiedChanged();

private:
    friend class DataModelIterator;

    QList<GroupItem> m_contextList;
    QList<GroupItem> m_labelList;

    bool save(const QString &fileName, QWidget *parent);
    void updateLocale();

    bool m_writable;
    bool m_modified;

    int m_numMessages;

    // For statistics
    int m_srcWords;
    int m_srcChars;
    int m_srcCharsSpc;

    QString m_srcFileName;
    QLocale::Language m_language;
    QLocale::Language m_sourceLanguage;
    QLocale::Territory m_territory;
    QLocale::Territory m_sourceTerritory;
    bool m_relativeLocations;
    Translator::ExtraData m_extra;

    QString m_localizedLanguage;
    QStringList m_numerusForms;
    QList<bool> m_countRefNeeds;
};

struct MultiMessageItem
{
public:
    MultiMessageItem(const MessageItem *m)
        : m_id(m->id()), m_text(m->text()), m_pluralText(m->pluralText()), m_comment(m->comment())
    {
    }
    QString id() const { return m_id; }
    QString text() const { return m_text; }
    QString pluralText() const { return m_pluralText; }
    QString comment() const { return m_comment; }
    bool isEmpty() const { return !m_nonnullCount; }
    // The next two include also read-only
    bool isObsolete() const { return m_nonnullCount && !m_nonobsoleteCount; }
    int countNonobsolete() const { return m_nonobsoleteCount; }
    // The next three include only read-write
    int countEditable() const { return m_editableCount; }
    bool isUnfinished() const { return m_unfinishedCount != 0; }
    int countUnfinished() const { return m_unfinishedCount; }

private:
    friend class MultiDataModel;
    void incrementNonnullCount() { ++m_nonnullCount; }
    void decrementNonnullCount() { --m_nonnullCount; }
    void incrementNonobsoleteCount() { ++m_nonobsoleteCount; }
    void decrementNonobsoleteCount() { --m_nonobsoleteCount; }
    void incrementEditableCount() { ++m_editableCount; }
    void decrementEditableCount() { --m_editableCount; }
    void incrementUnfinishedCount() { ++m_unfinishedCount; }
    void decrementUnfinishedCount() { --m_unfinishedCount; }

    QString m_id;
    QString m_text;
    QString m_pluralText;
    QString m_comment;
    int m_nonnullCount = 0; // all
    int m_nonobsoleteCount = 0; // all
    int m_editableCount = 0; // read-write
    int m_unfinishedCount = 0; // read-write
};

struct MultiGroupItem
{
public:
    MultiGroupItem(int oldCount, GroupItem *groupItem, bool writable);

    GroupItem *groupItem(int model) const { return m_groupList[model]; }
    MultiMessageItem *multiMessageItem(int msgIdx) const
        { return const_cast<MultiMessageItem *>(&m_multiMessageList[msgIdx]); }
    MessageItem *messageItem(int model, int msgIdx) const { return m_messageLists[model][msgIdx]; }
    int firstNonobsoleteMessageIndex(int msgIdx) const;
    int findMessage(const QString &sourcetext, const QString &comment) const;
    int findMessageById(const QString &id) const;
    QString group() const { return m_group; }
    TranslationType translationType() const { return m_translationType; }
    QString comment() const { return m_comment; }
    int messageCount() const { return m_messageLists.isEmpty() ? 0 : m_messageLists[0].size(); }
    // For item count in context list
    int getNumFinished() const { return m_finishedCount; }
    int getNumEditable() const { return m_editableCount; }
    // For background in context list
    bool isObsolete() const { return messageCount() && !m_nonobsoleteCount; }

private:
    friend class MultiDataModel;
    void appendEmptyModel();
    void assignLastModel(GroupItem *g, bool writable);
    void removeModel(int pos);
    void moveModel(int oldPos, int newPos); // newPos is *before* removing at oldPos
    void putMessageItem(int pos, MessageItem *m);
    void appendMessageItems(const QList<MessageItem *> &m);
    void removeMultiMessageItem(int pos);
    void incrementFinishedCount() { ++m_finishedCount; }
    void decrementFinishedCount() { --m_finishedCount; }
    void incrementEditableCount() { ++m_editableCount; }
    void decrementEditableCount() { --m_editableCount; }
    void incrementNonobsoleteCount() { ++m_nonobsoleteCount; }
    void decrementNonobsoleteCount() { --m_nonobsoleteCount; }

    QString m_group;
    QString m_comment;
    TranslationType m_translationType;
    QList<MultiMessageItem> m_multiMessageList;
    QList<GroupItem *> m_groupList;
    // The next two could be in the MultiMessageItems, but are here for efficiency
    QList<QList<MessageItem *> > m_messageLists;
    QList<QList<MessageItem *> *> m_writableMessageLists;
    int m_finishedCount = 0; // read-write
    int m_editableCount = 0; // read-write
    int m_nonobsoleteCount = 0; // all (note: this counts messages, not multi-messages)
};

class MultiDataIndex : public DataIndex
{
public:
    MultiDataIndex(TranslationType type = TEXTBASED, int model = -1, int group = -1,
                   int message = -1)
        : DataIndex(type, group, message), m_model(model)
    {
    }
    void setModel(int model) { m_model = model; }
    int model() const { return m_model; }
    bool operator==(const MultiDataIndex &other) const
    {
        return m_model == other.m_model && m_group == other.m_group && m_message == other.m_message;
    }
    bool operator!=(const MultiDataIndex &other) const { return !(*this == other); }

private:
    int m_model = -1;
};

class MultiDataModelIterator : public MultiDataIndex
{
public:
    MultiDataModelIterator(TranslationType type, MultiDataModel *model = 0, int modelNo = -1,
                           int groupNo = 0, int messageNo = 0);
    MessageItem *current() const;
    bool isValid() const;
    void operator++();

private:
    MultiDataModel *m_dataModel; // not owned
};

class MessageModel;

class MultiDataModel : public QObject
{
    Q_OBJECT

public:
    MultiDataModel(QObject *parent = 0);
    ~MultiDataModel();

    bool isWellMergeable(const DataModel *dm) const;
    void append(DataModel *dm, bool readWrite);
    bool save(int model, QWidget *parent) { return m_dataModels[model]->save(parent); }
    bool saveAs(int model, const QString &newFileName, QWidget *parent)
        { return m_dataModels[model]->saveAs(newFileName, parent); }
    bool release(int model, const QString &fileName, bool verbose, bool ignoreUnfinished, TranslatorSaveMode mode, QWidget *parent)
        { return m_dataModels[model]->release(fileName, verbose, ignoreUnfinished, mode, parent); }
    void close(int model);
    void closeAll();
    int isFileLoaded(const QString &name) const;
    void moveModel(int oldPos, int newPos); // newPos is *before* removing at oldPos; note that this does not emit update signals

    // Entire multi-model
    int modelCount() const { return m_dataModels.size(); }
    int labelCount() const { return m_multiLabelList.size(); }
    int contextCount() const { return m_multiContextList.size(); }
    int messageCount() const { return m_numMessages; }
    // Next two needed for progress indicator in main window
    int getNumFinished() const { return m_numFinished; }
    int getNumEditable() const { return m_numEditable; }
    bool isModified() const;
    QStringList srcFileNames(bool pretty = false) const;
    QString condensedSrcFileNames(bool pretty = false) const;

    // Per submodel
    QString srcFileName(int model, bool pretty = false) const { return m_dataModels[model]->srcFileName(pretty); }
    bool isModelWritable(int model) const { return m_dataModels[model]->isWritable(); }
    bool isModified(int model) const { return m_dataModels[model]->isModified(); }
    void setModified(int model, bool dirty) { m_dataModels[model]->setModified(dirty); }
    QLocale::Language language(int model) const { return m_dataModels[model]->language(); }
    QLocale::Language sourceLanguage(int model) const { return m_dataModels[model]->sourceLanguage(); }

    // Per message
    void setTranslation(const MultiDataIndex &index, const QString &translation);
    void setFinished(const MultiDataIndex &index, bool finished);
    void setDanger(const MultiDataIndex &index, bool danger);

    // Retrieve items
    DataModel *model(int i) { return m_dataModels[i]; }

    MultiGroupItem *multiGroupItem(const MultiDataIndex &index) const;
    MultiGroupItem *multiGroupItem(int idx, TranslationType type) const
    {
        const auto &list = type == IDBASED ? m_multiLabelList : m_multiContextList;
        return const_cast<MultiGroupItem *>(&list[idx]);
    }

    MultiMessageItem *multiMessageItem(const MultiDataIndex &index) const;
    MessageItem *messageItem(const MultiDataIndex &index, int model) const;
    MessageItem *messageItem(const MultiDataIndex &index) const
    {
        return messageItem(index, index.model());
    }

    int findGroupIndex(const QString &group, TranslationType type) const;
    MultiGroupItem *findGroup(const QString &group, TranslationType type) const;

    static QString condenseFileNames(const QStringList &names);
    static QStringList prettifyFileNames(const QStringList &names);

    QBrush brushForModel(int model) const;
    void updateColors();

signals:
    void modelAppended();
    void modelDeleted(int model);
    void allModelsDeleted();
    void languageChanged(int model);
    void statsChanged(const StatisticalData &newStats);
    void modifiedChanged(bool);
    void multiGroupDataChanged(const MultiDataIndex &index);
    void groupDataChanged(const MultiDataIndex &index);
    void messageDataChanged(const MultiDataIndex &index);
    void translationChanged(const MultiDataIndex &index); // Only the primary one

private slots:
    void onModifiedChanged();
    void onLanguageChanged();

private:
    friend class MultiDataModelIterator;
    friend class MessageModel;

    GroupItem *groupItem(const MultiDataIndex &index) const;

    void updateCountsOnAdd(int model, bool writable);
    void updateCountsOnRemove(int model, bool writable);
    void incrementFinishedCount() { ++m_numFinished; }
    void decrementFinishedCount() { --m_numFinished; }
    void incrementEditableCount() { ++m_numEditable; }
    void decrementEditableCount() { --m_numEditable; }

    int m_numFinished;
    int m_numEditable;
    int m_numMessages;

    bool m_modified;

    QList<MultiGroupItem> m_multiLabelList;
    QList<MultiGroupItem> m_multiContextList;
    QList<DataModel *> m_dataModels;
    MessageModel *m_textBasedMsgModel;
    MessageModel *m_idBasedMsgModel;

    QColor const *m_colors;
    QBitmap m_bitmap;
};

class MessageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum { SortRole = Qt::UserRole };
    MessageModel(TranslationType translationType, QObject *parent, MultiDataModel *data);

    // QAbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Convenience
    MultiDataIndex dataIndex(const QModelIndex &index, int model) const;
    MultiDataIndex dataIndex(const QModelIndex &index) const
    {
        return dataIndex(index,
                         index.column() - 1 < m_data->modelCount() ? index.column() - 1 : -1);
    }
    QModelIndex modelIndex(const MultiDataIndex &index);

private slots:
    void multiGroupItemChanged(const MultiDataIndex &index);
    void groupItemChanged(const MultiDataIndex &index);
    void messageItemChanged(const MultiDataIndex &index);

private:
    friend class MultiDataModel;

    MultiDataModel *m_data; // not owned
    const TranslationType m_translationType;
};

QT_END_NAMESPACE

#endif // MESSAGEMODEL_H
