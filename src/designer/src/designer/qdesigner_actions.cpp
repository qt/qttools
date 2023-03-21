// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_actions.h"
#include "designer_enums.h"
#include <qdesigner_utils_p.h>
#include "qdesigner.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "mainwindow.h"
#include "newform.h"
#include "versiondialog.h"
#include "saveformastemplate.h"
#include "preferencesdialog.h"
#include "appfontdialog.h"

#include <pluginmanager_p.h>
#include <qdesigner_formbuilder_p.h>
#include <qdesigner_utils_p.h>
#include <iconloader_p.h>
#include <previewmanager_p.h>
#include <codedialog_p.h>
#include <qdesigner_formwindowmanager_p.h>

// sdk
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/abstractmetadatabase.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformeditorplugin.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtDesigner/private/shared_settings_p.h>
#include <QtDesigner/private/formwindowbase_p.h>

#include <QtWidgets/qstylefactory.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qmdisubwindow.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstatusbar.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qevent.h>
#include <QtGui/qicon.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qscreen.h>
#if defined(QT_PRINTSUPPORT_LIB) // Some platforms may not build QtPrintSupport
#  include <QtPrintSupport/qtprintsupportglobal.h>
#  if QT_CONFIG(printer) && QT_CONFIG(printdialog)
#    include <QtPrintSupport/qprinter.h>
#    include <QtPrintSupport/qprintdialog.h>
#    define HAS_PRINTER
#  endif
#endif
#include <QtGui/qpainter.h>
#include <QtGui/qtransform.h>
#include <QtGui/qcursor.h>

#include <QtCore/qsize.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qscopedpointer.h>
#include <QtXml/qdom.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

const char *QDesignerActions::defaultToolbarPropertyName = "__qt_defaultToolBarAction";

//#ifdef Q_OS_MACOS
#  define NONMODAL_PREVIEW
//#endif

static QAction *createSeparator(QObject *parent) {
    QAction * rc = new QAction(parent);
    rc->setSeparator(true);
    return rc;
}

static QActionGroup *createActionGroup(QObject *parent, bool exclusive = false) {
    QActionGroup * rc = new QActionGroup(parent);
    rc->setExclusive(exclusive);
    return rc;
}

static void fixActionContext(const QList<QAction *> &actions)
{
    for (QAction *a : actions)
        a->setShortcutContext(Qt::ApplicationShortcut);
}

static inline QString savedMessage(const QString &fileName)
{
    return QDesignerActions::tr("Saved %1.").arg(fileName);
}

static QString fileDialogFilters(const QString &extension)
{
    return QDesignerActions::tr("Designer UI files (*.%1);;All Files (*)").arg(extension);
}

QFileDialog *createSaveAsDialog(QWidget *parent, const QString &dir, const QString &extension)
{
    auto result = new QFileDialog(parent, QDesignerActions::tr("Save Form As"),
                                  dir, fileDialogFilters(extension));
    result->setAcceptMode(QFileDialog::AcceptSave);
    result->setDefaultSuffix(extension);
    return result;
}

QDesignerActions::QDesignerActions(QDesignerWorkbench *workbench)
    : QObject(workbench),
      m_workbench(workbench),
      m_core(workbench->core()),
      m_settings(workbench->core()),
      m_backupTimer(new QTimer(this)),
      m_fileActions(createActionGroup(this)),
      m_recentFilesActions(createActionGroup(this)),
      m_editActions(createActionGroup(this)),
      m_formActions(createActionGroup(this)),
      m_settingsActions(createActionGroup(this)),
      m_windowActions(createActionGroup(this)),
      m_toolActions(createActionGroup(this, true)),
      m_editWidgetsAction(new QAction(tr("Edit Widgets"), this)),
      m_newFormAction(new QAction(qdesigner_internal::createIconSet(u"filenew.png"_s),
                                  tr("&New..."), this)),
      m_openFormAction(new QAction(qdesigner_internal::createIconSet(u"fileopen.png"_s),
                                   tr("&Open..."), this)),
      m_saveFormAction(new QAction(qdesigner_internal::createIconSet(u"filesave.png"_s),
                                   tr("&Save"), this)),
      m_saveFormAsAction(new QAction(tr("Save &As..."), this)),
      m_saveAllFormsAction(new QAction(tr("Save A&ll"), this)),
      m_saveFormAsTemplateAction(new QAction(tr("Save As &Template..."), this)),
      m_closeFormAction(new QAction(tr("&Close"), this)),
      m_savePreviewImageAction(new QAction(tr("Save &Image..."), this)),
      m_printPreviewAction(new QAction(tr("&Print..."), this)),
      m_quitAction(new QAction(tr("&Quit"), this)),
      m_viewCppCodeAction(new QAction(tr("View &C++ Code..."), this)),
      m_viewPythonCodeAction(new QAction(tr("View &Python Code..."), this)),
      m_minimizeAction(new QAction(tr("&Minimize"), this)),
      m_bringAllToFrontSeparator(createSeparator(this)),
      m_bringAllToFrontAction(new QAction(tr("Bring All to Front"), this)),
      m_windowListSeparatorAction(createSeparator(this)),
      m_preferencesAction(new QAction(tr("Preferences..."), this)),
      m_appFontAction(new QAction(tr("Additional Fonts..."), this))
{
#if defined (Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_newFormAction->setIcon(QIcon::fromTheme(u"document-new"_s,
                                              m_newFormAction->icon()));
    m_openFormAction->setIcon(QIcon::fromTheme(u"document-open"_s,
                                               m_openFormAction->icon()));
    m_saveFormAction->setIcon(QIcon::fromTheme(u"document-save"_s,
                                               m_saveFormAction->icon()));
    m_saveFormAsAction->setIcon(QIcon::fromTheme(u"document-save-as"_s,
                                                 m_saveFormAsAction->icon()));
    m_printPreviewAction->setIcon(QIcon::fromTheme(u"document-print"_s,
                                                   m_printPreviewAction->icon()));
    m_closeFormAction->setIcon(QIcon::fromTheme(u"window-close"_s, m_closeFormAction->icon()));
    m_quitAction->setIcon(QIcon::fromTheme(u"application-exit"_s, m_quitAction->icon()));
