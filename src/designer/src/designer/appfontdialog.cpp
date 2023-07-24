// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "appfontdialog.h"

#include <iconloader_p.h>

#include <QtDesigner/abstractsettings.h>

#include <QtGui/qfontdatabase.h>
#include <QtGui/qstandarditemmodel.h>

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qlayoutitem.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qtreeview.h>

#include <QtCore/qalgorithms.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlist.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstringlist.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

enum {FileNameRole = Qt::UserRole + 1, IdRole =  Qt::UserRole + 2 };
enum { debugAppFontWidget = 0 };

static const char fontFileKeyC[] = "fontFiles";

// AppFontManager: Singleton that maintains the mapping of loaded application font
// ids to the file names (which are not stored in QFontDatabase)
// and provides API for loading/unloading fonts as well for saving/restoring settings.

class AppFontManager
{
    Q_DISABLE_COPY_MOVE(AppFontManager)
    AppFontManager();
public:
    static AppFontManager &instance();

    void save(QDesignerSettingsInterface *s, const QString &prefix) const;
    void restore(const QDesignerSettingsInterface *s, const QString &prefix);

    // Return id or -1
    int add(const QString &fontFile, QString *errorMessage);

    bool remove(int id, QString *errorMessage);
    bool remove(const QString &fontFile, QString *errorMessage);
    bool removeAt(int index, QString *errorMessage);

    // Store loaded fonts as pair of file name and Id
    using FileNameFontIdPair = QPair<QString,int>;
    using FileNameFontIdPairs = QList<FileNameFontIdPair>;
    const FileNameFontIdPairs &fonts() const;

private:
    FileNameFontIdPairs m_fonts;
};

AppFontManager::AppFontManager() = default;

AppFontManager &AppFontManager::instance()
{
    static AppFontManager rc;
    return rc;
}

void AppFontManager::save(QDesignerSettingsInterface *s, const QString &prefix) const
{
    // Store as list of file names
    QStringList fontFiles;
    for (const auto &fnp : m_fonts)
        fontFiles.push_back(fnp.first);

    s->beginGroup(prefix);
    s->setValue(QLatin1StringView(fontFileKeyC),  fontFiles);
    s->endGroup();

    if (debugAppFontWidget)
        qDebug() << "AppFontManager::saved" << fontFiles.size() << "fonts under " << prefix;
}

void AppFontManager::restore(const QDesignerSettingsInterface *s, const QString &prefix)
{
    const QString key = prefix + u'/' + QLatin1StringView(fontFileKeyC);
    const QStringList fontFiles = s->value(key, QStringList()).toStringList();

    if (debugAppFontWidget)
        qDebug() << "AppFontManager::restoring" << fontFiles.size() << "fonts from " << prefix;
    if (!fontFiles.isEmpty()) {
        QString errorMessage;
        for (const auto &ff : fontFiles) {
            if (add(ff, &errorMessage) == -1)
                qWarning("%s", qPrintable(errorMessage));
        }
    }
}

int AppFontManager::add(const QString &fontFile, QString *errorMessage)
{
    const QFileInfo inf(fontFile);
    if (!inf.isFile()) {
        *errorMessage = QCoreApplication::translate("AppFontManager", "'%1' is not a file.").arg(fontFile);
        return -1;
    }
    if (!inf.isReadable()) {
        *errorMessage = QCoreApplication::translate("AppFontManager", "The font file '%1' does not have read permissions.").arg(fontFile);
        return -1;
    }
    const QString fullPath = inf.absoluteFilePath();
    // Check if already loaded
    for (const auto &fnp : std::as_const(m_fonts)) {
        if (fnp.first == fullPath) {
            *errorMessage = QCoreApplication::translate("AppFontManager", "The font file '%1' is already loaded.").arg(fontFile);
            return -1;
        }
    }

    const int id = QFontDatabase::addApplicationFont(fullPath);
    if (id == -1) {
        *errorMessage = QCoreApplication::translate("AppFontManager", "The font file '%1' could not be loaded.").arg(fontFile);
        return -1;
    }

    if (debugAppFontWidget)
        qDebug() << "AppFontManager::add" << fontFile << id;
    m_fonts.push_back(FileNameFontIdPair(fullPath, id));
    return id;
}

