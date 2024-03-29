// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "newform.h"
#include "qdesigner_workbench.h"
#include "qdesigner_actions.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"

#include <newformwidget_p.h>

#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmessagebox.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qtemporaryfile.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget, const QString &fileName)
    : QDialog(parentWidget, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      m_fileName(fileName),
      m_newFormWidget(QDesignerNewFormWidgetInterface::createNewFormWidget(workbench->core())),
      m_workbench(workbench),
      m_chkShowOnStartup(new QCheckBox(tr("Show this Dialog on Startup"))),
      m_createButton(new QPushButton(QApplication::translate("NewForm", "C&reate", nullptr))),
      m_recentButton(new QPushButton(QApplication::translate("NewForm", "Recent", nullptr)))
{
    setWindowTitle(tr("New Form"));
    QDesignerSettings settings(m_workbench->core());

    QVBoxLayout *vBoxLayout = new QVBoxLayout;

    connect(m_newFormWidget, &QDesignerNewFormWidgetInterface::templateActivated,
            this, &NewForm::slotTemplateActivated);
    connect(m_newFormWidget, &QDesignerNewFormWidgetInterface::currentTemplateChanged,
            this, &NewForm::slotCurrentTemplateChanged);
    vBoxLayout->addWidget(m_newFormWidget);

    QFrame *horizontalLine = new QFrame;
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);
    vBoxLayout->addWidget(horizontalLine);

    m_chkShowOnStartup->setChecked(settings.showNewFormOnStartup());
    vBoxLayout->addWidget(m_chkShowOnStartup);

    m_buttonBox = createButtonBox();
    vBoxLayout->addWidget(m_buttonBox);
    setLayout(vBoxLayout);

    resize(500, 400);
    slotCurrentTemplateChanged(m_newFormWidget->hasCurrentTemplate());
}

QDialogButtonBox *NewForm::createButtonBox()
{
    // Dialog buttons with 'recent files'
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(QApplication::translate("NewForm", "&Close", nullptr),
                         QDialogButtonBox::RejectRole);
    buttonBox->addButton(m_createButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QApplication::translate("NewForm", "&Open...", nullptr),
                         QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_recentButton, QDialogButtonBox::ActionRole);
    QDesignerActions *da = m_workbench->actionManager();
    QMenu *recentFilesMenu = new QMenu(tr("&Recent Forms"), m_recentButton);
    // Pop the "Recent Files" stuff in here.
    const auto recentActions = da->recentFilesActions()->actions();
    for (auto action : recentActions) {
        recentFilesMenu->addAction(action);
        connect(action, &QAction::triggered, this, &NewForm::recentFileChosen);
    }
    m_recentButton->setMenu(recentFilesMenu);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &NewForm::slotButtonBoxClicked);
    return buttonBox;
}

NewForm::~NewForm()
{
    QDesignerSettings settings (m_workbench->core());
    settings.setShowNewFormOnStartup(m_chkShowOnStartup->isChecked());
}

void NewForm::recentFileChosen()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action && action->objectName() != "__qt_action_clear_menu_"_L1)
        close();
}

void NewForm::slotCurrentTemplateChanged(bool templateSelected)
{
    if (templateSelected) {
        m_createButton->setEnabled(true);
        m_createButton->setDefault(true);
    } else {
        m_createButton->setEnabled(false);
    }
}

void NewForm::slotTemplateActivated()
{
    m_createButton->animateClick();
}

void NewForm::slotButtonBoxClicked(QAbstractButton *btn)
{
    switch (m_buttonBox->buttonRole(btn)) {
    case QDialogButtonBox::RejectRole:
        reject();
        break;
    case QDialogButtonBox::ActionRole:
        if (btn != m_recentButton) {
            m_fileName.clear();
            if (m_workbench->actionManager()->openForm(this))
                accept();
        }
        break;
    case QDialogButtonBox::AcceptRole: {
        QString errorMessage;
        if (openTemplate(&errorMessage)) {
            accept();
        }  else {
            QMessageBox::warning(this, tr("Read error"), errorMessage);
        }
    }
        break;
    default:
        break;
    }
}

bool NewForm::openTemplate(QString *ptrToErrorMessage)
{
    const QString contents = m_newFormWidget->currentTemplate(ptrToErrorMessage);
    if (contents.isEmpty())
        return false;
    // Write to temporary file and open
    QString tempPattern = QDir::tempPath();
    if (!tempPattern.endsWith(QDir::separator())) // platform-dependant
        tempPattern += QDir::separator();
    tempPattern += "XXXXXX.ui"_L1;
    QTemporaryFile tempFormFile(tempPattern);

    tempFormFile.setAutoRemove(true);
    if (!tempFormFile.open()) {
        *ptrToErrorMessage = tr("A temporary form file could not be created in %1: %2")
            .arg(QDir::toNativeSeparators(QDir::tempPath()), tempFormFile.errorString());
        return false;
    }
    const QString tempFormFileName = tempFormFile.fileName();
    tempFormFile.write(contents.toUtf8());
    if (!tempFormFile.flush())  {
        *ptrToErrorMessage = tr("The temporary form file %1 could not be written: %2")
            .arg(QDir::toNativeSeparators(tempFormFileName), tempFormFile.errorString());
        return false;
    }
    tempFormFile.close();
    return m_workbench->openTemplate(tempFormFileName, m_fileName, ptrToErrorMessage);
}

QImage NewForm::grabForm(QDesignerFormEditorInterface *core,
                         QIODevice &file,
                         const QString &workingDir,
                         const qdesigner_internal::DeviceProfile &dp)
{
    return qdesigner_internal::NewFormWidget::grabForm(core, file, workingDir, dp);
}

QT_END_NAMESPACE