#endif

    Q_ASSERT(m_core != nullptr);
    qdesigner_internal::QDesignerFormWindowManager *ifwm = qobject_cast<qdesigner_internal::QDesignerFormWindowManager *>(m_core->formWindowManager());
    Q_ASSERT(ifwm);
    m_previewManager = ifwm->previewManager();
    m_previewFormAction = ifwm->action(QDesignerFormWindowManagerInterface::DefaultPreviewAction);
    m_styleActions = ifwm->actionGroup(QDesignerFormWindowManagerInterface::StyledPreviewActionGroup);
    connect(ifwm, &QDesignerFormWindowManagerInterface::formWindowSettingsChanged,
            this, &QDesignerActions::formWindowSettingsChanged);

    m_editWidgetsAction->setObjectName(u"__qt_edit_widgets_action"_s);
    m_newFormAction->setObjectName(u"__qt_new_form_action"_s);
    m_openFormAction->setObjectName(u"__qt_open_form_action"_s);
    m_saveFormAction->setObjectName(u"__qt_save_form_action"_s);
    m_saveFormAsAction->setObjectName(u"__qt_save_form_as_action"_s);
    m_saveAllFormsAction->setObjectName(u"__qt_save_all_forms_action"_s);
    m_saveFormAsTemplateAction->setObjectName(u"__qt_save_form_as_template_action"_s);
    m_closeFormAction->setObjectName(u"__qt_close_form_action"_s);
    m_quitAction->setObjectName(u"__qt_quit_action"_s);
    m_previewFormAction->setObjectName(u"__qt_preview_form_action"_s);
    m_viewCppCodeAction->setObjectName(u"__qt_preview_cpp_code_action"_s);
    m_viewPythonCodeAction->setObjectName(u"__qt_preview_python_code_action"_s);
    m_minimizeAction->setObjectName(u"__qt_minimize_action"_s);
    m_bringAllToFrontAction->setObjectName(u"__qt_bring_all_to_front_action"_s);
    m_preferencesAction->setObjectName(u"__qt_preferences_action"_s);

    m_helpActions = createHelpActions();

    m_newFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    m_openFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    m_saveFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);

    QDesignerFormWindowManagerInterface *formWindowManager = m_core->formWindowManager();
    Q_ASSERT(formWindowManager != nullptr);

//
// file actions
//
    m_newFormAction->setShortcut(QKeySequence::New);
    connect(m_newFormAction, &QAction::triggered, this, &QDesignerActions::createForm);
    m_fileActions->addAction(m_newFormAction);

    m_openFormAction->setShortcut(QKeySequence::Open);
    connect(m_openFormAction, &QAction::triggered, this, &QDesignerActions::slotOpenForm);
    m_fileActions->addAction(m_openFormAction);

    m_fileActions->addAction(createRecentFilesMenu());
    m_fileActions->addAction(createSeparator(this));

    m_saveFormAction->setShortcut(QKeySequence::Save);
    connect(m_saveFormAction, &QAction::triggered, this,
            QOverload<>::of(&QDesignerActions::saveForm));
    m_fileActions->addAction(m_saveFormAction);

    connect(m_saveFormAsAction, &QAction::triggered, this,
            QOverload<>::of(&QDesignerActions::saveFormAs));
    m_fileActions->addAction(m_saveFormAsAction);

#ifdef Q_OS_MACOS
    m_saveAllFormsAction->setShortcut(tr("ALT+CTRL+S"));
#else
    m_saveAllFormsAction->setShortcut(tr("CTRL+SHIFT+S")); // Commonly "Save As" on Mac
#endif
    connect(m_saveAllFormsAction, &QAction::triggered, this, &QDesignerActions::saveAllForms);
    m_fileActions->addAction(m_saveAllFormsAction);

    connect(m_saveFormAsTemplateAction, &QAction::triggered, this, &QDesignerActions::saveFormAsTemplate);
    m_fileActions->addAction(m_saveFormAsTemplateAction);

    m_fileActions->addAction(createSeparator(this));

    m_printPreviewAction->setShortcut(QKeySequence::Print);
    connect(m_printPreviewAction,  &QAction::triggered, this, &QDesignerActions::printPreviewImage);
    m_fileActions->addAction(m_printPreviewAction);
    m_printPreviewAction->setObjectName(u"__qt_print_action"_s);

    connect(m_savePreviewImageAction,  &QAction::triggered, this, &QDesignerActions::savePreviewImage);
    m_savePreviewImageAction->setObjectName(u"__qt_saveimage_action"_s);
    m_fileActions->addAction(m_savePreviewImageAction);
    m_fileActions->addAction(createSeparator(this));

    m_closeFormAction->setShortcut(QKeySequence::Close);
    connect(m_closeFormAction, &QAction::triggered, this, &QDesignerActions::closeForm);
    m_fileActions->addAction(m_closeFormAction);
    updateCloseAction();

    m_fileActions->addAction(createSeparator(this));

    m_quitAction->setShortcuts(QKeySequence::Quit);
    m_quitAction->setMenuRole(QAction::QuitRole);
    connect(m_quitAction, &QAction::triggered, this, &QDesignerActions::shutdown);
    m_fileActions->addAction(m_quitAction);

//
// edit actions
//
    QAction *undoAction = formWindowManager->action(QDesignerFormWindowManagerInterface::UndoAction);
    undoAction->setObjectName(u"__qt_undo_action"_s);
    undoAction->setShortcut(QKeySequence::Undo);
    m_editActions->addAction(undoAction);

    QAction *redoAction = formWindowManager->action(QDesignerFormWindowManagerInterface::RedoAction);
    redoAction->setObjectName(u"__qt_redo_action"_s);
    redoAction->setShortcut(QKeySequence::Redo);
    m_editActions->addAction(redoAction);

    m_editActions->addAction(createSeparator(this));