bool AppFontManager::remove(int id, QString *errorMessage)
{
    for (qsizetype i = 0, count = m_fonts.size(); i < count; ++i)
        if (m_fonts.at(i).second == id)
            return removeAt(i, errorMessage);

    *errorMessage = QCoreApplication::translate("AppFontManager", "'%1' is not a valid font id.").arg(id);
    return false;
}

bool AppFontManager::remove(const QString &fontFile, QString *errorMessage)
{
    for (qsizetype i = 0, count = m_fonts.size(); i < count; ++i)
        if (m_fonts.at(i).first == fontFile)
            return removeAt(i, errorMessage);

    *errorMessage = QCoreApplication::translate("AppFontManager", "There is no loaded font matching the id '%1'.").arg(fontFile);
    return false;
}

bool AppFontManager::removeAt(int index, QString *errorMessage)
{
    Q_ASSERT(index >= 0 && index < m_fonts.size());

    const QString fontFile = m_fonts[index].first;
    const int id = m_fonts[index].second;

    if (debugAppFontWidget)
        qDebug() << "AppFontManager::removeAt" << index << '(' <<  fontFile << id << ')';

    if (!QFontDatabase::removeApplicationFont(id)) {
        *errorMessage = QCoreApplication::translate("AppFontManager", "The font '%1' (%2) could not be unloaded.").arg(fontFile).arg(id);
        return false;
    }
    m_fonts.removeAt(index);
    return true;
}

const AppFontManager::FileNameFontIdPairs &AppFontManager::fonts() const
{
    return  m_fonts;
}

// ------------- AppFontModel
class AppFontModel : public QStandardItemModel {
    Q_DISABLE_COPY_MOVE(AppFontModel)
public:
    AppFontModel(QObject *parent = nullptr);

    void init(const AppFontManager &mgr);
    void add(const QString &fontFile, int id);
    int idAt(const QModelIndex &idx) const;
};

AppFontModel::AppFontModel(QObject * parent) :
    QStandardItemModel(parent)
{
    setHorizontalHeaderLabels(QStringList(AppFontWidget::tr("Fonts")));
}

void AppFontModel::init(const AppFontManager &mgr)
{
    using FileNameFontIdPairs = AppFontManager::FileNameFontIdPairs;

    const FileNameFontIdPairs &fonts = mgr.fonts();
    for (const auto &fnp : fonts)
        add(fnp.first, fnp.second);
}

void AppFontModel::add(const QString &fontFile, int id)
{
    const QFileInfo inf(fontFile);
    // Root item with base name
    QStandardItem *fileItem = new QStandardItem(inf.completeBaseName());
    const QString fullPath = inf.absoluteFilePath();
    fileItem->setData(fullPath, FileNameRole);
    fileItem->setToolTip(fullPath);
    fileItem->setData(id, IdRole);
    fileItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    appendRow(fileItem);
    const QStringList families = QFontDatabase::applicationFontFamilies(id);
    for (const auto &fam : families) {
        QStandardItem *familyItem = new QStandardItem(fam);
        familyItem->setToolTip(fullPath);
        familyItem->setFont(QFont(fam));
        familyItem->setFlags(Qt::ItemIsEnabled);
        fileItem->appendRow(familyItem);
    }
}

int AppFontModel::idAt(const QModelIndex &idx) const
{
    if (const QStandardItem *item = itemFromIndex(idx))
        return item->data(IdRole).toInt();
    return -1;
}

