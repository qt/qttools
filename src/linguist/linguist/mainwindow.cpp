/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*  TRANSLATOR MainWindow

  This is the application's main window.
*/

#include "mainwindow.h"

#include "batchtranslationdialog.h"
#include "errorsview.h"
#include "finddialog.h"
#include "formpreviewview.h"
#include "globals.h"
#include "messageeditor.h"
#include "messagemodel.h"
#include "phrasebookbox.h"
#include "phrasemodel.h"
#include "phraseview.h"
#include "printout.h"
#include "sourcecodeview.h"
#include "statistics.h"
#include "translatedialog.h"
#include "translationsettingsdialog.h"

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QCloseEvent>
#include <QDebug>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemDelegate>
#include <QLabel>
#include <QLayout>
#include <QLibraryInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPrintDialog>
#include <QPrinter>
#include <QProcess>
#include <QRegularExpression>
#include <QScreen>
#include <QShortcut>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include <QUrl>
#include <QWhatsThis>

#include <ctype.h>

QT_BEGIN_NAMESPACE

static const int MessageMS = 2500;

enum Ending {
    End_None,
    End_FullStop,
    End_Interrobang,
    End_Colon,
    End_Ellipsis
};

static bool hasFormPreview(const QString &fileName)
{
    return fileName.endsWith(QLatin1String(".ui"))
      || fileName.endsWith(QLatin1String(".jui"));
}

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

static Ending ending(QString str, QLocale::Language lang)
{
    str = str.simplified();
    if (str.isEmpty())
        return End_None;

    switch (str.at(str.length() - 1).unicode()) {
    case 0x002e: // full stop
        if (str.endsWith(QLatin1String("...")))
            return End_Ellipsis;
        else
            return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
        return End_FullStop;
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
        return End_Interrobang;
    case 0x003b: // greek 'compatibility' questionmark
        return lang == QLocale::Greek ? End_Interrobang : End_None;
    case 0x003a: // colon
    case 0xff1a: // full width colon
        return End_Colon;
    case 0x2026: // horizontal ellipsis
        return End_Ellipsis;
    default:
        return End_None;
    }
}


class ContextItemDelegate : public QItemDelegate
{
public:
    ContextItemDelegate(QObject *parent, MultiDataModel *model) : QItemDelegate(parent), m_dataModel(model) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        const QAbstractItemModel *model = index.model();
        Q_ASSERT(model);

        if (!model->parent(index).isValid()) {
            if (index.column() - 1 == m_dataModel->modelCount()) {
                QStyleOptionViewItem opt = option;
                opt.font.setBold(true);
                QItemDelegate::paint(painter, opt, index);
                return;
            }
        }
        QItemDelegate::paint(painter, option, index);
    }

private:
    MultiDataModel *m_dataModel;
};

static const QVariant &pxObsolete()
{
    static const QVariant v =
        QVariant::fromValue(QPixmap(QLatin1String(":/images/s_check_obsolete.png")));
    return v;
}


class SortedMessagesModel : public QSortFilterProxyModel
{
public:
    SortedMessagesModel(QObject *parent, MultiDataModel *model) : QSortFilterProxyModel(parent), m_dataModel(model) {}

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
            switch (section - m_dataModel->modelCount()) {
                case 0: return QString();
                case 1: return MainWindow::tr("Source text");
                case 2: return MainWindow::tr("Index");
            }

        if (role == Qt::DecorationRole && orientation == Qt::Horizontal && section - 1 < m_dataModel->modelCount())
            return pxObsolete();

        return QVariant();
    }

private:
    MultiDataModel *m_dataModel;
};

class SortedContextsModel : public QSortFilterProxyModel
{
public:
    SortedContextsModel(QObject *parent, MultiDataModel *model) : QSortFilterProxyModel(parent), m_dataModel(model) {}

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
            switch (section - m_dataModel->modelCount()) {
                case 0: return QString();
                case 1: return MainWindow::tr("Context");
                case 2: return MainWindow::tr("Items");
                case 3: return MainWindow::tr("Index");
            }

        if (role == Qt::DecorationRole && orientation == Qt::Horizontal && section - 1 < m_dataModel->modelCount())
            return pxObsolete();

        return QVariant();
    }

private:
    MultiDataModel *m_dataModel;
};

class FocusWatcher : public QObject
{
public:
    FocusWatcher(MessageEditor *msgedit, QObject *parent) : QObject(parent), m_messageEditor(msgedit) {}

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    MessageEditor *m_messageEditor;
};

bool FocusWatcher::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        m_messageEditor->setEditorFocusForModel(-1);
    return false;
}

MainWindow::MainWindow()
    : QMainWindow(0, Qt::Window),
      m_assistantProcess(0),
      m_printer(0),
      m_findMatchCase(Qt::CaseInsensitive),
      m_findIgnoreAccelerators(true),
      m_findSkipObsolete(false),
      m_findUseRegExp(false),
      m_findWhere(DataModel::NoLocation),
      m_translationSettingsDialog(0),
      m_settingCurrentMessage(false),
      m_fileActiveModel(-1),
      m_editActiveModel(-1),
      m_statistics(0)
{
    setUnifiedTitleAndToolBarOnMac(true);
    m_ui.setupUi(this);

#if !defined(Q_OS_OSX) && !defined(Q_OS_WIN)
    setWindowIcon(QPixmap(QLatin1String(":/images/appicon.png") ));
#endif

    m_dataModel = new MultiDataModel(this);
    m_messageModel = new MessageModel(this, m_dataModel);

    // Set up the context dock widget
    m_contextDock = new QDockWidget(this);
    m_contextDock->setObjectName(QLatin1String("ContextDockWidget"));
    m_contextDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_contextDock->setWindowTitle(tr("Context"));
    m_contextDock->setAcceptDrops(true);
    m_contextDock->installEventFilter(this);

    m_sortedContextsModel = new SortedContextsModel(this, m_dataModel);
    m_sortedContextsModel->setSortRole(MessageModel::SortRole);
    m_sortedContextsModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortedContextsModel->setSourceModel(m_messageModel);

    m_contextView = new QTreeView(this);
    m_contextView->setRootIsDecorated(false);
    m_contextView->setItemsExpandable(false);
    m_contextView->setUniformRowHeights(true);
    m_contextView->setAlternatingRowColors(true);
    m_contextView->setAllColumnsShowFocus(true);
    m_contextView->setItemDelegate(new ContextItemDelegate(this, m_dataModel));
    m_contextView->setSortingEnabled(true);
    m_contextView->setWhatsThis(tr("This panel lists the source contexts."));
    m_contextView->setModel(m_sortedContextsModel);
    m_contextView->header()->setSectionsMovable(false);
    m_contextView->setColumnHidden(0, true);
    m_contextView->header()->setStretchLastSection(false);

    m_contextDock->setWidget(m_contextView);

    // Set up the messages dock widget
    m_messagesDock = new QDockWidget(this);
    m_messagesDock->setObjectName(QLatin1String("StringsDockWidget"));
    m_messagesDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_messagesDock->setWindowTitle(tr("Strings"));
    m_messagesDock->setAcceptDrops(true);
    m_messagesDock->installEventFilter(this);

    m_sortedMessagesModel = new SortedMessagesModel(this, m_dataModel);
    m_sortedMessagesModel->setSortRole(MessageModel::SortRole);
    m_sortedMessagesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortedMessagesModel->setSortLocaleAware(true);
    m_sortedMessagesModel->setSourceModel(m_messageModel);

    m_messageView = new QTreeView(m_messagesDock);
    m_messageView->setSortingEnabled(true);
    m_messageView->setRootIsDecorated(false);
    m_messageView->setUniformRowHeights(true);
    m_messageView->setAllColumnsShowFocus(true);
    m_messageView->setItemsExpandable(false);
    m_messageView->setModel(m_sortedMessagesModel);
    m_messageView->header()->setSectionsMovable(false);
    m_messageView->setColumnHidden(0, true);

    m_messagesDock->setWidget(m_messageView);

    // Set up main message view
    m_messageEditor = new MessageEditor(m_dataModel, this);
    m_messageEditor->setAcceptDrops(true);
    m_messageEditor->installEventFilter(this);
    // We can't call setCentralWidget(m_messageEditor), since it is already called in m_ui.setupUi()
    QBoxLayout *lout = new QBoxLayout(QBoxLayout::TopToBottom, m_ui.centralwidget);
    lout->addWidget(m_messageEditor);
    lout->setContentsMargins(QMargins());
    m_ui.centralwidget->setLayout(lout);

    // Set up the phrases & guesses dock widget
    m_phrasesDock = new QDockWidget(this);
    m_phrasesDock->setObjectName(QLatin1String("PhrasesDockwidget"));
    m_phrasesDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_phrasesDock->setWindowTitle(tr("Phrases and guesses"));

    m_phraseView = new PhraseView(m_dataModel, &m_phraseDict, this);
    m_phrasesDock->setWidget(m_phraseView);

    // Set up source code and form preview dock widget
    m_sourceAndFormDock = new QDockWidget(this);
    m_sourceAndFormDock->setObjectName(QLatin1String("SourceAndFormDock"));
    m_sourceAndFormDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_sourceAndFormDock->setWindowTitle(tr("Sources and Forms"));
    m_sourceAndFormView = new QStackedWidget(this);
    m_sourceAndFormDock->setWidget(m_sourceAndFormView);
    m_formPreviewView = new FormPreviewView(0, m_dataModel);
    m_sourceCodeView = new SourceCodeView(0);
    m_sourceAndFormView->addWidget(m_sourceCodeView);
    m_sourceAndFormView->addWidget(m_formPreviewView);

    // Set up errors dock widget
    m_errorsDock = new QDockWidget(this);
    m_errorsDock->setObjectName(QLatin1String("ErrorsDockWidget"));
    m_errorsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_errorsDock->setWindowTitle(tr("Warnings"));
    m_errorsView = new ErrorsView(m_dataModel, this);
    m_errorsDock->setWidget(m_errorsView);

    // Arrange dock widgets
    setDockNestingEnabled(true);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_contextDock);
    addDockWidget(Qt::TopDockWidgetArea, m_messagesDock);
    addDockWidget(Qt::BottomDockWidgetArea, m_phrasesDock);
    addDockWidget(Qt::TopDockWidgetArea, m_sourceAndFormDock);
    addDockWidget(Qt::BottomDockWidgetArea, m_errorsDock);
    //tabifyDockWidget(m_errorsDock, m_sourceAndFormDock);
    //tabifyDockWidget(m_sourceCodeDock, m_phrasesDock);

    // Allow phrases doc to intercept guesses shortcuts
    m_messageEditor->installEventFilter(m_phraseView);

    // Set up shortcuts for the dock widgets
    QShortcut *contextShortcut = new QShortcut(QKeySequence(Qt::Key_F6), this);
    connect(contextShortcut, &QShortcut::activated,
            this, &MainWindow::showContextDock);
    QShortcut *messagesShortcut = new QShortcut(QKeySequence(Qt::Key_F7), this);
    connect(messagesShortcut, &QShortcut::activated,
            this, &MainWindow::showMessagesDock);
    QShortcut *errorsShortcut = new QShortcut(QKeySequence(Qt::Key_F8), this);
    connect(errorsShortcut, &QShortcut::activated,
            this, &MainWindow::showErrorDock);
    QShortcut *sourceCodeShortcut = new QShortcut(QKeySequence(Qt::Key_F9), this);
    connect(sourceCodeShortcut, &QShortcut::activated,
            this, &MainWindow::showSourceCodeDock);
    QShortcut *phrasesShortcut = new QShortcut(QKeySequence(Qt::Key_F10), this);
    connect(phrasesShortcut, &QShortcut::activated,
            this, &MainWindow::showPhrasesDock);

    connect(m_phraseView, &PhraseView::phraseSelected,
            m_messageEditor, &MessageEditor::setTranslation);
    connect(m_phraseView, &PhraseView::setCurrentMessageFromGuess,
            this, &MainWindow::setCurrentMessageFromGuess);
    connect(m_contextView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::selectedContextChanged);
    connect(m_messageView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::selectedMessageChanged);
    connect(m_contextView->selectionModel(), &QItemSelectionModel::currentColumnChanged,
            this, &MainWindow::updateLatestModel);
    connect(m_messageView->selectionModel(), &QItemSelectionModel::currentColumnChanged,
            this, &MainWindow::updateLatestModel);

    connect(m_messageEditor, &MessageEditor::activeModelChanged,
            this, &MainWindow::updateActiveModel);

    m_translateDialog = new TranslateDialog(this);
    m_batchTranslateDialog = new BatchTranslationDialog(m_dataModel, this);
    m_findDialog = new FindDialog(this);

    setupMenuBar();
    setupToolBars();

    m_progressLabel = new QLabel();
    statusBar()->addPermanentWidget(m_progressLabel);
    m_modifiedLabel = new QLabel(tr(" MOD ", "status bar: file(s) modified"));
    statusBar()->addPermanentWidget(m_modifiedLabel);

    modelCountChanged();
    initViewHeaders();
    resetSorting();

    connect(m_dataModel, &MultiDataModel::modifiedChanged,
            this, &QWidget::setWindowModified);
    connect(m_dataModel, &MultiDataModel::modifiedChanged,
            m_modifiedLabel, &QWidget::setVisible);
    connect(m_dataModel, &MultiDataModel::multiContextDataChanged,
            this, &MainWindow::updateProgress);
    connect(m_dataModel, &MultiDataModel::messageDataChanged,
            this, &MainWindow::maybeUpdateStatistics);
    connect(m_dataModel, &MultiDataModel::translationChanged,
            this, &MainWindow::translationChanged);
    connect(m_dataModel, &MultiDataModel::languageChanged,
            this, &MainWindow::updatePhraseDict);

    setWindowModified(m_dataModel->isModified());
    m_modifiedLabel->setVisible(m_dataModel->isModified());

    connect(m_messageView, &QAbstractItemView::clicked,
            this, &MainWindow::toggleFinished);
    connect(m_messageView, &QAbstractItemView::activated,
            m_messageEditor, &MessageEditor::setEditorFocus);
    connect(m_contextView, &QAbstractItemView::activated,
            m_messageView, qOverload<>(&QWidget::setFocus));
    connect(m_messageEditor, &MessageEditor::translationChanged,
            this, &MainWindow::updateTranslation);
    connect(m_messageEditor, &MessageEditor::translatorCommentChanged,
            this, &MainWindow::updateTranslatorComment);
    connect(m_findDialog, &FindDialog::findNext,
            this, &MainWindow::findNext);
    connect(m_translateDialog, &TranslateDialog::requestMatchUpdate,
            this, &MainWindow::updateTranslateHit);
    connect(m_translateDialog, &TranslateDialog::activated,
            this, &MainWindow::translate);

    QSize as(screen()->size());
    as -= QSize(30, 30);
    resize(QSize(1000, 800).boundedTo(as));
    show();
    readConfig();
    m_statistics = 0;

    connect(m_ui.actionLengthVariants, &QAction::toggled,
            m_messageEditor, &MessageEditor::setLengthVariants);
    m_messageEditor->setLengthVariants(m_ui.actionLengthVariants->isChecked());
    m_messageEditor->setVisualizeWhitespace(m_ui.actionVisualizeWhitespace->isChecked());

    m_focusWatcher = new FocusWatcher(m_messageEditor, this);
    m_contextView->installEventFilter(m_focusWatcher);
    m_messageView->installEventFilter(m_focusWatcher);
    m_messageEditor->installEventFilter(m_focusWatcher);
    m_sourceAndFormView->installEventFilter(m_focusWatcher);
    m_phraseView->installEventFilter(m_focusWatcher);
    m_errorsView->installEventFilter(m_focusWatcher);
}