#if QT_CONFIG(clipboard)
    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::CutAction));
    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::CopyAction));
    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::PasteAction));
#endif
    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::DeleteAction));

    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SelectAllAction));

    m_editActions->addAction(createSeparator(this));

    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::LowerAction));
    m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::RaiseAction));

    formWindowManager->action(QDesignerFormWindowManagerInterface::LowerAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::RaiseAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);

//
// edit mode actions
//

    m_editWidgetsAction->setCheckable(true);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::Key_F3));
    shortcuts.append(QKeySequence(Qt::Key_Escape));
    m_editWidgetsAction->setShortcuts(shortcuts);
    QIcon fallback(m_core->resourceLocation() + "/widgettool.png"_L1);
    m_editWidgetsAction->setIcon(QIcon::fromTheme(u"designer-edit-widget"_s,
                                                  fallback));
    connect(m_editWidgetsAction, &QAction::triggered, this, &QDesignerActions::editWidgetsSlot);
    m_editWidgetsAction->setChecked(true);
    m_editWidgetsAction->setEnabled(false);
    m_editWidgetsAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    m_toolActions->addAction(m_editWidgetsAction);

    connect(formWindowManager, &qdesigner_internal::QDesignerFormWindowManager::activeFormWindowChanged,
                this, &QDesignerActions::activeFormWindowChanged);

    const QObjectList builtinPlugins = QPluginLoader::staticInstances()
        + m_core->pluginManager()->instances();
    for (QObject *plugin : builtinPlugins) {
        if (QDesignerFormEditorPluginInterface *formEditorPlugin = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) {
            if (QAction *action = formEditorPlugin->action()) {
                m_toolActions->addAction(action);
                action->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
                action->setCheckable(true);
            }
        }
    }

    connect(m_preferencesAction, &QAction::triggered,  this, &QDesignerActions::showPreferencesDialog);
    m_preferencesAction->setMenuRole(QAction::PreferencesRole);
    m_settingsActions->addAction(m_preferencesAction);

    connect(m_appFontAction, &QAction::triggered,  this, &QDesignerActions::showAppFontDialog);
    m_settingsActions->addAction(m_appFontAction);
//
// form actions
//

    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::HorizontalLayoutAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::VerticalLayoutAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SplitHorizontalAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SplitVerticalAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::GridLayoutAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::FormLayoutAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::BreakLayoutAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::AdjustSizeAction));
    m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SimplifyLayoutAction));
    m_formActions->addAction(createSeparator(this));

    formWindowManager->action(QDesignerFormWindowManagerInterface::HorizontalLayoutAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::VerticalLayoutAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::SplitHorizontalAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::SplitVerticalAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::GridLayoutAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::FormLayoutAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::BreakLayoutAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
    formWindowManager->action(QDesignerFormWindowManagerInterface::AdjustSizeAction)->setProperty(QDesignerActions::defaultToolbarPropertyName, true);

    m_previewFormAction->setShortcut(tr("CTRL+R"));
    m_formActions->addAction(m_previewFormAction);
    connect(m_previewManager, &qdesigner_internal::PreviewManager::firstPreviewOpened,
            this, &QDesignerActions::updateCloseAction);
    connect(m_previewManager, &qdesigner_internal::PreviewManager::lastPreviewClosed,
            this, &QDesignerActions::updateCloseAction);

    connect(m_viewCppCodeAction, &QAction::triggered, this,
            [this] () { this->viewCode(qdesigner_internal::UicLanguage::Cpp); });
    connect(m_viewPythonCodeAction, &QAction::triggered, this,
            [this] () { this->viewCode(qdesigner_internal::UicLanguage::Python); });

    // Preview code only in Cpp/Python (uic)
    if (qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core) == nullptr) {
        m_formActions->addAction(m_viewCppCodeAction);
        m_formActions->addAction(m_viewPythonCodeAction);
    }

    m_formActions->addAction(createSeparator(this));

    m_formActions->addAction(ifwm->action(QDesignerFormWindowManagerInterface::FormWindowSettingsDialogAction));
//
// window actions
//
    m_minimizeAction->setEnabled(false);
    m_minimizeAction->setCheckable(true);
    m_minimizeAction->setShortcut(tr("CTRL+M"));
    connect(m_minimizeAction, &QAction::triggered, m_workbench, &QDesignerWorkbench::toggleFormMinimizationState);
    m_windowActions->addAction(m_minimizeAction);

    m_windowActions->addAction(m_bringAllToFrontSeparator);
    connect(m_bringAllToFrontAction, &QAction::triggered, m_workbench, &QDesignerWorkbench::bringAllToFront);
    m_windowActions->addAction(m_bringAllToFrontAction);
    m_windowActions->addAction(m_windowListSeparatorAction);

    setWindowListSeparatorVisible(false);

//
// connections
//
    fixActionContext(m_fileActions->actions());
    fixActionContext(m_editActions->actions());
    fixActionContext(m_toolActions->actions());
    fixActionContext(m_formActions->actions());
    fixActionContext(m_windowActions->actions());
    fixActionContext(m_helpActions->actions());

    activeFormWindowChanged(core()->formWindowManager()->activeFormWindow());

    m_backupTimer->start(180000); // 3min
    connect(m_backupTimer, &QTimer::timeout, this, &QDesignerActions::backupForms);

    // Enable application font action
    connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowAdded,
            this, &QDesignerActions::formWindowCountChanged);
    connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowRemoved,
            this, &QDesignerActions::formWindowCountChanged);
    formWindowCountChanged();
}

