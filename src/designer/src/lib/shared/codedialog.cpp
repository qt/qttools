// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "codedialog_p.h"
#include "qdesigner_utils_p.h"
#include "iconloader_p.h"

#include <texteditfindwidget_p.h>

#include <QtWidgets/qapplication.h>
#if QT_CONFIG(clipboard)
#include <QtGui/qclipboard.h>
#endif
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qboxlayout.h>

#include <QtGui/qaction.h>
#include <QtGui/qevent.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qicon.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qmimedatabase.h>
#include <QtCore/qtemporaryfile.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {
// ----------------- CodeDialogPrivate
struct CodeDialog::CodeDialogPrivate {
    CodeDialogPrivate();

    QTextEdit *m_textEdit;
    TextEditFindWidget *m_findWidget;
    QString m_formFileName;
    QString m_mimeType;
};

CodeDialog::CodeDialogPrivate::CodeDialogPrivate()
    : m_textEdit(new QTextEdit)
    , m_findWidget(new TextEditFindWidget)
{
}

// ----------------- CodeDialog
CodeDialog::CodeDialog(QWidget *parent) :
    QDialog(parent),
    m_impl(new CodeDialogPrivate)
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;

    // Edit tool bar
    QToolBar *toolBar = new QToolBar;

    const QIcon saveIcon = createIconSet(QIcon::ThemeIcon::DocumentSave,
                                         "filesave.png"_L1);
    QAction *saveAction = toolBar->addAction(saveIcon, tr("Save..."));
    connect(saveAction, &QAction::triggered, this, &CodeDialog::slotSaveAs);

#if QT_CONFIG(clipboard)
    const QIcon copyIcon = createIconSet(QIcon::ThemeIcon::EditCopy,
                                         "editcopy.png"_L1);
    QAction *copyAction = toolBar->addAction(copyIcon, tr("Copy All"));
    connect(copyAction, &QAction::triggered, this, &CodeDialog::copyAll);
#endif

    toolBar->addAction(m_impl->m_findWidget->createFindAction(toolBar));

    vBoxLayout->addWidget(toolBar);

    // Edit
    m_impl->m_textEdit->setReadOnly(true);
    const auto font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
    const int editorWidth = QFontMetrics(font, this).averageCharWidth() * 100;
    m_impl->m_textEdit->setFont(font);
    m_impl->m_textEdit->setMinimumSize(QSize(
                qMax(editorWidth, m_impl->m_findWidget->minimumSize().width()),
                500));
    vBoxLayout->addWidget(m_impl->m_textEdit);

    // Find
    m_impl->m_findWidget->setTextEdit(m_impl->m_textEdit);
    vBoxLayout->addWidget(m_impl->m_findWidget);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Disable auto default
    QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);
    closeButton->setAutoDefault(false);
    vBoxLayout->addWidget(buttonBox);

    setLayout(vBoxLayout);
}

CodeDialog::~CodeDialog()
{
    delete m_impl;
}

void CodeDialog::setCode(const QString &code)
{
    m_impl->m_textEdit->setPlainText(code);
}

QString CodeDialog::code() const
{
   return m_impl->m_textEdit->toPlainText();
}

void CodeDialog::setFormFileName(const QString &f)
{
    m_impl->m_formFileName = f;
}

QString CodeDialog::formFileName() const
{
    return m_impl->m_formFileName;
}

void CodeDialog::setMimeType(const QString &m)
{
    m_impl->m_mimeType = m;
}

bool CodeDialog::generateCode(const QDesignerFormWindowInterface *fw,
                              UicLanguage language,
                              QString *code,
                              QString *errorMessage)
{
    // Generate temporary file name similar to form file name
    // (for header guards)
    QString tempPattern = QDir::tempPath();
    if (!tempPattern.endsWith(QDir::separator())) // platform-dependant
        tempPattern += QDir::separator();
    const QString fileName = fw->fileName();
    if (fileName.isEmpty()) {
        tempPattern += "designer"_L1;
    } else {
        tempPattern += QFileInfo(fileName).baseName();
    }
    tempPattern += "XXXXXX.ui"_L1;
    // Write to temp file
    QTemporaryFile tempFormFile(tempPattern);

    tempFormFile.setAutoRemove(true);
    if (!tempFormFile.open()) {
        *errorMessage = tr("A temporary form file could not be created in %1.").arg(QDir::tempPath());
        return false;
    }
    const QString tempFormFileName = tempFormFile.fileName();
    tempFormFile.write(fw->contents().toUtf8());
    if (!tempFormFile.flush())  {
        *errorMessage = tr("The temporary form file %1 could not be written.").arg(tempFormFileName);
        return false;
    }
    tempFormFile.close();
    // Run uic
    QByteArray rc;
    if (!runUIC(tempFormFileName, language, rc, *errorMessage))
        return false;
    *code = QString::fromUtf8(rc);
    return true;
}

bool CodeDialog::showCodeDialog(const QDesignerFormWindowInterface *fw,
                                UicLanguage language,
                                QWidget *parent,
                                QString *errorMessage)
{
    QString code;
    if (!generateCode(fw, language, &code, errorMessage))
        return false;

    auto dialog = new CodeDialog(parent);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setCode(code);
    dialog->setFormFileName(fw->fileName());
    QLatin1StringView languageName;
    switch (language) {
    case UicLanguage::Cpp:
        languageName = "C++"_L1;
        dialog->setMimeType(u"text/x-chdr"_s);
        break;
    case UicLanguage::Python:
        languageName = "Python"_L1;
        dialog->setMimeType(u"text/x-python"_s);
        break;
    }
    dialog->setWindowTitle(tr("%1 - [%2 Code]").
                           arg(fw->mainContainer()->windowTitle(), languageName));
    dialog->show();
    return true;
}

void CodeDialog::slotSaveAs()
{
    // build the default relative name 'ui_sth.h'
    QMimeDatabase mimeDb;
    const QString suffix = mimeDb.mimeTypeForName(m_impl->m_mimeType).preferredSuffix();

    // file dialog
    QFileDialog fileDialog(this, tr("Save Code"));
    fileDialog.setMimeTypeFilters(QStringList(m_impl->m_mimeType));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDefaultSuffix(suffix);
    const QString uiFile = formFileName();
    if (!uiFile.isEmpty()) {
        QFileInfo uiFi(uiFile);
        fileDialog.setDirectory(uiFi.absolutePath());
        fileDialog.selectFile("ui_"_L1 + uiFi.baseName()
                              + '.'_L1 + suffix);
    }

    while (true) {
        if (fileDialog.exec() != QDialog::Accepted)
            break;
        const QString fileName = fileDialog.selectedFiles().constFirst();

         QFile file(fileName);
         if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
             warning(tr("The file %1 could not be opened: %2")
                     .arg(fileName, file.errorString()));
             continue;
         }
         file.write(code().toUtf8());
         if (!file.flush()) {
             warning(tr("The file %1 could not be written: %2")
                     .arg(fileName, file.errorString()));
             continue;
         }
         file.close();
         break;
    }
}

void CodeDialog::warning(const QString &msg)
{
     QMessageBox::warning(
             this, tr("%1 - Error").arg(windowTitle()),
             msg, QMessageBox::Close);
}

#if QT_CONFIG(clipboard)
void CodeDialog::copyAll()
{
    QApplication::clipboard()->setText(code());
}
#endif

} // namespace qdesigner_internal

QT_END_NAMESPACE