MainWindow::~MainWindow()
{
    writeConfig();
    if (m_assistantProcess && m_assistantProcess->state() == QProcess::Running) {
        m_assistantProcess->terminate();
        m_assistantProcess->waitForFinished(3000);
    }
    qDeleteAll(m_phraseBooks);
    delete m_dataModel;
    delete m_statistics;
    delete m_printer;
}

void MainWindow::initViewHeaders()
{
    m_contextView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_contextView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_messageView->setColumnHidden(2, true);
    // last visible column auto-stretches
}

void MainWindow::modelCountChanged()
{
    int mc = m_dataModel->modelCount();

    for (int i = 0; i < mc; ++i) {
        m_contextView->header()->setSectionResizeMode(i + 1, QHeaderView::Fixed);
        m_contextView->header()->resizeSection(i + 1, 24);

        m_messageView->header()->setSectionResizeMode(i + 1, QHeaderView::Fixed);
        m_messageView->header()->resizeSection(i + 1, 24);
    }

    if (!mc) {
        selectedMessageChanged(QModelIndex(), QModelIndex());
        doUpdateLatestModel(-1);
    } else {
        if (!m_contextView->currentIndex().isValid()) {
            // Ensure that something is selected
            m_contextView->setCurrentIndex(m_sortedContextsModel->index(0, 0));
        } else {
            // Plug holes that turn up in the selection due to inserting columns
            m_contextView->selectionModel()->select(m_contextView->currentIndex(),
                        QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            m_messageView->selectionModel()->select(m_messageView->currentIndex(),
                        QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        }
        // Field insertions/removals are automatic, but not the re-fill
        m_messageEditor->showMessage(m_currentIndex);
        if (mc == 1)
            doUpdateLatestModel(0);
        else if (m_currentIndex.model() >= mc)
            doUpdateLatestModel(mc - 1);
    }

    m_contextView->setUpdatesEnabled(true);
    m_messageView->setUpdatesEnabled(true);

    updateProgress();
    updateCaption();

    m_ui.actionFind->setEnabled(m_dataModel->contextCount() > 0);
    m_ui.actionFindNext->setEnabled(false);

    m_formPreviewView->setSourceContext(-1, 0);
}

struct OpenedFile {
    OpenedFile(DataModel *_dataModel, bool _readWrite, bool _langGuessed)
        { dataModel = _dataModel; readWrite = _readWrite; langGuessed = _langGuessed; }
    DataModel *dataModel;
    bool readWrite;
    bool langGuessed;
};

bool MainWindow::openFiles(const QStringList &names, bool globalReadWrite)
{
    if (names.isEmpty())
        return false;

    bool waitCursor = false;
    statusBar()->showMessage(tr("Loading..."));
    qApp->processEvents();

    QList<OpenedFile> opened;
    bool closeOld = false;
    for (QString name : names) {
        if (!waitCursor) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            waitCursor = true;
        }

        bool readWrite = globalReadWrite;
        if (name.startsWith(QLatin1Char('='))) {
            name.remove(0, 1);
            readWrite = false;
        }
        QFileInfo fi(name);
        if (fi.exists()) // Make the loader error out instead of reading stdin
            name = fi.canonicalFilePath();
        if (m_dataModel->isFileLoaded(name) >= 0)
            continue;

        bool langGuessed;
        DataModel *dm = new DataModel(m_dataModel);
        if (!dm->load(name, &langGuessed, this)) {
            delete dm;
            continue;
        }
        if (opened.isEmpty()) {
            if (!m_dataModel->isWellMergeable(dm)) {
                QApplication::restoreOverrideCursor();
                waitCursor = false;
                switch (QMessageBox::information(this, tr("Loading File - Qt Linguist"),
                    tr("The file '%1' does not seem to be related to the currently open file(s) '%2'.\n\n"
                       "Close the open file(s) first?")
                       .arg(DataModel::prettifyPlainFileName(name), m_dataModel->condensedSrcFileNames(true)),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
                {
                    case QMessageBox::Cancel:
                        delete dm;
                        return false;
                    case QMessageBox::Yes:
                        closeOld = true;
                        break;
                    default:
                        break;
                }
            }
        } else {
            if (!opened.first().dataModel->isWellMergeable(dm)) {
                QApplication::restoreOverrideCursor();
                waitCursor = false;
                switch (QMessageBox::information(this, tr("Loading File - Qt Linguist"),
                    tr("The file '%1' does not seem to be related to the file '%2'"
                       " which is being loaded as well.\n\n"
                       "Skip loading the first named file?")
                       .arg(DataModel::prettifyPlainFileName(name), opened.first().dataModel->srcFileName(true)),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
                {
                    case QMessageBox::Cancel:
                        delete dm;
                        for (const OpenedFile &op : qAsConst(opened))
                            delete op.dataModel;
                        return false;
                    case QMessageBox::Yes:
                        delete dm;
                        continue;
                    default:
                        break;
                }
            }
        }
        opened.append(OpenedFile(dm, readWrite, langGuessed));
    }

    if (closeOld) {
        if (waitCursor) {
            QApplication::restoreOverrideCursor();
            waitCursor = false;
        }
        if (!closeAll()) {
            for (const OpenedFile &op : qAsConst(opened))
                delete op.dataModel;
            return false;
        }
    }

    for (const OpenedFile &op : qAsConst(opened)) {
        if (op.langGuessed) {
            if (waitCursor) {
                QApplication::restoreOverrideCursor();
                waitCursor = false;
            }
            if (!m_translationSettingsDialog)
                m_translationSettingsDialog = new TranslationSettingsDialog(this);
            m_translationSettingsDialog->setDataModel(op.dataModel);
            m_translationSettingsDialog->exec();
        }
    }

    if (!waitCursor)
        QApplication::setOverrideCursor(Qt::WaitCursor);
    m_contextView->setUpdatesEnabled(false);
    m_messageView->setUpdatesEnabled(false);
    int totalCount = 0;
    for (const OpenedFile &op : qAsConst(opened)) {
        m_phraseDict.append(QHash<QString, QList<Phrase *> >());
        m_dataModel->append(op.dataModel, op.readWrite);
        if (op.readWrite)
            updatePhraseDictInternal(m_phraseDict.size() - 1);
        totalCount += op.dataModel->messageCount();
    }
    statusBar()->showMessage(tr("%n translation unit(s) loaded.", 0, totalCount), MessageMS);
    modelCountChanged();
    recentFiles().addFiles(m_dataModel->srcFileNames());

    revalidate();
    QApplication::restoreOverrideCursor();
    return true;
}

RecentFiles &MainWindow::recentFiles()
{
    static RecentFiles recentFiles(10);
    return recentFiles;
}

void MainWindow::open()
{
    openFiles(pickTranslationFiles());
}

void MainWindow::openAux()
{
    openFiles(pickTranslationFiles(), false);
}

void MainWindow::closeFile()
{
    int model = m_currentIndex.model();
    if (model >= 0 && maybeSave(model)) {
        m_phraseDict.removeAt(model);
        m_contextView->setUpdatesEnabled(false);
        m_messageView->setUpdatesEnabled(false);
        m_dataModel->close(model);
        modelCountChanged();
    }
}

bool MainWindow::closeAll()
{
    if (maybeSaveAll()) {
        m_phraseDict.clear();
        m_contextView->setUpdatesEnabled(false);
        m_messageView->setUpdatesEnabled(false);
        m_dataModel->closeAll();
        modelCountChanged();
        initViewHeaders();
        recentFiles().closeGroup();
        return true;
    }
    return false;
}

static QString fileFilters(bool allFirst)
{
    static const QString pattern(QLatin1String("%1 (*.%2);;"));
    QStringList allExtensions;
    QString filter;
    for (const Translator::FileFormat &format : qAsConst(Translator::registeredFileFormats())) {
        if (format.fileType == Translator::FileFormat::TranslationSource && format.priority >= 0) {
            filter.append(pattern.arg(format.description(), format.extension));
            allExtensions.append(QLatin1String("*.") + format.extension);
        }
    }
    QString allFilter = QObject::tr("Translation files (%1);;").arg(allExtensions.join(QLatin1Char(' ')));
    if (allFirst)
        filter.prepend(allFilter);
    else
        filter.append(allFilter);
    filter.append(QObject::tr("All files (*)"));
    return filter;
}

QStringList MainWindow::pickTranslationFiles()
{
    QString dir;
    if (!recentFiles().isEmpty())
        dir = QFileInfo(recentFiles().lastOpenedFile()).path();

    QString varFilt;
    if (m_dataModel->modelCount()) {
        QFileInfo mainFile(m_dataModel->srcFileName(0));
        QString mainFileBase = mainFile.baseName();
        int pos = mainFileBase.indexOf(QLatin1Char('_'));
        if (pos > 0)
            varFilt = tr("Related files (%1);;")
                .arg(mainFileBase.left(pos) + QLatin1String("_*.") + mainFile.completeSuffix());
    }

    return QFileDialog::getOpenFileNames(this, tr("Open Translation Files"), dir,
        varFilt +
        fileFilters(true));
}

void MainWindow::saveInternal(int model)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (m_dataModel->save(model, this)) {
        updateCaption();
        statusBar()->showMessage(tr("File saved."), MessageMS);
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::saveAll()
{
    for (int i = 0; i < m_dataModel->modelCount(); ++i)
        if (m_dataModel->isModelWritable(i))
            saveInternal(i);
    recentFiles().closeGroup();
}

void MainWindow::save()
{
    if (m_currentIndex.model() < 0)
        return;

    saveInternal(m_currentIndex.model());
}

void MainWindow::saveAs()
{
    if (m_currentIndex.model() < 0)
        return;

    QString newFilename = QFileDialog::getSaveFileName(this, QString(), m_dataModel->srcFileName(m_currentIndex.model()),
        fileFilters(false));
    if (!newFilename.isEmpty()) {
        if (m_dataModel->saveAs(m_currentIndex.model(), newFilename, this)) {
            updateCaption();
            statusBar()->showMessage(tr("File saved."), MessageMS);
            recentFiles().addFiles(m_dataModel->srcFileNames());
        }
    }
}

void MainWindow::releaseAs()
{
    if (m_currentIndex.model() < 0)
        return;

    QFileInfo oldFile(m_dataModel->srcFileName(m_currentIndex.model()));
    QString newFilename = oldFile.path() + QLatin1String("/")
                + oldFile.completeBaseName() + QLatin1String(".qm");

    newFilename = QFileDialog::getSaveFileName(this, tr("Release"), newFilename,
        tr("Qt message files for released applications (*.qm)\nAll files (*)"));
    if (!newFilename.isEmpty()) {
        if (m_dataModel->release(m_currentIndex.model(), newFilename, false, false, SaveEverything, this))
            statusBar()->showMessage(tr("File created."), MessageMS);
    }
}

void MainWindow::releaseInternal(int model)
{
    QFileInfo oldFile(m_dataModel->srcFileName(model));
    QString newFilename = oldFile.path() + QLatin1Char('/')
                + oldFile.completeBaseName() + QLatin1String(".qm");

    if (!newFilename.isEmpty()) {
        if (m_dataModel->release(model, newFilename, false, false, SaveEverything, this))
            statusBar()->showMessage(tr("File created."), MessageMS);
    }
}

// No-question
void MainWindow::release()
{
    if (m_currentIndex.model() < 0)
        return;

    releaseInternal(m_currentIndex.model());
}

void MainWindow::releaseAll()
{
    for (int i = 0; i < m_dataModel->modelCount(); ++i)
        if (m_dataModel->isModelWritable(i))
            releaseInternal(i);
}

QPrinter *MainWindow::printer()
{
    if (!m_printer)
        m_printer = new QPrinter;
    return m_printer;
}

void MainWindow::print()
{
    int pageNum = 0;
    QPrintDialog dlg(printer(), this);
    if (dlg.exec()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        printer()->setDocName(m_dataModel->condensedSrcFileNames(true));
        statusBar()->showMessage(tr("Printing..."));
        PrintOut pout(printer());

        for (int i = 0; i < m_dataModel->contextCount(); ++i) {
            MultiContextItem *mc = m_dataModel->multiContextItem(i);
            pout.vskip();
            pout.setRule(PrintOut::ThickRule);
            pout.setGuide(mc->context());
            pout.addBox(100, tr("Context: %1").arg(mc->context()),
                PrintOut::Strong);
            pout.flushLine();
            pout.addBox(4);
            pout.addBox(92, mc->comment(), PrintOut::Emphasis);
            pout.flushLine();
            pout.setRule(PrintOut::ThickRule);

            for (int j = 0; j < mc->messageCount(); ++j) {
                pout.setRule(PrintOut::ThinRule);
                bool printedSrc = false;
                QString comment;
                for (int k = 0; k < m_dataModel->modelCount(); ++k) {
                    if (const MessageItem *m = mc->messageItem(k, j)) {
                        if (!printedSrc) {
                            pout.addBox(40, m->text());
                            pout.addBox(4);
                            comment = m->comment();
                            printedSrc = true;
                        } else {
                            pout.addBox(44); // Maybe put the name of the translation here
                        }
                        if (m->message().isPlural() && m_dataModel->language(k) != QLocale::C) {
                            QStringList transls = m->translations();
                            pout.addBox(40, transls.join(QLatin1Char('\n')));
                        } else {
                            pout.addBox(40, m->translation());
                        }
                        pout.addBox(4);
                        QString type;
                        switch (m->message().type()) {
                        case TranslatorMessage::Finished:
                            type = tr("finished");
                            break;
                        case TranslatorMessage::Unfinished:
                            type = m->danger() ? tr("unresolved") : QLatin1String("unfinished");
                            break;
                        case TranslatorMessage::Obsolete:
                        case TranslatorMessage::Vanished:
                            type = tr("obsolete");
                            break;
                        }
                        pout.addBox(12, type, PrintOut::Normal, Qt::AlignRight);
                        pout.flushLine();
                    }
                }
                if (!comment.isEmpty()) {
                    pout.addBox(4);
                    pout.addBox(92, comment, PrintOut::Emphasis);
                    pout.flushLine(true);
                }

                if (pout.pageNum() != pageNum) {
                    pageNum = pout.pageNum();
                    statusBar()->showMessage(tr("Printing... (page %1)")
                        .arg(pageNum));
                }
            }
        }
        pout.flushLine(true);
        QApplication::restoreOverrideCursor();
        statusBar()->showMessage(tr("Printing completed"), MessageMS);
    } else {
        statusBar()->showMessage(tr("Printing aborted"), MessageMS);
    }
}

bool MainWindow::searchItem(DataModel::FindLocation where, const QString &searchWhat)
{
    if ((m_findWhere & where) == 0)
        return false;

    QString text = searchWhat;

    if (m_findIgnoreAccelerators)
        // FIXME: This removes too much. The proper solution might be too slow, though.
        text.remove(QLatin1Char('&'));

    if (m_findUseRegExp)
        return m_findDialog->getRegExp().match(text).hasMatch();
    else
        return text.indexOf(m_findText, 0, m_findMatchCase) >= 0;
}

void MainWindow::findAgain()
{
    if (m_dataModel->contextCount() == 0)
        return;

    const QModelIndex &startIndex = m_messageView->currentIndex();
    QModelIndex index = nextMessage(startIndex);

    while (index.isValid()) {
        QModelIndex realIndex = m_sortedMessagesModel->mapToSource(index);
        MultiDataIndex dataIndex = m_messageModel->dataIndex(realIndex, -1);
        bool hadMessage = false;
        for (int i = 0; i < m_dataModel->modelCount(); ++i) {
            if (MessageItem *m = m_dataModel->messageItem(dataIndex, i)) {
                if (m_findSkipObsolete && m->isObsolete())
                    continue;
                bool found = true;
                do {
                    if (!hadMessage) {
                        if (searchItem(DataModel::SourceText, m->text()))
                            break;
                        if (searchItem(DataModel::SourceText, m->pluralText()))
                            break;
                        if (searchItem(DataModel::Comments, m->comment()))
                            break;
                        if (searchItem(DataModel::Comments, m->extraComment()))
                            break;
                    }
                    const auto translations = m->translations();
                    for (const QString &trans : translations)
                        if (searchItem(DataModel::Translations, trans))
                            goto didfind;
                    if (searchItem(DataModel::Comments, m->translatorComment()))
                        break;
                    found = false;
                    // did not find the search string in this message
                } while (0);
                if (found) {
                  didfind:
                    setCurrentMessage(realIndex, i);

                    // determine whether the search wrapped
                    const QModelIndex &c1 = m_sortedContextsModel->mapFromSource(
                            m_sortedMessagesModel->mapToSource(startIndex)).parent();
                    const QModelIndex &c2 = m_sortedContextsModel->mapFromSource(realIndex).parent();
                    const QModelIndex &m = m_sortedMessagesModel->mapFromSource(realIndex);

                    if (c2.row() < c1.row() || (c1.row() == c2.row() && m.row() <= startIndex.row()))
                        statusBar()->showMessage(tr("Search wrapped."), MessageMS);

                    m_findDialog->hide();
                    return;
                }
                hadMessage = true;
            }
        }

        // since we don't search startIndex at the beginning, only now we have searched everything
        if (index == startIndex)
            break;

        index = nextMessage(index);
    }

    qApp->beep();
    QMessageBox::warning(m_findDialog, tr("Qt Linguist"),
                         tr("Cannot find the string '%1'.").arg(m_findText));
}

void MainWindow::showBatchTranslateDialog()
{
    m_messageModel->blockSignals(true);
    m_batchTranslateDialog->setPhraseBooks(m_phraseBooks, m_currentIndex.model());
    if (m_batchTranslateDialog->exec() != QDialog::Accepted)
        m_messageModel->blockSignals(false);
    // else signal finished() calls refreshItemViews()
}

void MainWindow::showTranslateDialog()
{
    m_latestCaseSensitivity = -1;
    QModelIndex idx = m_messageView->currentIndex();
    QModelIndex idx2 = m_sortedMessagesModel->index(idx.row(), m_currentIndex.model() + 1, idx.parent());
    m_messageView->setCurrentIndex(idx2);
    QString fn = QFileInfo(m_dataModel->srcFileName(m_currentIndex.model())).baseName();
    m_translateDialog->setWindowTitle(tr("Search And Translate in '%1' - Qt Linguist").arg(fn));
    m_translateDialog->exec();
}

void MainWindow::updateTranslateHit(bool &hit)
{
    MessageItem *m;
    hit = (m = m_dataModel->messageItem(m_currentIndex))
          && !m->isObsolete()
          && m->compare(m_translateDialog->findText(), false, m_translateDialog->caseSensitivity());
}

void MainWindow::translate(int mode)
{
    QString findText = m_translateDialog->findText();
    QString replaceText = m_translateDialog->replaceText();
    bool markFinished = m_translateDialog->markFinished();
    Qt::CaseSensitivity caseSensitivity = m_translateDialog->caseSensitivity();

    int translatedCount = 0;

    if (mode == TranslateDialog::TranslateAll) {
        for (MultiDataModelIterator it(m_dataModel, m_currentIndex.model()); it.isValid(); ++it) {
            MessageItem *m = it.current();
            if (m && !m->isObsolete() && m->compare(findText, false, caseSensitivity)) {
                if (!translatedCount)
                    m_messageModel->blockSignals(true);
                m_dataModel->setTranslation(it, replaceText);
                m_dataModel->setFinished(it, markFinished);
                ++translatedCount;
            }
        }
        if (translatedCount) {
            refreshItemViews();
            QMessageBox::warning(m_translateDialog, tr("Translate - Qt Linguist"),
                    tr("Translated %n entry(s)", 0, translatedCount));
        }
    } else {
        if (mode == TranslateDialog::Translate) {
            m_dataModel->setTranslation(m_currentIndex, replaceText);
            m_dataModel->setFinished(m_currentIndex, markFinished);
        }

        if (findText != m_latestFindText || caseSensitivity != m_latestCaseSensitivity) {
            m_latestFindText = findText;
            m_latestCaseSensitivity = caseSensitivity;
            m_remainingCount = m_dataModel->messageCount();
            m_hitCount = 0;
        }

        QModelIndex index = m_messageView->currentIndex();
        int prevRemained = m_remainingCount;
        forever {
            if (--m_remainingCount <= 0) {
                if (!m_hitCount)
                    break;
                m_remainingCount = m_dataModel->messageCount() - 1;
                if (QMessageBox::question(m_translateDialog, tr("Translate - Qt Linguist"),
                        tr("No more occurrences of '%1'. Start over?").arg(findText),
                        QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
                    return;
                m_remainingCount -= prevRemained;
            }

            index = nextMessage(index);

            QModelIndex realIndex = m_sortedMessagesModel->mapToSource(index);
            MultiDataIndex dataIndex = m_messageModel->dataIndex(realIndex, m_currentIndex.model());
            if (MessageItem *m = m_dataModel->messageItem(dataIndex)) {
                if (!m->isObsolete() && m->compare(findText, false, caseSensitivity)) {
                    setCurrentMessage(realIndex, m_currentIndex.model());
                    ++translatedCount;
                    ++m_hitCount;
                    break;
                }
            }
        }
    }

    if (!translatedCount) {
        qApp->beep();
        QMessageBox::warning(m_translateDialog, tr("Translate - Qt Linguist"),
                tr("Cannot find the string '%1'.").arg(findText));
    }
}

void MainWindow::newPhraseBook()
{
    QString name = QFileDialog::getSaveFileName(this, tr("Create New Phrase Book"),
            m_phraseBookDir, tr("Qt phrase books (*.qph)\nAll files (*)"));
    if (!name.isEmpty()) {
        PhraseBook pb;
        if (!m_translationSettingsDialog)
            m_translationSettingsDialog = new TranslationSettingsDialog(this);
        m_translationSettingsDialog->setPhraseBook(&pb);
        if (!m_translationSettingsDialog->exec())
            return;
        m_phraseBookDir = QFileInfo(name).absolutePath();
        if (savePhraseBook(&name, pb)) {
            if (doOpenPhraseBook(name))
                statusBar()->showMessage(tr("Phrase book created."), MessageMS);
        }
    }
}

bool MainWindow::isPhraseBookOpen(const QString &name)
{
    for (const PhraseBook *pb : qAsConst(m_phraseBooks)) {
        if (pb->fileName() == name)
            return true;
    }

    return false;
}

void MainWindow::openPhraseBook()
{
    QString name = QFileDialog::getOpenFileName(this, tr("Open Phrase Book"),
    m_phraseBookDir, tr("Qt phrase books (*.qph);;All files (*)"));

    if (!name.isEmpty()) {
        m_phraseBookDir = QFileInfo(name).absolutePath();
        if (!isPhraseBookOpen(name)) {
            if (PhraseBook *phraseBook = doOpenPhraseBook(name)) {
                int n = phraseBook->phrases().count();
                statusBar()->showMessage(tr("%n phrase(s) loaded.", 0, n), MessageMS);
            }
        }
    }
}

void MainWindow::closePhraseBook(QAction *action)
{
    PhraseBook *pb = m_phraseBookMenu[PhraseCloseMenu].value(action);
    if (!maybeSavePhraseBook(pb))
        return;

    m_phraseBookMenu[PhraseCloseMenu].remove(action);
    m_ui.menuClosePhraseBook->removeAction(action);

    QAction *act = m_phraseBookMenu[PhraseEditMenu].key(pb);
    m_phraseBookMenu[PhraseEditMenu].remove(act);
    m_ui.menuEditPhraseBook->removeAction(act);

    act = m_phraseBookMenu[PhrasePrintMenu].key(pb);
    m_ui.menuPrintPhraseBook->removeAction(act);

    m_phraseBooks.removeOne(pb);
    disconnect(pb, &PhraseBook::listChanged,
               this, &MainWindow::updatePhraseDicts);
    updatePhraseDicts();
    delete pb;
    updatePhraseBookActions();
}

void MainWindow::editPhraseBook(QAction *action)
{
    PhraseBook *pb = m_phraseBookMenu[PhraseEditMenu].value(action);
    PhraseBookBox box(pb, this);
    box.exec();

    updatePhraseDicts();
}

void MainWindow::printPhraseBook(QAction *action)
{
    PhraseBook *phraseBook = m_phraseBookMenu[PhrasePrintMenu].value(action);

    int pageNum = 0;

    QPrintDialog dlg(printer(), this);
    if (dlg.exec()) {
        printer()->setDocName(phraseBook->fileName());
        statusBar()->showMessage(tr("Printing..."));
        PrintOut pout(printer());
        pout.setRule(PrintOut::ThinRule);
        const auto phrases = phraseBook->phrases();
        for (const Phrase *p : phrases) {
            pout.setGuide(p->source());
            pout.addBox(29, p->source());
            pout.addBox(4);
            pout.addBox(29, p->target());
            pout.addBox(4);
            pout.addBox(34, p->definition(), PrintOut::Emphasis);

            if (pout.pageNum() != pageNum) {
                pageNum = pout.pageNum();
                statusBar()->showMessage(tr("Printing... (page %1)")
                    .arg(pageNum));
            }
            pout.setRule(PrintOut::NoRule);
            pout.flushLine(true);
        }
        pout.flushLine(true);
        statusBar()->showMessage(tr("Printing completed"), MessageMS);
    } else {
        statusBar()->showMessage(tr("Printing aborted"), MessageMS);
    }
}

void MainWindow::addToPhraseBook()
{
    QStringList phraseBookList;
    QHash<QString, PhraseBook *> phraseBookHash;
    for (PhraseBook *pb : qAsConst(m_phraseBooks)) {
        if (pb->language() != QLocale::C && m_dataModel->language(m_currentIndex.model()) != QLocale::C) {
            if (pb->language() != m_dataModel->language(m_currentIndex.model()))
                continue;
            if (pb->country() == m_dataModel->model(m_currentIndex.model())->country())
                phraseBookList.prepend(pb->friendlyPhraseBookName());
            else
                phraseBookList.append(pb->friendlyPhraseBookName());
        } else {
            phraseBookList.append(pb->friendlyPhraseBookName());
        }
        phraseBookHash.insert(pb->friendlyPhraseBookName(), pb);
    }
    if (phraseBookList.isEmpty()) {
        QMessageBox::warning(this, tr("Add to phrase book"),
              tr("No appropriate phrasebook found."));
        return;
    }

    QString selectedPhraseBook;
    if (phraseBookList.size() == 1) {
        selectedPhraseBook = phraseBookList.at(0);
        if (QMessageBox::information(this, tr("Add to phrase book"),
              tr("Adding entry to phrasebook %1").arg(selectedPhraseBook),
               QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)
                              != QMessageBox::Ok)
            return;
    } else {
        bool okPressed = false;
        selectedPhraseBook = QInputDialog::getItem(this, tr("Add to phrase book"),
                                                   tr("Select phrase book to add to"),
                                                   phraseBookList, 0, false, &okPressed);
        if (!okPressed)
            return;
    }

    MessageItem *currentMessage = m_dataModel->messageItem(m_currentIndex);
    Phrase *phrase = new Phrase(currentMessage->text(), currentMessage->translation(),
                                QString(), nullptr);

    phraseBookHash.value(selectedPhraseBook)->append(phrase);
}

void MainWindow::resetSorting()
{
    m_contextView->sortByColumn(-1, Qt::AscendingOrder);
    m_messageView->sortByColumn(-1, Qt::AscendingOrder);
}

void MainWindow::manual()
{
    if (!m_assistantProcess)
        m_assistantProcess = new QProcess();

    if (m_assistantProcess->state() != QProcess::Running) {
        QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath) + QDir::separator();
#if !defined(Q_OS_MAC)
        app += QLatin1String("assistant");
#else
        app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif

        m_assistantProcess->start(app, QStringList() << QLatin1String("-enableRemoteControl"));
        if (!m_assistantProcess->waitForStarted()) {
            QMessageBox::critical(this, tr("Qt Linguist"),
                tr("Unable to launch Qt Assistant (%1)").arg(app));
            return;
        }
    }
    QTextStream str(m_assistantProcess);
    str << QLatin1String("SetSource qthelp://org.qt-project.linguist.")
        << (QT_VERSION >> 16) << ((QT_VERSION >> 8) & 0xFF)
        << (QT_VERSION & 0xFF)
        << QLatin1String("/qtlinguist/qtlinguist-index.html")
        << QLatin1Char('\n') << Qt::endl;
}

void MainWindow::about()
{
    QMessageBox box(this);
    box.setTextFormat(Qt::RichText);
    QString version = tr("Version %1");
    version = version.arg(QLatin1String(QT_VERSION_STR));

    const QString description
            = tr("Qt Linguist is a tool for adding translations to Qt applications.");
    const QString copyright
            = tr("Copyright (C) %1 The Qt Company Ltd.").arg(QStringLiteral("2023"));
    box.setText(QStringLiteral("<center><img src=\":/images/icons/linguist-128-32.png\"/></img><p>%1</p></center>"
                               "<p>%2</p>"
                               "<p>%3</p>").arg(version, description, copyright));

    box.setWindowTitle(QApplication::translate("AboutDialog", "Qt Linguist"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Linguist"));
}

void MainWindow::setupPhrase()
{
    bool enabled = !m_phraseBooks.isEmpty();
    m_ui.menuClosePhraseBook->setEnabled(enabled);
    m_ui.menuEditPhraseBook->setEnabled(enabled);
    m_ui.menuPrintPhraseBook->setEnabled(enabled);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSaveAll() && maybeSavePhraseBooks())
        e->accept();
    else
        e->ignore();
}

bool MainWindow::maybeSaveAll()
{
    if (!m_dataModel->isModified())
        return true;

    switch (QMessageBox::information(this, tr("Qt Linguist"),
        tr("Do you want to save the modified files?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
    {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Yes:
            saveAll();
            return !m_dataModel->isModified();
        default:
            break;
    }
    return true;
}

bool MainWindow::maybeSave(int model)
{
    if (!m_dataModel->isModified(model))
        return true;

    switch (QMessageBox::information(this, tr("Qt Linguist"),
        tr("Do you want to save '%1'?").arg(m_dataModel->srcFileName(model, true)),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
    {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Yes:
            saveInternal(model);
            return !m_dataModel->isModified(model);
        default:
            break;
    }
    return true;
}

void MainWindow::updateCaption()
{
    QString cap;
    bool enable = false;
    bool enableRw = false;
    for (int i = 0; i < m_dataModel->modelCount(); ++i) {
        enable = true;
        if (m_dataModel->isModelWritable(i)) {
            enableRw = true;
            break;
        }
    }
    m_ui.actionSaveAll->setEnabled(enableRw);
    m_ui.actionReleaseAll->setEnabled(enableRw);
    m_ui.actionCloseAll->setEnabled(enable);
    m_ui.actionPrint->setEnabled(enable);
    m_ui.actionAccelerators->setEnabled(enable);
    m_ui.actionSurroundingWhitespace->setEnabled(enable);
    m_ui.actionEndingPunctuation->setEnabled(enable);
    m_ui.actionPhraseMatches->setEnabled(enable);
    m_ui.actionPlaceMarkerMatches->setEnabled(enable);
    m_ui.actionResetSorting->setEnabled(enable);

    updateActiveModel(m_messageEditor->activeModel());
    // Ensure that the action labels get updated
    m_fileActiveModel = m_editActiveModel = -2;

    if (!enable)
        cap = tr("Qt Linguist[*]");
    else
        cap = tr("%1[*] - Qt Linguist").arg(m_dataModel->condensedSrcFileNames(true));
    setWindowTitle(cap);
}

void MainWindow::selectedContextChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex)
{
    if (sortedIndex.isValid()) {
        if (m_settingCurrentMessage)
            return; // Avoid playing ping-pong with the current message

        QModelIndex sourceIndex = m_sortedContextsModel->mapToSource(sortedIndex);
        if (m_messageModel->parent(currentMessageIndex()).row() == sourceIndex.row())
            return;

        QModelIndex contextIndex = setMessageViewRoot(sourceIndex);
        const QModelIndex &firstChild =
                m_sortedMessagesModel->index(0, sourceIndex.column(), contextIndex);
        m_messageView->setCurrentIndex(firstChild);
    } else if (oldIndex.isValid()) {
        m_contextView->setCurrentIndex(oldIndex);
    }
}

/*
 * Updates the message displayed in the message editor and related actions.
 */
void MainWindow::selectedMessageChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex)
{
    // Keep a valid selection whenever possible
    if (!sortedIndex.isValid() && oldIndex.isValid()) {
        m_messageView->setCurrentIndex(oldIndex);
        return;
    }

    int model = -1;
    MessageItem *m = 0;
    QModelIndex index = m_sortedMessagesModel->mapToSource(sortedIndex);
    if (index.isValid()) {
        model = (index.column() && (index.column() - 1 < m_dataModel->modelCount())) ?
                        index.column() - 1 : m_currentIndex.model();
        m_currentIndex = m_messageModel->dataIndex(index, model);
        m_messageEditor->showMessage(m_currentIndex);
        if (model >= 0 && (m = m_dataModel->messageItem(m_currentIndex))) {
            if (m_dataModel->isModelWritable(model) && !m->isObsolete())
                m_phraseView->setSourceText(m_currentIndex.model(), m->text());
            else
                m_phraseView->setSourceText(-1, QString());
        } else {
            if (model < 0) {
                model = m_dataModel->multiContextItem(m_currentIndex.context())
                                ->firstNonobsoleteMessageIndex(m_currentIndex.message());
                if (model >= 0)
                    m = m_dataModel->messageItem(m_currentIndex, model);
            }
            m_phraseView->setSourceText(-1, QString());
        }
        m_errorsView->setEnabled(m != 0);
        updateDanger(m_currentIndex, true);
    } else {
        m_currentIndex = MultiDataIndex();
        m_messageEditor->showNothing();
        m_phraseView->setSourceText(-1, QString());
    }
    updateSourceView(model, m);

    updatePhraseBookActions();
    m_ui.actionSelectAll->setEnabled(index.isValid());
}

void MainWindow::translationChanged(const MultiDataIndex &index)
{
    // We get that as a result of batch translation or search & translate,
    // so the current model is known to match.
    if (index != m_currentIndex)
        return;

    m_messageEditor->showMessage(index);
    updateDanger(index, true);

    MessageItem *m = m_dataModel->messageItem(index);
    if (hasFormPreview(m->fileName()))
        m_formPreviewView->setSourceContext(index.model(), m);
}

// This and the following function operate directly on the messageitem,
// so the model does not emit modification notifications.
void MainWindow::updateTranslation(const QStringList &translations)
{
    MessageItem *m = m_dataModel->messageItem(m_currentIndex);
    if (!m)
        return;
    if (translations == m->translations())
        return;

    m->setTranslations(translations);
    if (!m->fileName().isEmpty() && hasFormPreview(m->fileName()))
        m_formPreviewView->setSourceContext(m_currentIndex.model(), m);
    updateDanger(m_currentIndex, true);

    if (m->isFinished())
        m_dataModel->setFinished(m_currentIndex, false);
    else
        m_dataModel->setModified(m_currentIndex.model(), true);
}

void MainWindow::updateTranslatorComment(const QString &comment)
{
    MessageItem *m = m_dataModel->messageItem(m_currentIndex);
    if (!m)
        return;
    if (comment == m->translatorComment())
        return;

    m->setTranslatorComment(comment);

    m_dataModel->setModified(m_currentIndex.model(), true);
}

void MainWindow::refreshItemViews()
{
    m_messageModel->blockSignals(false);
    m_contextView->update();
    m_messageView->update();
    setWindowModified(m_dataModel->isModified());
    m_modifiedLabel->setVisible(m_dataModel->isModified());
    updateStatistics();
}

void MainWindow::done()
{
    int model = m_messageEditor->activeModel();
    if (model >= 0 && m_dataModel->isModelWritable(model))
        m_dataModel->setFinished(m_currentIndex, true);
}

void MainWindow::doneAndNext()
{
    done();
    if (!m_messageEditor->focusNextUnfinished())
        nextUnfinished();
}

void MainWindow::toggleFinished(const QModelIndex &index)
{
    if (!index.isValid() || index.column() - 1 >= m_dataModel->modelCount()
        || !m_dataModel->isModelWritable(index.column() - 1) || index.parent() == QModelIndex())
        return;

    QModelIndex item = m_sortedMessagesModel->mapToSource(index);
    MultiDataIndex dataIndex = m_messageModel->dataIndex(item);
    MessageItem *m = m_dataModel->messageItem(dataIndex);

    if (!m || m->message().type() == TranslatorMessage::Obsolete
           || m->message().type() == TranslatorMessage::Vanished)
        return;

    m_dataModel->setFinished(dataIndex, !m->isFinished());
}

/*
 * Receives a context index in the sorted messages model and returns the next
 * logical context index in the same model, based on the sort order of the
 * contexts in the sorted contexts model.
 */
QModelIndex MainWindow::nextContext(const QModelIndex &index) const
{
    QModelIndex sortedContextIndex = m_sortedContextsModel->mapFromSource(
            m_sortedMessagesModel->mapToSource(index));

    int nextRow = sortedContextIndex.row() + 1;
    if (nextRow >= m_sortedContextsModel->rowCount())
        nextRow = 0;
    sortedContextIndex = m_sortedContextsModel->index(nextRow, index.column());

    return m_sortedMessagesModel->mapFromSource(
            m_sortedContextsModel->mapToSource(sortedContextIndex));
}

/*
 * See nextContext.
 */
QModelIndex MainWindow::prevContext(const QModelIndex &index) const
{
    QModelIndex sortedContextIndex = m_sortedContextsModel->mapFromSource(
            m_sortedMessagesModel->mapToSource(index));

    int prevRow = sortedContextIndex.row() - 1;
    if (prevRow < 0) prevRow = m_sortedContextsModel->rowCount() - 1;
    sortedContextIndex = m_sortedContextsModel->index(prevRow, index.column());

    return m_sortedMessagesModel->mapFromSource(
            m_sortedContextsModel->mapToSource(sortedContextIndex));
}

QModelIndex MainWindow::nextMessage(const QModelIndex &currentIndex, bool checkUnfinished) const
{
    QModelIndex idx = currentIndex.isValid() ? currentIndex : m_sortedMessagesModel->index(0, 0);
    do {
        int row = 0;
        QModelIndex par = idx.parent();
        if (par.isValid()) {
            row = idx.row() + 1;
        } else {        // In case we are located on a top-level node
            par = idx;
        }

        if (row >= m_sortedMessagesModel->rowCount(par)) {
            par = nextContext(par);
            row = 0;
        }
        idx = m_sortedMessagesModel->index(row, idx.column(), par);

        if (!checkUnfinished)
            return idx;

        QModelIndex item = m_sortedMessagesModel->mapToSource(idx);
        MultiDataIndex index = m_messageModel->dataIndex(item, -1);
        if (m_dataModel->multiMessageItem(index)->isUnfinished())
            return idx;
    } while (idx != currentIndex);
    return QModelIndex();
}

QModelIndex MainWindow::prevMessage(const QModelIndex &currentIndex, bool checkUnfinished) const
{
    QModelIndex idx = currentIndex.isValid() ? currentIndex : m_sortedMessagesModel->index(0, 0);
    do {
        int row = idx.row() - 1;
        QModelIndex par = idx.parent();
        if (!par.isValid()) {   // In case we are located on a top-level node
            par = idx;
            row = -1;
        }

        if (row < 0) {
            par = prevContext(par);
            row = m_sortedMessagesModel->rowCount(par) - 1;
        }
        idx = m_sortedMessagesModel->index(row, idx.column(), par);

        if (!checkUnfinished)
            return idx;

        QModelIndex item = m_sortedMessagesModel->mapToSource(idx);
        MultiDataIndex index = m_messageModel->dataIndex(item, -1);
        if (m_dataModel->multiMessageItem(index)->isUnfinished())
            return idx;
    } while (idx != currentIndex);
    return QModelIndex();
}

void MainWindow::nextUnfinished()
{
    if (m_ui.actionNextUnfinished->isEnabled()) {
        if (!doNext(true)) {
            // If no Unfinished message is left, the user has finished the job.  We
            // congratulate on a job well done with this ringing bell.
            statusBar()->showMessage(tr("No untranslated translation units left."), MessageMS);
            qApp->beep();
        }
    }
}

void MainWindow::prevUnfinished()
{
    if (m_ui.actionNextUnfinished->isEnabled()) {
        if (!doPrev(true)) {
            // If no Unfinished message is left, the user has finished the job.  We
            // congratulate on a job well done with this ringing bell.
            statusBar()->showMessage(tr("No untranslated translation units left."), MessageMS);
            qApp->beep();
        }
    }
}

void MainWindow::prev()
{
    doPrev(false);
}

void MainWindow::next()
{
    doNext(false);
}

bool MainWindow::doPrev(bool checkUnfinished)
{
    QModelIndex index = prevMessage(m_messageView->currentIndex(), checkUnfinished);
    if (index.isValid())
        setCurrentMessage(m_sortedMessagesModel->mapToSource(index));
    if (checkUnfinished)
        m_messageEditor->setUnfinishedEditorFocus();
    else
        m_messageEditor->setEditorFocus();
    return index.isValid();
}

bool MainWindow::doNext(bool checkUnfinished)
{
    QModelIndex index = nextMessage(m_messageView->currentIndex(), checkUnfinished);
    if (index.isValid())
        setCurrentMessage(m_sortedMessagesModel->mapToSource(index));
    if (checkUnfinished)
        m_messageEditor->setUnfinishedEditorFocus();
    else
        m_messageEditor->setEditorFocus();
    return index.isValid();
}

void MainWindow::findNext(const QString &text, DataModel::FindLocation where,
                          bool matchCase, bool ignoreAccelerators, bool skipObsolete, bool useRegExp)
{
    if (text.isEmpty())
        return;
    m_findText = text;
    m_findWhere = where;
    m_findMatchCase = matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
    m_findIgnoreAccelerators = ignoreAccelerators;
    m_findSkipObsolete = skipObsolete;
    m_findUseRegExp = useRegExp;
    if (m_findUseRegExp) {
        m_findDialog->getRegExp().setPatternOptions(matchCase
                                                    ? QRegularExpression::NoPatternOption
                                                    : QRegularExpression::CaseInsensitiveOption);
    }
    m_ui.actionFindNext->setEnabled(true);
    findAgain();
}

void MainWindow::revalidate()
{
    for (MultiDataModelIterator it(m_dataModel, -1); it.isValid(); ++it)
        updateDanger(it, false);

    if (m_currentIndex.isValid())
        updateDanger(m_currentIndex, true);
}

QString MainWindow::friendlyString(const QString& str)
{
    QString f = str.toLower();
    f.replace(QRegularExpression(QString(QLatin1String("[.,:;!?()-]"))), QString(QLatin1String(" ")));
    f.remove(QLatin1Char('&'));
    return f.simplified();
}

void MainWindow::setupMenuBar()
{

    const bool hasThemeIcons = !QApplication::platformName().compare(QStringLiteral("xcb"), Qt::CaseInsensitive);
    if (hasThemeIcons) {  // There are no fallback icons for these
        m_ui.menuRecentlyOpenedFiles->setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent")));
        m_ui.actionCloseAll->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        m_ui.actionExit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
        m_ui.actionSelectAll->setIcon(QIcon::fromTheme(QStringLiteral("edit-select-all")));
    }

    // Prefer theme icons when available for these actions
    const QString prefix = QApplication::platformName().compare(QStringLiteral("cocoa"), Qt::CaseInsensitive) ?
                           QStringLiteral(":/images/win") : QStringLiteral(":/images/mac");

    m_ui.actionOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open"),
                                              QIcon(prefix + QStringLiteral("/fileopen.png"))));
    m_ui.actionOpenAux->setIcon(QIcon::fromTheme(QStringLiteral("document-open"),
                                                 QIcon(prefix + QStringLiteral("/fileopen.png"))));
    m_ui.actionSave->setIcon(QIcon::fromTheme(QStringLiteral("document-save"),
                                              QIcon(prefix + QStringLiteral("/filesave.png"))));
    m_ui.actionSaveAll->setIcon(QIcon::fromTheme(QStringLiteral("document-save"),
                                                 QIcon(prefix + QStringLiteral("/filesave.png"))));
    m_ui.actionPrint->setIcon(QIcon::fromTheme(QStringLiteral("document-print"),
                                               QIcon(prefix + QStringLiteral("/print.png"))));
    m_ui.actionRedo->setIcon(QIcon::fromTheme(QStringLiteral("edit-redo"),
                                              QIcon(prefix + QStringLiteral("/redo.png"))));
    m_ui.actionUndo->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo"),
                                              QIcon(prefix + QStringLiteral("/undo.png"))));
    m_ui.actionCut->setIcon(QIcon::fromTheme(QStringLiteral("edit-cut"),
                                             QIcon(prefix + QStringLiteral("/editcut.png"))));
    m_ui.actionCopy->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy"),
                                              QIcon(prefix + QStringLiteral("/editcopy.png"))));
    m_ui.actionPaste->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste"),
                                               QIcon(prefix + QStringLiteral("/editpaste.png"))));
    m_ui.actionFind->setIcon(QIcon::fromTheme(QStringLiteral("edit-find"),
                                              QIcon(prefix + QStringLiteral("/searchfind.png"))));

    // No well defined theme icons for these actions
    m_ui.actionAccelerators->setIcon(QIcon(prefix + QStringLiteral("/accelerator.png")));
    m_ui.actionOpenPhraseBook->setIcon(QIcon(prefix + QStringLiteral("/book.png")));
    m_ui.actionDone->setIcon(QIcon(prefix + QStringLiteral("/done.png")));
    m_ui.actionDoneAndNext->setIcon(QIcon(prefix + QStringLiteral("/doneandnext.png")));
    m_ui.actionNext->setIcon(QIcon(prefix + QStringLiteral("/next.png")));
    m_ui.actionNextUnfinished->setIcon(QIcon(prefix + QStringLiteral("/nextunfinished.png")));
    m_ui.actionPhraseMatches->setIcon(QIcon(prefix + QStringLiteral("/phrase.png")));
    m_ui.actionSurroundingWhitespace->setIcon(QIcon(prefix + QStringLiteral("/surroundingwhitespace.png")));
    m_ui.actionEndingPunctuation->setIcon(QIcon(prefix + QStringLiteral("/punctuation.png")));
    m_ui.actionPrev->setIcon(QIcon(prefix + QStringLiteral("/prev.png")));
    m_ui.actionPrevUnfinished->setIcon(QIcon(prefix + QStringLiteral("/prevunfinished.png")));
    m_ui.actionPlaceMarkerMatches->setIcon(QIcon(prefix + QStringLiteral("/validateplacemarkers.png")));
    m_ui.actionWhatsThis->setIcon(QIcon(prefix + QStringLiteral("/whatsthis.png")));

    // File menu
    connect(m_ui.menuFile, &QMenu::aboutToShow, this, &MainWindow::fileAboutToShow);
    connect(m_ui.actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(m_ui.actionOpenAux, &QAction::triggered, this, &MainWindow::openAux);
    connect(m_ui.actionSaveAll, &QAction::triggered, this, &MainWindow::saveAll);
    connect(m_ui.actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(m_ui.actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(m_ui.actionReleaseAll, &QAction::triggered, this, &MainWindow::releaseAll);
    connect(m_ui.actionRelease, &QAction::triggered, this, &MainWindow::release);
    connect(m_ui.actionReleaseAs, &QAction::triggered, this, &MainWindow::releaseAs);
    connect(m_ui.actionPrint, &QAction::triggered, this, &MainWindow::print);
    connect(m_ui.actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(m_ui.actionCloseAll, &QAction::triggered, this, &MainWindow::closeAll);
    connect(m_ui.actionExit, &QAction::triggered, this, &MainWindow::close);

    // Edit menu
    connect(m_ui.menuEdit, &QMenu::aboutToShow, this, &MainWindow::editAboutToShow);

    connect(m_ui.actionUndo, &QAction::triggered, m_messageEditor, &MessageEditor::undo);
    connect(m_messageEditor, &MessageEditor::undoAvailable, m_ui.actionUndo, &QAction::setEnabled);

    connect(m_ui.actionRedo, &QAction::triggered, m_messageEditor, &MessageEditor::redo);
    connect(m_messageEditor, &MessageEditor::redoAvailable, m_ui.actionRedo, &QAction::setEnabled);

#ifndef QT_NO_CLIPBOARD
    connect(m_ui.actionCut, &QAction::triggered, m_messageEditor, &MessageEditor::cut);
    connect(m_messageEditor, &MessageEditor::cutAvailable, m_ui.actionCut, &QAction::setEnabled);

    connect(m_ui.actionCopy, &QAction::triggered, m_messageEditor, &MessageEditor::copy);
    connect(m_messageEditor, &MessageEditor::copyAvailable, m_ui.actionCopy, &QAction::setEnabled);

    connect(m_ui.actionPaste, &QAction::triggered, m_messageEditor, &MessageEditor::paste);
    connect(m_messageEditor, &MessageEditor::pasteAvailable, m_ui.actionPaste, &QAction::setEnabled);
#endif

    connect(m_ui.actionSelectAll, &QAction::triggered,
            m_messageEditor, &MessageEditor::selectAll);
    connect(m_ui.actionFind, &QAction::triggered,
            m_findDialog, &FindDialog::find);
    connect(m_ui.actionFindNext, &QAction::triggered,
            this, &MainWindow::findAgain);
    connect(m_ui.actionSearchAndTranslate, &QAction::triggered,
            this, &MainWindow::showTranslateDialog);
    connect(m_ui.actionBatchTranslation, &QAction::triggered,
            this, &MainWindow::showBatchTranslateDialog);
    connect(m_ui.actionTranslationFileSettings, &QAction::triggered,
            this, &MainWindow::showTranslationSettings);

    connect(m_batchTranslateDialog, &BatchTranslationDialog::finished,
            this, &MainWindow::refreshItemViews);

    // Translation menu
    // when updating the accelerators, remember the status bar
    connect(m_ui.actionPrevUnfinished, &QAction::triggered, this, &MainWindow::prevUnfinished);
    connect(m_ui.actionNextUnfinished, &QAction::triggered, this, &MainWindow::nextUnfinished);
    connect(m_ui.actionNext, &QAction::triggered, this, &MainWindow::next);
    connect(m_ui.actionPrev, &QAction::triggered, this, &MainWindow::prev);
    connect(m_ui.actionDone, &QAction::triggered, this, &MainWindow::done);
    connect(m_ui.actionDoneAndNext, &QAction::triggered, this, &MainWindow::doneAndNext);
    connect(m_ui.actionBeginFromSource, &QAction::triggered, m_messageEditor, &MessageEditor::beginFromSource);
    connect(m_messageEditor, &MessageEditor::beginFromSourceAvailable,
            m_ui.actionBeginFromSource, &QAction::setEnabled);

    // Phrasebook menu
    connect(m_ui.actionNewPhraseBook, &QAction::triggered, this, &MainWindow::newPhraseBook);
    connect(m_ui.actionOpenPhraseBook, &QAction::triggered, this, &MainWindow::openPhraseBook);
    connect(m_ui.menuClosePhraseBook, &QMenu::triggered,
            this, &MainWindow::closePhraseBook);
    connect(m_ui.menuEditPhraseBook, &QMenu::triggered,
            this, &MainWindow::editPhraseBook);
    connect(m_ui.menuPrintPhraseBook, &QMenu::triggered,
            this, &MainWindow::printPhraseBook);
    connect(m_ui.actionAddToPhraseBook, &QAction::triggered,
            this, &MainWindow::addToPhraseBook);

    // Validation menu
    connect(m_ui.actionAccelerators, &QAction::triggered, this, &MainWindow::revalidate);
    connect(m_ui.actionSurroundingWhitespace, &QAction::triggered, this, &MainWindow::revalidate);
    connect(m_ui.actionEndingPunctuation, &QAction::triggered, this, &MainWindow::revalidate);
    connect(m_ui.actionPhraseMatches, &QAction::triggered, this, &MainWindow::revalidate);
    connect(m_ui.actionPlaceMarkerMatches, &QAction::triggered, this, &MainWindow::revalidate);

    // View menu
    connect(m_ui.actionResetSorting, &QAction::triggered,
            this, &MainWindow::resetSorting);
    connect(m_ui.actionDisplayGuesses, &QAction::triggered,
            m_phraseView, &PhraseView::toggleGuessing);
    connect(m_ui.actionStatistics, &QAction::triggered,
            this, &MainWindow::toggleStatistics);
    connect(m_ui.actionVisualizeWhitespace, &QAction::triggered,
            this, &MainWindow::toggleVisualizeWhitespace);
    connect(m_ui.menuView, &QMenu::aboutToShow,
            this, &MainWindow::updateViewMenu);
    connect(m_ui.actionIncreaseZoom, &QAction::triggered,
            m_messageEditor, &MessageEditor::increaseFontSize);
    connect(m_ui.actionDecreaseZoom, &QAction::triggered,
            m_messageEditor, &MessageEditor::decreaseFontSize);
    connect(m_ui.actionResetZoomToDefault, &QAction::triggered,
            m_messageEditor, &MessageEditor::resetFontSize);
    connect(m_ui.actionShowMoreGuesses, &QAction::triggered,
            m_phraseView, &PhraseView::moreGuesses);
    connect(m_ui.actionShowFewerGuesses, &QAction::triggered,
            m_phraseView, &PhraseView::fewerGuesses);
    connect(m_phraseView, &PhraseView::showFewerGuessesAvailable,
            m_ui.actionShowFewerGuesses, &QAction::setEnabled);
    connect(m_ui.actionResetGuessesToDefault, &QAction::triggered,
            m_phraseView, &PhraseView::resetNumGuesses);
    m_ui.menuViewViews->addAction(m_contextDock->toggleViewAction());
    m_ui.menuViewViews->addAction(m_messagesDock->toggleViewAction());
    m_ui.menuViewViews->addAction(m_phrasesDock->toggleViewAction());
    m_ui.menuViewViews->addAction(m_sourceAndFormDock->toggleViewAction());
    m_ui.menuViewViews->addAction(m_errorsDock->toggleViewAction());

#if defined(Q_OS_MAC)
    // Window menu
    QMenu *windowMenu = new QMenu(tr("&Window"), this);
    menuBar()->insertMenu(m_ui.menuHelp->menuAction(), windowMenu);
    windowMenu->addAction(tr("Minimize"), this,
        &QWidget::showMinimized, QKeySequence(tr("Ctrl+M")));
#endif

    // Help
    connect(m_ui.actionManual, &QAction::triggered, this, &MainWindow::manual);
    connect(m_ui.actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui.actionAboutQt, &QAction::triggered, this, &MainWindow::aboutQt);
    connect(m_ui.actionWhatsThis, &QAction::triggered, this, &MainWindow::onWhatsThis);

    connect(m_ui.menuRecentlyOpenedFiles, &QMenu::triggered,
            this, &MainWindow::recentFileActivated);

    m_ui.actionManual->setWhatsThis(tr("Display the manual for %1.").arg(tr("Qt Linguist")));
    m_ui.actionAbout->setWhatsThis(tr("Display information about %1.").arg(tr("Qt Linguist")));
    m_ui.actionDone->setShortcuts(QList<QKeySequence>()
                                     << QKeySequence(QLatin1String("Alt+Return"))
                                     << QKeySequence(QLatin1String("Alt+Enter")));
    m_ui.actionDoneAndNext->setShortcuts(QList<QKeySequence>()
                                            << QKeySequence(QLatin1String("Ctrl+Return"))
                                            << QKeySequence(QLatin1String("Ctrl+Enter")));

    // Disable the Close/Edit/Print phrasebook menuitems if they are not loaded
    connect(m_ui.menuPhrases, &QMenu::aboutToShow, this, &MainWindow::setupPhrase);

    connect(m_ui.menuRecentlyOpenedFiles, &QMenu::aboutToShow,
            this, &MainWindow::setupRecentFilesMenu);
}

void MainWindow::updateActiveModel(int model)
{
    if (model >= 0)
        doUpdateLatestModel(model);
}

// Arriving here implies that the messageEditor does not have focus
void MainWindow::updateLatestModel(const QModelIndex &index)
{
    if (index.column() && (index.column() - 1 < m_dataModel->modelCount()))
        doUpdateLatestModel(index.column() - 1);
}

void MainWindow::doUpdateLatestModel(int model)
{
    m_currentIndex = MultiDataIndex(model, m_currentIndex.context(), m_currentIndex.message());
    bool enable = false;
    bool enableRw = false;
    MessageItem *item = 0;
    if (model >= 0) {
        enable = true;
        if (m_dataModel->isModelWritable(model))
            enableRw = true;

        if (m_currentIndex.isValid()) {
            if ((item = m_dataModel->messageItem(m_currentIndex))) {
                if (enableRw && !item->isObsolete())
                    m_phraseView->setSourceText(model, item->text());
                else
                    m_phraseView->setSourceText(-1, QString());
            } else {
                m_phraseView->setSourceText(-1, QString());
            }
        }
    }
    updateSourceView(model, item);
    m_ui.actionSave->setEnabled(enableRw);
    m_ui.actionSaveAs->setEnabled(enableRw);
    m_ui.actionRelease->setEnabled(enableRw);
    m_ui.actionReleaseAs->setEnabled(enableRw);
    m_ui.actionClose->setEnabled(enable);
    m_ui.actionTranslationFileSettings->setEnabled(enableRw);
    m_ui.actionSearchAndTranslate->setEnabled(enableRw);
    // cut & paste - edit only
    updatePhraseBookActions();
    updateStatistics();
}

void MainWindow::updateSourceView(int model, MessageItem *item)
{
    if (item && !item->fileName().isEmpty()) {
        if (hasFormPreview(item->fileName())) {
            m_sourceAndFormView->setCurrentWidget(m_formPreviewView);
            m_formPreviewView->setSourceContext(model, item);
        } else {
            m_sourceAndFormView->setCurrentWidget(m_sourceCodeView);
            QDir dir = QFileInfo(m_dataModel->srcFileName(model)).dir();
            QString fileName = QDir::cleanPath(dir.absoluteFilePath(item->fileName()));
            m_sourceCodeView->setSourceContext(fileName, item->lineNumber());
        }
    } else {
        m_sourceAndFormView->setCurrentWidget(m_sourceCodeView);
        m_sourceCodeView->setSourceContext(QString(), 0);
    }
}

// Note for *AboutToShow: Due to the delayed nature, only actions without shortcuts
// and representations outside the menu may be setEnabled()/setVisible() here.

void MainWindow::fileAboutToShow()
{
    if (m_fileActiveModel != m_currentIndex.model()) {
        // We rename the actions so the shortcuts need not be reassigned.
        bool en;
        if (m_dataModel->modelCount() > 1) {
            if (m_currentIndex.model() >= 0) {
                QString fn = QFileInfo(m_dataModel->srcFileName(m_currentIndex.model())).baseName();
                m_ui.actionSave->setText(tr("&Save '%1'").arg(fn));
                m_ui.actionSaveAs->setText(tr("Save '%1' &As...").arg(fn));
                m_ui.actionRelease->setText(tr("Release '%1'").arg(fn));
                m_ui.actionReleaseAs->setText(tr("Release '%1' As...").arg(fn));
                m_ui.actionClose->setText(tr("&Close '%1'").arg(fn));
            } else {
                m_ui.actionSave->setText(tr("&Save"));
                m_ui.actionSaveAs->setText(tr("Save &As..."));
                m_ui.actionRelease->setText(tr("Release"));
                m_ui.actionReleaseAs->setText(tr("Release As..."));
                m_ui.actionClose->setText(tr("&Close"));
            }

            m_ui.actionSaveAll->setText(tr("Save All"));
            m_ui.actionReleaseAll->setText(tr("&Release All"));
            m_ui.actionCloseAll->setText(tr("Close All"));
            en = true;
        } else {
            m_ui.actionSaveAs->setText(tr("Save &As..."));
            m_ui.actionReleaseAs->setText(tr("Release As..."));

            m_ui.actionSaveAll->setText(tr("&Save"));
            m_ui.actionReleaseAll->setText(tr("&Release"));
            m_ui.actionCloseAll->setText(tr("&Close"));
            en = false;
        }
        m_ui.actionSave->setVisible(en);
        m_ui.actionRelease->setVisible(en);
        m_ui.actionClose->setVisible(en);
        m_fileActiveModel = m_currentIndex.model();
    }
}

void MainWindow::editAboutToShow()
{
    if (m_editActiveModel != m_currentIndex.model()) {
        if (m_currentIndex.model() >= 0 && m_dataModel->modelCount() > 1) {
            QString fn = QFileInfo(m_dataModel->srcFileName(m_currentIndex.model())).baseName();
            m_ui.actionTranslationFileSettings->setText(tr("Translation File &Settings for '%1'...").arg(fn));
            m_ui.actionBatchTranslation->setText(tr("&Batch Translation of '%1'...").arg(fn));
            m_ui.actionSearchAndTranslate->setText(tr("Search And &Translate in '%1'...").arg(fn));
        } else {
            m_ui.actionTranslationFileSettings->setText(tr("Translation File &Settings..."));
            m_ui.actionBatchTranslation->setText(tr("&Batch Translation..."));
            m_ui.actionSearchAndTranslate->setText(tr("Search And &Translate..."));
        }
        m_editActiveModel = m_currentIndex.model();
    }
}

void MainWindow::updateViewMenu()
{
    bool check = m_statistics ? m_statistics->isVisible() : false;
    m_ui.actionStatistics->setChecked(check);
}

void MainWindow::showContextDock()
{
    m_contextDock->show();
    m_contextDock->raise();
}

void MainWindow::showMessagesDock()
{
    m_messagesDock->show();
    m_messagesDock->raise();
}

void MainWindow::showPhrasesDock()
{
    m_phrasesDock->show();
    m_phrasesDock->raise();
}

void MainWindow::showSourceCodeDock()
{
    m_sourceAndFormDock->show();
    m_sourceAndFormDock->raise();
}

void MainWindow::showErrorDock()
{
    m_errorsDock->show();
    m_errorsDock->raise();
}

void MainWindow::onWhatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}

void MainWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar(this);
    filet->setObjectName(QLatin1String("FileToolbar"));
    filet->setWindowTitle(tr("File"));
    this->addToolBar(filet);
    m_ui.menuToolbars->addAction(filet->toggleViewAction());

    QToolBar *editt = new QToolBar(this);
    editt->setVisible(false);
    editt->setObjectName(QLatin1String("EditToolbar"));
    editt->setWindowTitle(tr("Edit"));
    this->addToolBar(editt);
    m_ui.menuToolbars->addAction(editt->toggleViewAction());

    QToolBar *translationst = new QToolBar(this);
    translationst->setObjectName(QLatin1String("TranslationToolbar"));
    translationst->setWindowTitle(tr("Translation"));
    this->addToolBar(translationst);
    m_ui.menuToolbars->addAction(translationst->toggleViewAction());

    QToolBar *validationt = new QToolBar(this);
    validationt->setObjectName(QLatin1String("ValidationToolbar"));
    validationt->setWindowTitle(tr("Validation"));
    this->addToolBar(validationt);
    m_ui.menuToolbars->addAction(validationt->toggleViewAction());

    QToolBar *helpt = new QToolBar(this);
    helpt->setVisible(false);
    helpt->setObjectName(QLatin1String("HelpToolbar"));
    helpt->setWindowTitle(tr("Help"));
    this->addToolBar(helpt);
    m_ui.menuToolbars->addAction(helpt->toggleViewAction());


    filet->addAction(m_ui.actionOpen);
    filet->addAction(m_ui.actionSaveAll);
    filet->addAction(m_ui.actionPrint);
    filet->addSeparator();
    filet->addAction(m_ui.actionOpenPhraseBook);

    editt->addAction(m_ui.actionUndo);
    editt->addAction(m_ui.actionRedo);
    editt->addSeparator();
    editt->addAction(m_ui.actionCut);
    editt->addAction(m_ui.actionCopy);
    editt->addAction(m_ui.actionPaste);
    editt->addSeparator();
    editt->addAction(m_ui.actionFind);

    translationst->addAction(m_ui.actionPrev);
    translationst->addAction(m_ui.actionNext);
    translationst->addAction(m_ui.actionPrevUnfinished);
    translationst->addAction(m_ui.actionNextUnfinished);
    translationst->addAction(m_ui.actionDone);
    translationst->addAction(m_ui.actionDoneAndNext);

    validationt->addAction(m_ui.actionAccelerators);
    validationt->addAction(m_ui.actionSurroundingWhitespace);
    validationt->addAction(m_ui.actionEndingPunctuation);
    validationt->addAction(m_ui.actionPhraseMatches);
    validationt->addAction(m_ui.actionPlaceMarkerMatches);

    helpt->addAction(m_ui.actionWhatsThis);
}

QModelIndex MainWindow::setMessageViewRoot(const QModelIndex &index)
{
    const QModelIndex &sortedContextIndex = m_sortedMessagesModel->mapFromSource(index);
    const QModelIndex &trueContextIndex = m_sortedMessagesModel->index(sortedContextIndex.row(), 0);
    if (m_messageView->rootIndex() != trueContextIndex)
        m_messageView->setRootIndex(trueContextIndex);
    return trueContextIndex;
}

/*
 * Updates the selected entries in the context and message views.
 */
void MainWindow::setCurrentMessage(const QModelIndex &index)
{
    const QModelIndex &contextIndex = m_messageModel->parent(index);
    if (!contextIndex.isValid())
        return;

    const QModelIndex &trueIndex = m_messageModel->index(contextIndex.row(), index.column(), QModelIndex());
    m_settingCurrentMessage = true;
    m_contextView->setCurrentIndex(m_sortedContextsModel->mapFromSource(trueIndex));
    m_settingCurrentMessage = false;

    setMessageViewRoot(contextIndex);
    m_messageView->setCurrentIndex(m_sortedMessagesModel->mapFromSource(index));
}

void MainWindow::setCurrentMessage(const QModelIndex &index, int model)
{
    const QModelIndex &theIndex = m_messageModel->index(index.row(), model + 1, index.parent());
    setCurrentMessage(theIndex);
    m_messageEditor->setEditorFocusForModel(model);
}

void MainWindow::setCurrentMessageFromGuess(int modelIndex, const Candidate &cand)
{
    int contextIndex = m_dataModel->findContextIndex(cand.context);
    int messageIndex = m_dataModel->multiContextItem(contextIndex)->findMessage(cand.source,
                                                                                cand.disambiguation);
    setCurrentMessage(m_messageModel->modelIndex(MultiDataIndex(modelIndex, contextIndex,
                                                                messageIndex)));
}

QModelIndex MainWindow::currentContextIndex() const
{
    return m_sortedContextsModel->mapToSource(m_contextView->currentIndex());
}

QModelIndex MainWindow::currentMessageIndex() const
{
    return m_sortedMessagesModel->mapToSource(m_messageView->currentIndex());
}

PhraseBook *MainWindow::doOpenPhraseBook(const QString& name)
{
    PhraseBook *pb = new PhraseBook();
    bool langGuessed;
    if (!pb->load(name, &langGuessed)) {
        QMessageBox::warning(this, tr("Qt Linguist"),
            tr("Cannot read from phrase book '%1'.").arg(name));
        delete pb;
        return 0;
    }
    if (langGuessed) {
        if (!m_translationSettingsDialog)
            m_translationSettingsDialog = new TranslationSettingsDialog(this);
        m_translationSettingsDialog->setPhraseBook(pb);
        m_translationSettingsDialog->exec();
    }

    m_phraseBooks.append(pb);

    QAction *a = m_ui.menuClosePhraseBook->addAction(pb->friendlyPhraseBookName());
    m_phraseBookMenu[PhraseCloseMenu].insert(a, pb);
    a->setWhatsThis(tr("Close this phrase book."));

    a = m_ui.menuEditPhraseBook->addAction(pb->friendlyPhraseBookName());
    m_phraseBookMenu[PhraseEditMenu].insert(a, pb);
    a->setWhatsThis(tr("Enables you to add, modify, or delete"
        " entries in this phrase book."));

    a = m_ui.menuPrintPhraseBook->addAction(pb->friendlyPhraseBookName());
    m_phraseBookMenu[PhrasePrintMenu].insert(a, pb);
    a->setWhatsThis(tr("Print the entries in this phrase book."));

    connect(pb, &PhraseBook::listChanged, this, &MainWindow::updatePhraseDicts);
    updatePhraseDicts();
    updatePhraseBookActions();

    return pb;
}

bool MainWindow::savePhraseBook(QString *name, PhraseBook &pb)
{
    if (!name->contains(QLatin1Char('.')))
        *name += QLatin1String(".qph");

    if (!pb.save(*name)) {
        QMessageBox::warning(this, tr("Qt Linguist"),
            tr("Cannot create phrase book '%1'.").arg(*name));
        return false;
    }
    return true;
}

bool MainWindow::maybeSavePhraseBook(PhraseBook *pb)
{
    if (pb->isModified())
        switch (QMessageBox::information(this, tr("Qt Linguist"),
            tr("Do you want to save phrase book '%1'?").arg(pb->friendlyPhraseBookName()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
        {
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Yes:
                if (!pb->save(pb->fileName()))
                    return false;
                break;
            default:
                break;
        }
    return true;
}

bool MainWindow::maybeSavePhraseBooks()
{
    for (PhraseBook *phraseBook : qAsConst(m_phraseBooks))
        if (!maybeSavePhraseBook(phraseBook))
            return false;
    return true;
}

void MainWindow::updateProgress()
{
    int numEditable = m_dataModel->getNumEditable();
    int numFinished = m_dataModel->getNumFinished();
    if (!m_dataModel->modelCount()) {
        m_progressLabel->setText(QString(QLatin1String("    ")));
        m_progressLabel->setToolTip(QString());
    } else {
        m_progressLabel->setText(QStringLiteral(" %1/%2 ").arg(numFinished).arg(numEditable));
        m_progressLabel->setToolTip(tr("%n unfinished message(s) left.", 0,
                                       numEditable - numFinished));
    }
    bool enable = numFinished != numEditable;
    m_ui.actionPrevUnfinished->setEnabled(enable);
    m_ui.actionNextUnfinished->setEnabled(enable);
    m_ui.actionDone->setEnabled(enable);
    m_ui.actionDoneAndNext->setEnabled(enable);

    m_ui.actionPrev->setEnabled(m_dataModel->contextCount() > 0);
    m_ui.actionNext->setEnabled(m_dataModel->contextCount() > 0);
}

void MainWindow::updatePhraseBookActions()
{
    bool phraseBookLoaded = (m_currentIndex.model() >= 0) && !m_phraseBooks.isEmpty();
    m_ui.actionBatchTranslation->setEnabled(m_dataModel->contextCount() > 0 && phraseBookLoaded
                                            && m_dataModel->isModelWritable(m_currentIndex.model()));
    m_ui.actionAddToPhraseBook->setEnabled(currentMessageIndex().isValid() && phraseBookLoaded);
}

void MainWindow::updatePhraseDictInternal(int model)
{
    QHash<QString, QList<Phrase *> > &pd = m_phraseDict[model];

    pd.clear();
    for (PhraseBook *pb : qAsConst(m_phraseBooks)) {
        bool before;
        if (pb->language() != QLocale::C && m_dataModel->language(model) != QLocale::C) {
            if (pb->language() != m_dataModel->language(model))
                continue;
            before = (pb->country() == m_dataModel->model(model)->country());
        } else {
            before = false;
        }
        const auto phrases = pb->phrases();
        for (Phrase *p : phrases) {
            QString f = friendlyString(p->source());
            if (f.length() > 0) {
                f = f.split(QLatin1Char(' ')).first();
                if (!pd.contains(f)) {
                    pd.insert(f, QList<Phrase *>());
                }
                if (before)
                    pd[f].prepend(p);
                else
                    pd[f].append(p);
            }
        }
    }
}

void MainWindow::updatePhraseDict(int model)
{
    updatePhraseDictInternal(model);
    m_phraseView->update();
}

void MainWindow::updatePhraseDicts()
{
    for (int i = 0; i < m_phraseDict.size(); ++i)
        if (!m_dataModel->isModelWritable(i))
            m_phraseDict[i].clear();
        else
            updatePhraseDictInternal(i);
    revalidate();
    m_phraseView->update();
}

static bool haveMnemonic(const QString &str)
{
    for (const ushort *p = (ushort *)str.constData();; ) { // Assume null-termination
        ushort c = *p++;
        if (!c)
            break;
        if (c == '&') {
            c = *p++;
            if (!c)
                return false;
            // "Nobody" ever really uses these alt-space, and they are highly annoying
            // because we get a lot of false positives.
            if (c != '&' && c != ' ' && QChar(c).isPrint()) {
                const ushort *pp = p;
                for (; *p < 256 && isalpha(*p); p++) ;
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

void MainWindow::updateDanger(const MultiDataIndex &index, bool verbose)
{
    MultiDataIndex curIdx = index;
    m_errorsView->clear();

    QString source;
    for (int mi = 0; mi < m_dataModel->modelCount(); ++mi) {
        if (!m_dataModel->isModelWritable(mi))
            continue;
        curIdx.setModel(mi);
        MessageItem *m = m_dataModel->messageItem(curIdx);
        if (!m || m->isObsolete())
            continue;

        bool danger = false;
        if (m->message().isTranslated()) {
            if (source.isEmpty()) {
                source = m->pluralText();
                if (source.isEmpty())
                    source = m->text();
            }
            QStringList translations = m->translations();

            // Truncated variants are permitted to be "denormalized"
            for (int i = 0; i < translations.count(); ++i) {
                int sep = translations.at(i).indexOf(QChar(Translator::BinaryVariantSeparator));
                if (sep >= 0)
                    translations[i].truncate(sep);
            }

            if (m_ui.actionAccelerators->isChecked()) {
                bool sk = haveMnemonic(source);
                bool tk = true;
                for (int i = 0; i < translations.count() && tk; ++i) {
                    tk &= haveMnemonic(translations[i]);
                }

                if (!sk && tk) {
                    if (verbose)
                        m_errorsView->addError(mi, ErrorsView::SuperfluousAccelerator);
                    danger = true;
                } else if (sk && !tk) {
                    if (verbose)
                        m_errorsView->addError(mi, ErrorsView::MissingAccelerator);
                    danger = true;
                }
            }
            if (m_ui.actionSurroundingWhitespace->isChecked()) {
                bool whitespaceok = true;
                for (int i = 0; i < translations.count() && whitespaceok; ++i) {
                    whitespaceok &= (leadingWhitespace(source) == leadingWhitespace(translations[i]));
                    whitespaceok &= (trailingWhitespace(source) == trailingWhitespace(translations[i]));
                }

                if (!whitespaceok) {
                    if (verbose)
                        m_errorsView->addError(mi, ErrorsView::SurroundingWhitespaceDiffers);
                    danger = true;
                }
            }
            if (m_ui.actionEndingPunctuation->isChecked()) {
                bool endingok = true;
                for (int i = 0; i < translations.count() && endingok; ++i) {
                    endingok &= (ending(source, m_dataModel->sourceLanguage(mi)) ==
                                ending(translations[i], m_dataModel->language(mi)));
                }

                if (!endingok) {
                    if (verbose)
                        m_errorsView->addError(mi, ErrorsView::PunctuationDiffers);
                    danger = true;
                }
            }
            if (m_ui.actionPhraseMatches->isChecked()) {
                QString fsource = friendlyString(source);
                QString ftranslation = friendlyString(translations.first());
                QStringList lookupWords = fsource.split(QLatin1Char(' '));

                bool phraseFound;
                for (const QString &s : qAsConst(lookupWords)) {
                    if (m_phraseDict[mi].contains(s)) {
                        phraseFound = true;
                        const auto phrases = m_phraseDict[mi].value(s);
                        for (const Phrase *p : phrases) {
                            if (fsource == friendlyString(p->source())) {
                                if (ftranslation.indexOf(friendlyString(p->target())) >= 0) {
                                    phraseFound = true;
                                    break;
                                } else {
                                    phraseFound = false;
                                }
                            }
                        }
                        if (!phraseFound) {
                            if (verbose)
                                m_errorsView->addError(mi, ErrorsView::IgnoredPhrasebook, s);
                            danger = true;
                        }
                    }
                }
            }

            if (m_ui.actionPlaceMarkerMatches->isChecked()) {
                // Stores the occurrence count of the place markers in the map placeMarkerIndexes.
                // i.e. the occurrence count of %1 is stored at placeMarkerIndexes[1],
                // count of %2 is stored at placeMarkerIndexes[2] etc.
                // In the first pass, it counts all place markers in the sourcetext.
                // In the second pass it (de)counts all place markers in the translation.
                // When finished, all elements should have returned to a count of 0,
                // if not there is a mismatch
                // between place markers in the source text and the translation text.
                QHash<int, int> placeMarkerIndexes;
                QString translation;
                int numTranslations = translations.count();
                for (int pass = 0; pass < numTranslations + 1; ++pass) {
                    const QChar *uc_begin = source.unicode();
                    const QChar *uc_end = uc_begin + source.length();
                    if (pass >= 1) {
                        translation = translations[pass - 1];
                        uc_begin = translation.unicode();
                        uc_end = uc_begin + translation.length();
                    }
                    const QChar *c = uc_begin;
                    while (c < uc_end) {
                        if (c->unicode() == '%') {
                            const QChar *escape_start = ++c;
                            while (c->isDigit())
                                ++c;
                            const QChar *escape_end = c;
                            bool ok = true;
                            int markerIndex = QString::fromRawData(
                                    escape_start, escape_end - escape_start).toInt(&ok);
                            if (ok)
                                placeMarkerIndexes[markerIndex] += (pass == 0 ? numTranslations : -1);
                        }
                        ++c;
                    }
                }

                for (int i : qAsConst(placeMarkerIndexes)) {
                    if (i != 0) {
                        if (verbose)
                            m_errorsView->addError(mi, ErrorsView::PlaceMarkersDiffer);
                        danger = true;
                        break;
                    }
                }

                // Piggy-backed on the general place markers, we check the plural count marker.
                if (m->message().isPlural()) {
                    for (int i = 0; i < numTranslations; ++i)
                        if (m_dataModel->model(mi)->countRefNeeds().at(i)
                            && !(translations[i].contains(QLatin1String("%n"))
                            || translations[i].contains(QLatin1String("%Ln")))) {
                            if (verbose)
                                m_errorsView->addError(mi, ErrorsView::NumerusMarkerMissing);
                            danger = true;
                            break;
                        }
                }
            }
        }

        if (danger != m->danger())
            m_dataModel->setDanger(curIdx, danger);
    }

    if (verbose)
        statusBar()->showMessage(m_errorsView->firstError());
}

void MainWindow::readConfig()
{
    QSettings config;

    restoreGeometry(config.value(settingPath("Geometry/WindowGeometry")).toByteArray());
    restoreState(config.value(settingPath("MainWindowState")).toByteArray());

    m_ui.actionAccelerators->setChecked(
        config.value(settingPath("Validators/Accelerator"), true).toBool());
    m_ui.actionSurroundingWhitespace->setChecked(
        config.value(settingPath("Validators/SurroundingWhitespace"), true).toBool());
    m_ui.actionEndingPunctuation->setChecked(
        config.value(settingPath("Validators/EndingPunctuation"), true).toBool());
    m_ui.actionPhraseMatches->setChecked(
        config.value(settingPath("Validators/PhraseMatch"), true).toBool());
    m_ui.actionPlaceMarkerMatches->setChecked(
        config.value(settingPath("Validators/PlaceMarkers"), true).toBool());
    m_ui.actionLengthVariants->setChecked(
        config.value(settingPath("Options/LengthVariants"), false).toBool());
    m_ui.actionVisualizeWhitespace->setChecked(
        config.value(settingPath("Options/VisualizeWhitespace"), true).toBool());

    m_messageEditor->setFontSize(
                config.value(settingPath("Options/EditorFontsize"), font().pointSize()).toReal());
    m_phraseView->setMaxCandidates(config.value(settingPath("Options/NumberOfGuesses"),
                                                PhraseView::getDefaultMaxCandidates()).toInt());

    recentFiles().readConfig();

    int size = config.beginReadArray(settingPath("OpenedPhraseBooks"));
    for (int i = 0; i < size; ++i) {
        config.setArrayIndex(i);
        doOpenPhraseBook(config.value(QLatin1String("FileName")).toString());
    }
    config.endArray();
}

void MainWindow::writeConfig()
{
    QSettings config;
    config.setValue(settingPath("Geometry/WindowGeometry"),
        saveGeometry());
    config.setValue(settingPath("Validators/Accelerator"),
        m_ui.actionAccelerators->isChecked());
    config.setValue(settingPath("Validators/SurroundingWhitespace"),
        m_ui.actionSurroundingWhitespace->isChecked());
    config.setValue(settingPath("Validators/EndingPunctuation"),
        m_ui.actionEndingPunctuation->isChecked());
    config.setValue(settingPath("Validators/PhraseMatch"),
        m_ui.actionPhraseMatches->isChecked());
    config.setValue(settingPath("Validators/PlaceMarkers"),
        m_ui.actionPlaceMarkerMatches->isChecked());
    config.setValue(settingPath("Options/LengthVariants"),
        m_ui.actionLengthVariants->isChecked());
    config.setValue(settingPath("Options/VisualizeWhitespace"),
        m_ui.actionVisualizeWhitespace->isChecked());
    config.setValue(settingPath("MainWindowState"),
        saveState());
    recentFiles().writeConfig();

    config.setValue(settingPath("Options/EditorFontsize"), m_messageEditor->fontSize());
    config.setValue(settingPath("Options/NumberOfGuesses"), m_phraseView->getMaxCandidates());

    config.beginWriteArray(settingPath("OpenedPhraseBooks"),
        m_phraseBooks.size());
    for (int i = 0; i < m_phraseBooks.size(); ++i) {
        config.setArrayIndex(i);
        config.setValue(QLatin1String("FileName"), m_phraseBooks.at(i)->fileName());
    }
    config.endArray();
}

void MainWindow::setupRecentFilesMenu()
{
    m_ui.menuRecentlyOpenedFiles->clear();
    for (const QStringList &strList : recentFiles().filesLists())
        if (strList.size() == 1) {
            const QString &str = strList.first();
            m_ui.menuRecentlyOpenedFiles->addAction(
                    DataModel::prettifyFileName(str))->setData(str);
        } else {
            QMenu *menu = m_ui.menuRecentlyOpenedFiles->addMenu(
                           MultiDataModel::condenseFileNames(
                                MultiDataModel::prettifyFileNames(strList)));
            menu->addAction(tr("All"))->setData(strList);
            for (const QString &str : strList)
                menu->addAction(DataModel::prettifyFileName(str))->setData(str);
        }
}

void MainWindow::recentFileActivated(QAction *action)
{
    openFiles(action->data().toStringList());
}

void MainWindow::toggleStatistics()
{
    if (m_ui.actionStatistics->isChecked()) {
        if (!m_statistics) {
            m_statistics = new Statistics(this);
            connect(m_dataModel, &MultiDataModel::statsChanged,
                    m_statistics, &Statistics::updateStats);
        }
        m_statistics->show();
        updateStatistics();
    }
    else if (m_statistics) {
        m_statistics->close();
    }
}

void MainWindow::toggleVisualizeWhitespace()
{
    m_messageEditor->setVisualizeWhitespace(m_ui.actionVisualizeWhitespace->isChecked());
}

void MainWindow::maybeUpdateStatistics(const MultiDataIndex &index)
{
    if (index.model() == m_currentIndex.model())
        updateStatistics();
}

void MainWindow::updateStatistics()
{
    // don't call this if stats dialog is not open
    // because this can be slow...
    if (!m_statistics || !m_statistics->isVisible() || m_currentIndex.model() < 0)
        return;

    m_dataModel->model(m_currentIndex.model())->updateStatistics();
}

void MainWindow::doShowTranslationSettings(int model)
{
    if (!m_translationSettingsDialog)
        m_translationSettingsDialog = new TranslationSettingsDialog(this);
    m_translationSettingsDialog->setDataModel(m_dataModel->model(model));
    m_translationSettingsDialog->exec();
}

void MainWindow::showTranslationSettings()
{
    doShowTranslationSettings(m_currentIndex.model());
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *e = static_cast<QDragEnterEvent*>(event);
        if (e->mimeData()->hasFormat(QLatin1String("text/uri-list"))) {
            e->acceptProposedAction();
            return true;
        }
    } else if (event->type() == QEvent::Drop) {
        QDropEvent *e = static_cast<QDropEvent*>(event);
        if (!e->mimeData()->hasFormat(QLatin1String("text/uri-list")))
            return false;
        QStringList urls;
        for (const QUrl &url : e->mimeData()->urls())
            if (!url.toLocalFile().isEmpty())
                urls << url.toLocalFile();
        if (!urls.isEmpty())
            openFiles(urls);
        e->acceptProposedAction();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape) {
            if (object == m_messageEditor)
                m_messageView->setFocus();
            else if (object == m_messagesDock)
                m_contextView->setFocus();
        } else if ((ke->key() == Qt::Key_Plus || ke->key() == Qt::Key_Equal)
                   && (ke->modifiers() & Qt::ControlModifier)) {
            m_messageEditor->increaseFontSize();
        } else if (ke->key() == Qt::Key_Minus
                   && (ke->modifiers() & Qt::ControlModifier)) {
            m_messageEditor->decreaseFontSize();
        }
    } else if (event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        if (we->modifiers() & Qt::ControlModifier) {
            if (we->angleDelta().y() > 0)
                m_messageEditor->increaseFontSize();
            else
                m_messageEditor->decreaseFontSize();
        }
    }
    return false;
}

QT_END_NAMESPACE