QActionGroup *QDesignerActions::createHelpActions()
{
    QActionGroup *helpActions = createActionGroup(this);

#ifndef QT_JAMBI_BUILD
    QAction *mainHelpAction = new QAction(tr("Qt Designer &Help"), this);
    mainHelpAction->setObjectName(u"__qt_designer_help_action"_s);
    connect(mainHelpAction, &QAction::triggered, this, &QDesignerActions::showDesignerHelp);
    mainHelpAction->setShortcut(Qt::CTRL | Qt::Key_Question);
    helpActions->addAction(mainHelpAction);

    helpActions->addAction(createSeparator(this));
    QAction *widgetHelp = new QAction(tr("Current Widget Help"), this);
    widgetHelp->setObjectName(u"__qt_current_widget_help_action"_s);
    widgetHelp->setShortcut(Qt::Key_F1);
    connect(widgetHelp, &QAction::triggered, this, &QDesignerActions::showWidgetSpecificHelp);
    helpActions->addAction(widgetHelp);

#endif

    helpActions->addAction(createSeparator(this));
    QAction *aboutPluginsAction = new QAction(tr("About Plugins"), this);
    aboutPluginsAction->setObjectName(u"__qt_about_plugins_action"_s);
    aboutPluginsAction->setMenuRole(QAction::ApplicationSpecificRole);
    connect(aboutPluginsAction, &QAction::triggered,
            m_core->formWindowManager(), &QDesignerFormWindowManagerInterface::showPluginDialog);
    helpActions->addAction(aboutPluginsAction);

    QAction *aboutDesignerAction = new QAction(tr("About Qt Designer"), this);
    aboutDesignerAction->setMenuRole(QAction::AboutRole);
    aboutDesignerAction->setObjectName(u"__qt_about_designer_action"_s);
    connect(aboutDesignerAction, &QAction::triggered, this, &QDesignerActions::aboutDesigner);
    helpActions->addAction(aboutDesignerAction);

    QAction *aboutQtAction = new QAction(tr("About Qt"), this);
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    aboutQtAction->setObjectName(u"__qt_about_qt_action"_s);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    helpActions->addAction(aboutQtAction);
    return helpActions;
}

QDesignerActions::~QDesignerActions()
{
#ifdef HAS_PRINTER
    delete m_printer;
#endif
}

QString QDesignerActions::uiExtension() const
{
    QDesignerLanguageExtension *lang
        = qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core);
    if (lang)
        return lang->uiExtension();
    return u"ui"_s;
}

QAction *QDesignerActions::createRecentFilesMenu()
{
    m_recentMenu.reset(new QMenu);
    QAction *act;
    // Need to insert this into the QAction.
    for (int i = 0; i < MaxRecentFiles; ++i) {
        act = new QAction(this);
        act->setVisible(false);
        connect(act, &QAction::triggered, this, &QDesignerActions::openRecentForm);
        m_recentFilesActions->addAction(act);
        m_recentMenu->addAction(act);
    }
    updateRecentFileActions();
    m_recentMenu->addSeparator();
    act = new QAction(QIcon::fromTheme(u"edit-clear"_s),
                      tr("Clear &Menu"), this);
    act->setObjectName(u"__qt_action_clear_menu_"_s);
    connect(act, &QAction::triggered, this, &QDesignerActions::clearRecentFiles);
    m_recentFilesActions->addAction(act);
    m_recentMenu->addAction(act);

    act = new QAction(QIcon::fromTheme(u"document-open-recent"_s),
                      tr("&Recent Forms"), this);
    act->setMenu(m_recentMenu.get());
    return act;
}

QActionGroup *QDesignerActions::toolActions() const
{ return m_toolActions; }

QDesignerWorkbench *QDesignerActions::workbench() const
{ return m_workbench; }

QDesignerFormEditorInterface *QDesignerActions::core() const
{ return m_core; }

QActionGroup *QDesignerActions::fileActions() const
{ return m_fileActions; }

QActionGroup *QDesignerActions::editActions() const
{ return m_editActions; }

QActionGroup *QDesignerActions::formActions() const
{ return m_formActions; }

QActionGroup *QDesignerActions::settingsActions() const
{  return m_settingsActions; }

QActionGroup *QDesignerActions::windowActions() const
{ return m_windowActions; }

QActionGroup *QDesignerActions::helpActions() const
{ return m_helpActions; }

QActionGroup *QDesignerActions::styleActions() const
{ return m_styleActions; }

QAction *QDesignerActions::previewFormAction() const
{ return m_previewFormAction; }

QAction *QDesignerActions::viewCodeAction() const
{ return m_viewCppCodeAction; }


void QDesignerActions::editWidgetsSlot()
{
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    for (int i=0; i<formWindowManager->formWindowCount(); ++i) {
        QDesignerFormWindowInterface *formWindow = formWindowManager->formWindow(i);
        formWindow->editWidgets();
    }
}

void QDesignerActions::createForm()
{
    showNewFormDialog(QString());
}

void QDesignerActions::showNewFormDialog(const QString &fileName)
{
    closePreview();
    NewForm *dlg = new NewForm(workbench(), workbench()->core()->topLevel(), fileName);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setAttribute(Qt::WA_ShowModal);

    dlg->setGeometry(fixDialogRect(dlg->rect()));
    dlg->exec();
}

void QDesignerActions::slotOpenForm()
{
    openForm(core()->topLevel());
}

bool QDesignerActions::openForm(QWidget *parent)
{
    closePreview();
    const QString extension = uiExtension();
    const QStringList fileNames = QFileDialog::getOpenFileNames(parent, tr("Open Form"),
        m_openDirectory, fileDialogFilters(extension), nullptr);

    if (fileNames.isEmpty())
        return false;

    bool atLeastOne = false;
    for (const QString &fileName : fileNames) {
        if (readInForm(fileName) && !atLeastOne)
            atLeastOne = true;
    }

    return atLeastOne;
}

bool QDesignerActions::saveFormAs(QDesignerFormWindowInterface *fw)
{
    const QString extension = uiExtension();

    QString dir = fw->fileName();
    if (dir.isEmpty()) {
        do {
            // Build untitled name
            if (!m_saveDirectory.isEmpty()) {
                dir = m_saveDirectory;
                break;
            }
            if (!m_openDirectory.isEmpty()) {
                dir = m_openDirectory;
                break;
            }
            dir = QDir::current().absolutePath();
        } while (false);
        dir += QDir::separator();
        dir += "untitled."_L1;
        dir += extension;
    }

    QScopedPointer<QFileDialog> saveAsDialog(createSaveAsDialog(fw, dir, extension));
    if (saveAsDialog->exec() != QDialog::Accepted)
        return false;

    const QString saveFile = saveAsDialog->selectedFiles().constFirst();
    saveAsDialog.reset(); // writeOutForm potentially shows other dialogs

    fw->setFileName(saveFile);
    return writeOutForm(fw, saveFile);
}