// ------------- AppFontWidget
AppFontWidget::AppFontWidget(QWidget *parent) :
    QGroupBox(parent),
    m_view(new QTreeView),
    m_addButton(new QToolButton),
    m_removeButton(new QToolButton),
    m_removeAllButton(new QToolButton),
    m_model(new AppFontModel(this))
{
    m_model->init(AppFontManager::instance());
    m_view->setModel(m_model);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->expandAll();
    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AppFontWidget::selectionChanged);

    m_addButton->setToolTip(tr("Add font files"));
    m_addButton->setIcon(qdesigner_internal::createIconSet(u"plus.png"_s));
    connect(m_addButton, &QAbstractButton::clicked, this, &AppFontWidget::addFiles);

    m_removeButton->setEnabled(false);
    m_removeButton->setToolTip(tr("Remove current font file"));
    m_removeButton->setIcon(qdesigner_internal::createIconSet(u"minus.png"_s));
    connect(m_removeButton, &QAbstractButton::clicked, this, &AppFontWidget::slotRemoveFiles);

    m_removeAllButton->setToolTip(tr("Remove all font files"));
    m_removeAllButton->setIcon(qdesigner_internal::createIconSet(u"editdelete.png"_s));
    connect(m_removeAllButton, &QAbstractButton::clicked, this, &AppFontWidget::slotRemoveAll);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_addButton);
    hLayout->addWidget(m_removeButton);
    hLayout->addWidget(m_removeAllButton);
    hLayout->addItem(new QSpacerItem(0, 0,QSizePolicy::MinimumExpanding));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_view);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);
}

void AppFontWidget::addFiles()
{
    const QStringList files =
        QFileDialog::getOpenFileNames(this, tr("Add Font Files"), QString(),
                                      tr("Font files (*.ttf)"));
    if (files.isEmpty())
        return;

    QString errorMessage;

    AppFontManager &fmgr = AppFontManager::instance();
    for (const auto &f : files) {
        const int id = fmgr.add(f, &errorMessage);
        if (id != -1) {
            m_model->add(f, id);
        } else {
            QMessageBox::critical(this, tr("Error Adding Fonts"), errorMessage);
        }
    }
    m_view->expandAll();
}

static void removeFonts(const QModelIndexList &selectedIndexes, AppFontModel *model, QWidget *dialogParent)
{
    if (selectedIndexes.isEmpty())
        return;

    // Reverse sort top level rows and remove
    AppFontManager &fmgr = AppFontManager::instance();
    QList<int> rows;
    rows.reserve(selectedIndexes.size());

    QString errorMessage;
    for (const auto &mi : selectedIndexes) {
        const int id = model->idAt(mi);
        if (id != -1) {
            if (fmgr.remove(id, &errorMessage)) {
                rows.append(mi.row());
            } else {
                QMessageBox::critical(dialogParent, AppFontWidget::tr("Error Removing Fonts"), errorMessage);
            }
        }
    }

    std::stable_sort(rows.begin(), rows.end());
    for (qsizetype i = rows.size() - 1; i >= 0; --i)
        model->removeRow(rows.at(i));
}

void AppFontWidget::slotRemoveFiles()
{
    removeFonts(m_view->selectionModel()->selectedIndexes(), m_model, this);
}

void AppFontWidget::slotRemoveAll()
{
    const int count = m_model->rowCount();
    if (!count)
        return;

    const QMessageBox::StandardButton answer =
        QMessageBox::question(this, tr("Remove Fonts"), tr("Would you like to remove all fonts?"),
                              QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::No)
        return;

    QModelIndexList topLevels;
    for (int i = 0; i < count; i++)
        topLevels.push_back(m_model->index(i, 0));
    removeFonts(topLevels, m_model, this);
}

void AppFontWidget::selectionChanged(const QItemSelection &selected, const QItemSelection & /*deselected*/)
{
     m_removeButton->setEnabled(!selected.indexes().isEmpty());
}

void AppFontWidget::save(QDesignerSettingsInterface *s, const QString &prefix)
{
    AppFontManager::instance().save(s, prefix);
}

void AppFontWidget::restore(const QDesignerSettingsInterface *s, const QString &prefix)
{
    AppFontManager::instance().restore(s, prefix);
}

// ------------ AppFontDialog
AppFontDialog::AppFontDialog(QWidget *parent) :
    QDialog(parent),
    m_appFontWidget(new AppFontWidget)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Additional Fonts"));
    setModal(false);
    QVBoxLayout *vl = new  QVBoxLayout;
    vl->addWidget(m_appFontWidget);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    QDialog::connect(bb, &QDialogButtonBox::rejected, this, &AppFontDialog::reject);
    vl->addWidget(bb);
    setLayout(vl);
}

QT_END_NAMESPACE
