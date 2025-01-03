// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "phrase.h"
#include "ui_mainwindow.h"
#include "recentfiles.h"
#include "messagemodel.h"
#include "finddialog.h"

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QLocale>

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE

class QPixmap;
class QAction;
class QDialog;
class QLabel;
class QMenu;
class QPrinter;
class QProcess;
class QIcon;
class QSortFilterProxyModel;
class QStackedWidget;
class QTableView;
class QTreeView;

class BatchTranslationDialog;
class ErrorsView;
class FocusWatcher;
class FormPreviewView;
class MessageEditor;
class PhraseView;
class SourceCodeView;
class Statistics;
class TranslateDialog;
class TranslationSettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum {PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};
    enum FindDirection {FindNext, FindPrev};

    MainWindow();
    ~MainWindow();

    bool openFiles(const QStringList &names, bool readWrite = true);
    static RecentFiles &recentFiles();
    static QString friendlyString(const QString &str);

public slots:
    void updateViewMenu();

protected:
    void readConfig();
    void writeConfig();
    void closeEvent(QCloseEvent *) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void done();
    void doneAndNext();
    void prev();
    void next();
    void recentFileActivated(QAction *action);
    void setupRecentFilesMenu();
    void open();
    void openAux();
    void saveAll();
    void save();
    void saveAs();
    void releaseAll();
    void release();
    void releaseAs();
    void print();
    void closeFile();
    bool closeAll();
    void showTranslateDialog();
    void showBatchTranslateDialog();
    void showTranslationSettings();
    void updateTranslateHit(bool &hit);
    void translate(int mode);
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook(QAction *action);
    void editPhraseBook(QAction *action);
    void printPhraseBook(QAction *action);
    void addToPhraseBook();
    void manual();
    void resetSorting();
    void about();
    void aboutQt();

    void fileAboutToShow();
    void editAboutToShow();

    void showContextDock();
    void showMessagesDock();
    void showPhrasesDock();
    void showSourceCodeDock();
    void showErrorDock();

    void setupPhrase();
    bool maybeSaveAll();
    bool maybeSave(int model);
    void updateProgress();
    void maybeUpdateStatistics(const MultiDataIndex &);
    void translationChanged(const MultiDataIndex &);
    void updateCaption();
    void updateLatestModel(const QModelIndex &index);
    void selectedContextChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);
    void selectedMessageChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);
    void setCurrentMessageFromGuess(int modelIndex, const Candidate &tm);

    // To synchronize from the message editor to the model ...
    void updateTranslation(const QStringList &translations);
    void updateTranslatorComment(const QString &comment);

    void updateActiveModel(int);
    void refreshItemViews();
    void toggleFinished(const QModelIndex &index);
    void prevUnfinished();
    void nextUnfinished();
    void findNext(const QString &text, DataModel::FindLocation where,
                  FindDialog::FindOptions options, int statusFilter);
    void revalidate();
    void toggleStatistics();
    void toggleVisualizeWhitespace();
    void onWhatsThis();
    void updatePhraseDicts();
    void updatePhraseDict(int model);

private:
    QModelIndex nextContext(const QModelIndex &index) const;
    QModelIndex prevContext(const QModelIndex &index) const;
    QModelIndex nextMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
    QModelIndex prevMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
    bool doNext(bool checkUnfinished);
    bool doPrev(bool checkUnfinished);
    void findAgain(FindDirection direction = FindNext);

    void updateStatistics();
    void initViewHeaders();
    void modelCountChanged();
    void setupMenuBar();
    void setupToolBars();
    void setCurrentMessage(const QModelIndex &index);
    void setCurrentMessage(const QModelIndex &index, int model);
    QModelIndex setMessageViewRoot(const QModelIndex &index);
    QModelIndex currentContextIndex() const;
    QModelIndex currentMessageIndex() const;
    PhraseBook *doOpenPhraseBook(const QString &name);
    bool isPhraseBookOpen(const QString &name);
    bool savePhraseBook(QString *name, PhraseBook &pb);
    bool maybeSavePhraseBook(PhraseBook *phraseBook);
    bool maybeSavePhraseBooks();
    QStringList pickTranslationFiles();
    void doShowTranslationSettings(int model);
    void doUpdateLatestModel(int model);
    void updateSourceView(int model, MessageItem *item);
    void updatePhraseBookActions();
    void updatePhraseDictInternal(int model);
    void releaseInternal(int model);
    void saveInternal(int model);

    QPrinter *printer();

    // FIXME: move to DataModel
    void updateDanger(const MultiDataIndex &index, bool verbose);

    bool searchItem(DataModel::FindLocation where, const QString &searchWhat);

    QProcess *m_assistantProcess;
    QTreeView *m_contextView;
    QTreeView *m_messageView;
    MultiDataModel *m_dataModel;
    MessageModel *m_messageModel;
    QSortFilterProxyModel *m_sortedContextsModel;
    QSortFilterProxyModel *m_sortedMessagesModel;
    MessageEditor *m_messageEditor;
    PhraseView *m_phraseView;
    QStackedWidget *m_sourceAndFormView;
    SourceCodeView *m_sourceCodeView;
    FormPreviewView *m_formPreviewView;
    ErrorsView *m_errorsView;
    QLabel *m_progressLabel;
    QLabel *m_modifiedLabel;
    FocusWatcher *m_focusWatcher;
    QString m_phraseBookDir;
    // model : keyword -> list of appropriate phrases in the phrasebooks
    QList<QHash<QString, QList<Phrase *> > > m_phraseDict;
    QList<PhraseBook *> m_phraseBooks;
    QMap<QAction *, PhraseBook *> m_phraseBookMenu[3];
    QPrinter *m_printer;

    FindDialog *m_findDialog;
    QString m_findText;
    FindDialog::FindOptions m_findOptions;
    int m_findStatusFilter = -1;
    DataModel::FindLocation m_findWhere;

    TranslateDialog *m_translateDialog;
    QString m_latestFindText;
    int m_latestCaseSensitivity;
    int m_remainingCount;
    int m_hitCount;

    BatchTranslationDialog *m_batchTranslateDialog;
    TranslationSettingsDialog *m_translationSettingsDialog;

    bool m_settingCurrentMessage;
    int m_fileActiveModel;
    int m_editActiveModel;
    MultiDataIndex m_currentIndex;

    QDockWidget *m_contextDock;
    QDockWidget *m_messagesDock;
    QDockWidget *m_phrasesDock;
    QDockWidget *m_sourceAndFormDock;
    QDockWidget *m_errorsDock;

    Ui::MainWindow m_ui;    // menus and actions
    Statistics *m_statistics;
};

QT_END_NAMESPACE

#endif