void QDesignerActions::saveForm()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        if (saveForm(fw))
            showStatusBarMessage(savedMessage(QFileInfo(fw->fileName()).fileName()));
    }
}

void QDesignerActions::saveAllForms()
{
    QString fileNames;
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    if (const int totalWindows = formWindowManager->formWindowCount()) {
        const auto separator = ", "_L1;
        for (int i = 0; i < totalWindows; ++i) {
            QDesignerFormWindowInterface *fw = formWindowManager->formWindow(i);
            if (fw && fw->isDirty()) {
                formWindowManager->setActiveFormWindow(fw);
                if (saveForm(fw)) {
                    if (!fileNames.isEmpty())
                        fileNames += separator;
                    fileNames += QFileInfo(fw->fileName()).fileName();
                } else {
                    break;
                }
            }
        }
    }

    if (!fileNames.isEmpty()) {
        showStatusBarMessage(savedMessage(fileNames));
    }
}

bool QDesignerActions::saveForm(QDesignerFormWindowInterface *fw)
{
    bool ret;
    if (fw->fileName().isEmpty())
        ret = saveFormAs(fw);
    else
        ret =  writeOutForm(fw, fw->fileName());
    return ret;
}

void QDesignerActions::closeForm()
{
    if (m_previewManager->previewCount()) {
        closePreview();
        return;
    }

    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow())
        if (QWidget *parent = fw->parentWidget()) {
            if (QMdiSubWindow *mdiSubWindow = qobject_cast<QMdiSubWindow *>(parent->parentWidget())) {
                mdiSubWindow->close();
            } else {
                parent->close();
            }
        }
}

void QDesignerActions::saveFormAs()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        if (saveFormAs(fw))
            showStatusBarMessage(savedMessage(fw->fileName()));
    }
}

void QDesignerActions::saveFormAsTemplate()
{
    if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
        SaveFormAsTemplate dlg(core(), fw, fw->window());
        dlg.exec();
    }
}

void QDesignerActions::notImplementedYet()
{
    QMessageBox::information(core()->topLevel(), tr("Designer"), tr("Feature not implemented yet!"));
}

void QDesignerActions::closePreview()
{
    m_previewManager->closeAllPreviews();
}

void QDesignerActions::viewCode(qdesigner_internal::UicLanguage language)
{
    QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
    if (!fw)
        return;
    QString errorMessage;
    if (!qdesigner_internal::CodeDialog::showCodeDialog(fw, language, fw, &errorMessage))
        QMessageBox::warning(fw, tr("Code generation failed"), errorMessage);
}

bool QDesignerActions::readInForm(const QString &fileName)
{
    QString fn = fileName;

    // First make sure that we don't have this one open already.
    QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
    const int totalWindows = formWindowManager->formWindowCount();
    for (int i = 0; i < totalWindows; ++i) {
        QDesignerFormWindowInterface *w = formWindowManager->formWindow(i);
        if (w->fileName() == fn) {
            w->raise();
            formWindowManager->setActiveFormWindow(w);
            addRecentFile(fn);
            return true;
        }
    }

    // Otherwise load it.
    do {
        QString errorMessage;
        if (workbench()->openForm(fn, &errorMessage)) {
            addRecentFile(fn);
            m_openDirectory = QFileInfo(fn).absolutePath();
            return true;
        } else {
            // prompt to reload
            QMessageBox box(QMessageBox::Warning, tr("Read error"),
                            tr("%1\nDo you want to update the file location or generate a new form?").arg(errorMessage),
                            QMessageBox::Cancel, core()->topLevel());

            QPushButton *updateButton = box.addButton(tr("&Update"), QMessageBox::ActionRole);
            QPushButton *newButton    = box.addButton(tr("&New Form"), QMessageBox::ActionRole);
            box.exec();
            if (box.clickedButton() == box.button(QMessageBox::Cancel))
                return false;

            if (box.clickedButton() == updateButton) {
                const QString extension = uiExtension();
                fn = QFileDialog::getOpenFileName(core()->topLevel(),
                                                  tr("Open Form"), m_openDirectory,
                                                  fileDialogFilters(extension), nullptr);

                if (fn.isEmpty())
                    return false;
            } else if (box.clickedButton() == newButton) {
                // If the file does not exist, but its directory, is valid, open the template with the editor file name set to it.
                // (called from command line).
                QString newFormFileName;
                const  QFileInfo fInfo(fn);
                if (!fInfo.exists()) {
                    // Normalize file name
                    const QString directory = fInfo.absolutePath();
                    if (QDir(directory).exists())
                        newFormFileName = directory + u'/' + fInfo.fileName();
                }
                showNewFormDialog(newFormFileName);
                return false;
            }
        }
    } while (true);
    return true;
}

bool QDesignerActions::writeOutForm(QDesignerFormWindowInterface *fw, const QString &saveFile, bool check)
{
    Q_ASSERT(fw && !saveFile.isEmpty());

    if (check) {
        const QStringList problems = fw->checkContents();
        if (!problems.isEmpty())
            QMessageBox::information(fw->window(), tr("Qt Designer"), problems.join("<br>"_L1));
    }

    QString contents = fw->contents();
    if (qdesigner_internal::FormWindowBase *fwb = qobject_cast<qdesigner_internal::FormWindowBase *>(fw)) {
        if (fwb->lineTerminatorMode() == qdesigner_internal::FormWindowBase::CRLFLineTerminator)
            contents.replace('\n'_L1, "\r\n"_L1);
    }
    m_workbench->updateBackup(fw);

    QSaveFile f(saveFile);
    while (!f.open(QFile::WriteOnly)) {
        QMessageBox box(QMessageBox::Warning,
                        tr("Save Form?"),
                        tr("Could not open file"),
                        QMessageBox::NoButton, fw);

        box.setWindowModality(Qt::WindowModal);
        box.setInformativeText(tr("The file %1 could not be opened."
                               "\nReason: %2"
                               "\nWould you like to retry or select a different file?")
                                .arg(f.fileName(), f.errorString()));
        QPushButton *retryButton = box.addButton(QMessageBox::Retry);
        retryButton->setDefault(true);
        QPushButton *switchButton = box.addButton(tr("Select New File"), QMessageBox::AcceptRole);
        QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
        box.exec();

        if (box.clickedButton() == cancelButton)
            return false;
        if (box.clickedButton() == switchButton) {
            QScopedPointer<QFileDialog> saveAsDialog(createSaveAsDialog(fw, QDir::currentPath(), uiExtension()));
            if (saveAsDialog->exec() != QDialog::Accepted)
                return false;

            const QString fileName = saveAsDialog->selectedFiles().constFirst();
            f.setFileName(fileName);
            fw->setFileName(fileName);
        }
        // loop back around...
    }
    f.write(contents.toUtf8());
    if (!f.commit()) {
        QMessageBox box(QMessageBox::Warning, tr("Save Form"),
                        tr("Could not write file"),
                        QMessageBox::Cancel, fw);
        box.setWindowModality(Qt::WindowModal);
        box.setInformativeText(tr("It was not possible to write the file %1 to disk."
                                "\nReason: %2")
                                .arg(f.fileName(), f.errorString()));
        box.exec();
        return false;
    }
    addRecentFile(saveFile);
    m_saveDirectory = QFileInfo(f.fileName()).absolutePath();

    fw->setDirty(false);
    fw->parentWidget()->setWindowModified(false);
    return true;
}

void QDesignerActions::shutdown()
{
    // Follow the idea from the Mac, i.e. send the Application a close event
    // and if it's accepted, quit.
    QCloseEvent ev;
    QApplication::sendEvent(qDesigner, &ev);
    if (ev.isAccepted())
        qDesigner->quit();
}

void QDesignerActions::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    const bool enable = formWindow != nullptr;
    m_saveFormAction->setEnabled(enable);
    m_saveFormAsAction->setEnabled(enable);
    m_saveAllFormsAction->setEnabled(enable);
    m_saveFormAsTemplateAction->setEnabled(enable);
    m_closeFormAction->setEnabled(enable);
    m_savePreviewImageAction->setEnabled(enable);
    m_printPreviewAction->setEnabled(enable);

    m_editWidgetsAction->setEnabled(enable);

    m_previewFormAction->setEnabled(enable);
    m_viewCppCodeAction->setEnabled(enable);
    m_viewPythonCodeAction->setEnabled(enable);
    m_styleActions->setEnabled(enable);
}

void QDesignerActions::formWindowSettingsChanged(QDesignerFormWindowInterface *fw)
{
    if (QDesignerFormWindow *window = m_workbench->findFormWindow(fw))
        window->updateChanged();
}

void QDesignerActions::updateRecentFileActions()
{
    QStringList files = m_settings.recentFilesList();
    auto existingEnd = std::remove_if(files.begin(), files.end(),
                                      [] (const QString &f) { return !QFileInfo::exists(f); });
    if (existingEnd != files.end()) {
        files.erase(existingEnd, files.end());
        m_settings.setRecentFilesList(files);
    }

    const auto recentFilesActs = m_recentFilesActions->actions();
    qsizetype i = 0;
    for (QAction *action : recentFilesActs) {
        if (i < files.size()) {
            const QString &file = files.at(i);
            action->setText(QFileInfo(file).fileName());
            action->setIconText(file);
            action->setVisible(true);
        } else {
            action->setVisible(false);
        }
        ++i;
    }
}

void QDesignerActions::openRecentForm()
{
    if (const QAction *action = qobject_cast<const QAction *>(sender())) {
        if (!readInForm(action->iconText()))
            updateRecentFileActions(); // File doesn't exist, remove it from settings
    }
}

void QDesignerActions::clearRecentFiles()
{
    m_settings.setRecentFilesList(QStringList());
    updateRecentFileActions();
}

QActionGroup *QDesignerActions::recentFilesActions() const
{
    return m_recentFilesActions;
}

void QDesignerActions::addRecentFile(const QString &fileName)
{
    QStringList files = m_settings.recentFilesList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    m_settings.setRecentFilesList(files);
    updateRecentFileActions();
}

QAction *QDesignerActions::openFormAction() const
{
    return  m_openFormAction;
}

QAction *QDesignerActions::closeFormAction() const
{
    return m_closeFormAction;
}

QAction *QDesignerActions::minimizeAction() const
{
    return m_minimizeAction;
}

void QDesignerActions::showDesignerHelp()
{
    QString url = AssistantClient::designerManualUrl();
    url += "qtdesigner-manual.html"_L1;
    showHelp(url);
}

void QDesignerActions::helpRequested(const QString &manual, const QString &document)
{
    QString url = AssistantClient::documentUrl(manual);
    url += document;
    showHelp(url);
}

void QDesignerActions::showHelp(const QString &url)
{
    QString errorMessage;
    if (!m_assistantClient.showPage(url, &errorMessage))
        QMessageBox::warning(core()->topLevel(), tr("Assistant"), errorMessage);
}

void QDesignerActions::aboutDesigner()
{
    VersionDialog mb(core()->topLevel());
    mb.setWindowTitle(tr("About Qt Designer"));
    if (mb.exec()) {
        QMessageBox messageBox(QMessageBox::Information, u"Easter Egg"_s,
                               u"Easter Egg"_s, QMessageBox::Ok, core()->topLevel());
        messageBox.setInformativeText(u"The Easter Egg has been removed."_s);
        messageBox.exec();
    }
}

QAction *QDesignerActions::editWidgets() const
{
    return m_editWidgetsAction;
}

void QDesignerActions::showWidgetSpecificHelp()
{
    const QString helpId = core()->integration()->contextHelpId();

    if (helpId.isEmpty()) {
        showDesignerHelp();
        return;
    }

    QString errorMessage;
    const bool rc = m_assistantClient.activateIdentifier(helpId, &errorMessage);
    if (!rc)
        QMessageBox::warning(core()->topLevel(), tr("Assistant"), errorMessage);
}

void QDesignerActions::updateCloseAction()
{
    if (m_previewManager->previewCount()) {
        m_closeFormAction->setText(tr("&Close Preview"));
    } else {
        m_closeFormAction->setText(tr("&Close"));
    }
}

void QDesignerActions::backupForms()
{
    const int count = m_workbench->formWindowCount();
    if (!count || !ensureBackupDirectories())
        return;


    QStringList tmpFiles;
    QMap<QString, QString> backupMap;
    QDir backupDir(m_backupPath);
    for (int i = 0; i < count; ++i) {
        QDesignerFormWindow *fw = m_workbench->formWindow(i);
        QDesignerFormWindowInterface *fwi = fw->editor();

        QString formBackupName;
        QTextStream(&formBackupName) << m_backupPath << QDir::separator()
                                     << "backup" << i << ".bak";

        QString fwn = QDir::toNativeSeparators(fwi->fileName());
        if (fwn.isEmpty())
            fwn = fw->windowTitle();

        backupMap.insert(fwn, formBackupName);

        QFile file(formBackupName.replace(m_backupPath, m_backupTmpPath));
        if (file.open(QFile::WriteOnly)){
            QString contents = fixResourceFileBackupPath(fwi, backupDir);
            if (qdesigner_internal::FormWindowBase *fwb = qobject_cast<qdesigner_internal::FormWindowBase *>(fwi)) {
                if (fwb->lineTerminatorMode() == qdesigner_internal::FormWindowBase::CRLFLineTerminator)
                    contents.replace('\n'_L1, "\r\n"_L1);
            }
            const QByteArray utf8Array = contents.toUtf8();
            if (file.write(utf8Array, utf8Array.size()) != utf8Array.size()) {
                backupMap.remove(fwn);
                qdesigner_internal::designerWarning(tr("The backup file %1 could not be written.").arg(file.fileName()));
            } else
                tmpFiles.append(formBackupName);

            file.close();
        }
    }
    if(!tmpFiles.isEmpty()) {
        const QStringList backupFiles = backupDir.entryList(QDir::Files);
        for (const QString &backupFile : backupFiles)
            backupDir.remove(backupFile);

        for (const QString &tmpName : std::as_const(tmpFiles)) {
            QString name(tmpName);
            name.replace(m_backupTmpPath, m_backupPath);
            QFile tmpFile(tmpName);
            if (!tmpFile.copy(name))
                qdesigner_internal::designerWarning(tr("The backup file %1 could not be written.").arg(name));
            tmpFile.remove();
        }

        m_settings.setBackup(backupMap);
    }
}

QString QDesignerActions::fixResourceFileBackupPath(QDesignerFormWindowInterface *fwi, const QDir& backupDir)
{
    const QString content = fwi->contents();
    QDomDocument domDoc(u"backup"_s);
    if(!domDoc.setContent(content))
        return content;

    const QDomNodeList list = domDoc.elementsByTagName(u"resources"_s);
    if (list.isEmpty())
        return content;

    for (int i = 0; i < list.count(); i++) {
        const QDomNode node = list.at(i);
        if (!node.isNull()) {
            const QDomElement element = node.toElement();
            if (!element.isNull() && element.tagName() == "resources"_L1) {
                QDomNode childNode = element.firstChild();
                while (!childNode.isNull()) {
                    QDomElement childElement = childNode.toElement();
                    if (!childElement.isNull() && childElement.tagName() == "include"_L1) {
                        const QString attr = childElement.attribute(u"location"_s);
                        const QString path = fwi->absoluteDir().absoluteFilePath(attr);
                        childElement.setAttribute(u"location"_s, backupDir.relativeFilePath(path));
                    }
                    childNode = childNode.nextSibling();
                }
            }
        }
    }


    return domDoc.toString();
}

QRect QDesignerActions::fixDialogRect(const QRect &rect) const
{
    QRect frameGeometry;
    const QRect availableGeometry = core()->topLevel()->screen()->geometry();

    if (workbench()->mode() == DockedMode) {
        frameGeometry = core()->topLevel()->frameGeometry();
    } else
        frameGeometry = availableGeometry;

    QRect dlgRect = rect;
    dlgRect.moveCenter(frameGeometry.center());

    // make sure that parts of the dialog are not outside of screen
    dlgRect.moveBottom(qMin(dlgRect.bottom(), availableGeometry.bottom()));
    dlgRect.moveRight(qMin(dlgRect.right(), availableGeometry.right()));
    dlgRect.moveLeft(qMax(dlgRect.left(), availableGeometry.left()));
    dlgRect.moveTop(qMax(dlgRect.top(), availableGeometry.top()));

    return dlgRect;
}

void QDesignerActions::showStatusBarMessage(const QString &message) const
{
    if (workbench()->mode() == DockedMode) {
        QStatusBar *bar = qDesigner->mainWindow()->statusBar();
        if (bar && !bar->isHidden())
            bar->showMessage(message, 3000);
    }
}

void QDesignerActions::setBringAllToFrontVisible(bool visible)
{
      m_bringAllToFrontSeparator->setVisible(visible);
      m_bringAllToFrontAction->setVisible(visible);
}

void QDesignerActions::setWindowListSeparatorVisible(bool visible)
{
    m_windowListSeparatorAction->setVisible(visible);
}

bool QDesignerActions::ensureBackupDirectories() {

    if (m_backupPath.isEmpty()) {
        // create names
        m_backupPath = qdesigner_internal::dataDirectory() + u"/backup"_s;
        m_backupTmpPath = m_backupPath + u"/tmp"_s;
    }

    // ensure directories
    const QDir backupDir(m_backupPath);
    const QDir backupTmpDir(m_backupTmpPath);

    if (!backupDir.exists()) {
        if (!backupDir.mkpath(m_backupPath)) {
            qdesigner_internal::designerWarning(tr("The backup directory %1 could not be created.")
                                                .arg(QDir::toNativeSeparators(m_backupPath)));
            return false;
        }
    }
    if (!backupTmpDir.exists()) {
        if (!backupTmpDir.mkpath(m_backupTmpPath)) {
            qdesigner_internal::designerWarning(tr("The temporary backup directory %1 could not be created.")
                                                .arg(QDir::toNativeSeparators(m_backupTmpPath)));
            return false;
        }
    }
    return true;
}

void QDesignerActions::showPreferencesDialog()
{
    {
        PreferencesDialog preferencesDialog(workbench()->core(), m_core->topLevel());
        preferencesDialog.exec();
    }   // Make sure the preference dialog is destroyed before switching UI modes.
    m_workbench->applyUiSettings();
}

void QDesignerActions::showAppFontDialog()
{
    if (!m_appFontDialog) // Might get deleted when switching ui modes
        m_appFontDialog = new AppFontDialog(core()->topLevel());
    m_appFontDialog->show();
    m_appFontDialog->raise();
}

QPixmap QDesignerActions::createPreviewPixmap(QDesignerFormWindowInterface *fw)
{
    const QCursor oldCursor = core()->topLevel()->cursor();
    core()->topLevel()->setCursor(Qt::WaitCursor);

    QString errorMessage;
    const QPixmap pixmap = m_previewManager->createPreviewPixmap(fw, QString(), &errorMessage);
    core()->topLevel()->setCursor(oldCursor);
    if (pixmap.isNull()) {
        QMessageBox::warning(fw, tr("Preview failed"), errorMessage);
    }
    return pixmap;
}

qdesigner_internal::PreviewConfiguration QDesignerActions::previewConfiguration()
{
    qdesigner_internal::PreviewConfiguration pc;
    qdesigner_internal::QDesignerSharedSettings settings(core());
    if (settings.isCustomPreviewConfigurationEnabled())
        pc = settings.customPreviewConfiguration();
    return pc;
}

void QDesignerActions::savePreviewImage()
{
    const char *format = "png";

    QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
    if (!fw)
        return;

    QImage image;
    const QString extension = QString::fromLatin1(format);
    const QString filter = tr("Image files (*.%1)").arg(extension);

    QString suggestion = fw->fileName();
    if (!suggestion.isEmpty())
        suggestion = QFileInfo(suggestion).baseName() + u'.' + extension;

    QFileDialog dialog(fw, tr("Save Image"), suggestion, filter);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(extension);

    do {
        if (dialog.exec() != QDialog::Accepted)
            break;
        const QString fileName = dialog.selectedFiles().constFirst();

        if (image.isNull()) {
            const QPixmap pixmap = createPreviewPixmap(fw);
            if (pixmap.isNull())
                break;

            image = pixmap.toImage();
        }

        if (image.save(fileName, format)) {
            showStatusBarMessage(tr("Saved image %1.").arg(QFileInfo(fileName).fileName()));
            break;
        }

        QMessageBox box(QMessageBox::Warning, tr("Save Image"),
                        tr("The file %1 could not be written.").arg( fileName),
                        QMessageBox::Retry|QMessageBox::Cancel, fw);
        if (box.exec() == QMessageBox::Cancel)
            break;
    } while (true);
}

void QDesignerActions::formWindowCountChanged()
{
    const bool enabled = m_core->formWindowManager()->formWindowCount() == 0;
    /* Disable the application font action if there are form windows open
     * as the reordering of the fonts sets font properties to 'changed'
     * and overloaded fonts are not updated. */
    static const QString disabledTip = tr("Please close all forms to enable the loading of additional fonts.");
    m_appFontAction->setEnabled(enabled);
    m_appFontAction->setStatusTip(enabled ? QString() : disabledTip);
}

void QDesignerActions::printPreviewImage()
{
#ifdef HAS_PRINTER
    QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
    if (!fw)
        return;

    if (!m_printer)
        m_printer = new QPrinter(QPrinter::HighResolution);

    m_printer->setFullPage(false);

    // Grab the image to be able to a suggest suitable orientation
    const QPixmap pixmap = createPreviewPixmap(fw);
    if (pixmap.isNull())
        return;

    const QSizeF pixmapSize = pixmap.size();

    m_printer->setPageOrientation(pixmapSize.width() > pixmapSize.height() ?
                                  QPageLayout::Landscape : QPageLayout::Portrait);

    // Printer parameters
    QPrintDialog dialog(m_printer, fw);
    if (!dialog.exec())
        return;

    const QCursor oldCursor = core()->topLevel()->cursor();
    core()->topLevel()->setCursor(Qt::WaitCursor);
    // Estimate of required scaling to make form look the same on screen and printer.
    const double suggestedScaling = static_cast<double>(m_printer->physicalDpiX()) /  static_cast<double>(fw->physicalDpiX());

    QPainter painter(m_printer);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Clamp to page
    const QRectF page =  painter.viewport();
    const double maxScaling = qMin(page.size().width() / pixmapSize.width(), page.size().height() / pixmapSize.height());
    const double scaling = qMin(suggestedScaling, maxScaling);

    const double xOffset = page.left() + qMax(0.0, (page.size().width()  - scaling * pixmapSize.width())  / 2.0);
    const double yOffset = page.top()  + qMax(0.0, (page.size().height() - scaling * pixmapSize.height()) / 2.0);

    // Draw.
    painter.translate(xOffset, yOffset);
    painter.scale(scaling, scaling);
    painter.drawPixmap(0, 0, pixmap);
    core()->topLevel()->setCursor(oldCursor);

    showStatusBarMessage(tr("Printed %1.").arg(QFileInfo(fw->fileName()).fileName()));
#endif // HAS_PRINTER
}

QT_END_NAMESPACE
