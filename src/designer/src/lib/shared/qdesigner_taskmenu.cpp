/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#include "qdesigner_taskmenu_p.h"
#include "qdesigner_command_p.h"
#include "qdesigner_command2_p.h"
#include "richtexteditor_p.h"
#include "plaintexteditor_p.h"
#include "stylesheeteditor_p.h"
#include "qlayout_widget_p.h"
#include "layout_p.h"
#include "selectsignaldialog_p.h"
#include "spacer_widget_p.h"
#include "textpropertyeditor_p.h"
#include "promotiontaskmenu_p.h"
#include "metadatabase_p.h"
#include "signalslotdialog_p.h"
#include "qdesigner_membersheet_p.h"
#include "qdesigner_propertycommand_p.h"
#include "qdesigner_utils_p.h"
#include "qdesigner_objectinspector_p.h"
#include "morphmenu_p.h"
#include "formlayoutmenu_p.h"
#include "widgetfactory_p.h"
#include "abstractintrospection_p.h"
#include "widgetdatabase_p.h"

#include <shared_enums_p.h>

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qundostack.h>

#include <QtCore/qdebug.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

static inline QAction *createSeparatorHelper(QObject *parent) {
    QAction *rc = new QAction(parent);
    rc->setSeparator(true);
    return rc;
}

static QString objName(const QDesignerFormEditorInterface *core, QObject *object) {
    QDesignerPropertySheetExtension *sheet
            = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(sheet != nullptr);

    const QString objectNameProperty = QStringLiteral("objectName");
    const int index = sheet->indexOf(objectNameProperty);
    const QVariant v = sheet->property(index);
    if (v.canConvert<qdesigner_internal::PropertySheetStringValue>())
        return v.value<qdesigner_internal::PropertySheetStringValue>().value();
    return v.toString();
}

enum { ApplyMinimumWidth = 0x1, ApplyMinimumHeight = 0x2, ApplyMaximumWidth = 0x4, ApplyMaximumHeight = 0x8 };

namespace  {
// --------------- ObjectNameDialog
class ObjectNameDialog : public QDialog
{
     public:
         ObjectNameDialog(QWidget *parent, const QString &oldName);
         QString newObjectName() const;

     private:
         qdesigner_internal::TextPropertyEditor *m_editor;
};

ObjectNameDialog::ObjectNameDialog(QWidget *parent, const QString &oldName)
    : QDialog(parent),
      m_editor( new qdesigner_internal::TextPropertyEditor(this, qdesigner_internal::TextPropertyEditor::EmbeddingNone,
                                                           qdesigner_internal::ValidationObjectName))
{
    setWindowTitle(QCoreApplication::translate("ObjectNameDialog", "Change Object Name"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(new QLabel(QCoreApplication::translate("ObjectNameDialog", "Object Name")));

    m_editor->setText(oldName);
    m_editor->selectAll();
    m_editor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    vboxLayout->addWidget(m_editor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    vboxLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString ObjectNameDialog::newObjectName() const
{
    return m_editor->text();
}
} // namespace

namespace qdesigner_internal {

// Sub menu displaying the alignment options of a widget in a managed
// grid/box layout cell.
class LayoutAlignmentMenu {
public:
    explicit LayoutAlignmentMenu(QObject *parent);

    QAction *subMenuAction() const { return m_subMenuAction; }

    void connect(QObject *receiver, const char *aSlot);

    // Set up enabled state and checked actions according to widget (managed box/grid)
    bool setAlignment(const QDesignerFormEditorInterface *core, QWidget *w);

    // Return the currently checked alignment
    Qt::Alignment alignment() const;

private:
    enum Actions { HorizNone, Left, HorizCenter, Right, VerticalNone, Top, VerticalCenter, Bottom };
    static QAction *createAction(const QString &text, int data, QMenu *menu, QActionGroup *ag);

    QAction *m_subMenuAction;
    QActionGroup *m_horizGroup;
    QActionGroup *m_verticalGroup;
    QAction *m_actions[Bottom + 1];
};

QAction *LayoutAlignmentMenu::createAction(const QString &text, int data, QMenu *menu, QActionGroup *ag)
{
    QAction * a = new QAction(text, nullptr);
    a->setCheckable(true);
    a->setData(QVariant(data));
    menu->addAction(a);
    ag->addAction(a);
    return a;
}

LayoutAlignmentMenu::LayoutAlignmentMenu(QObject *parent) :
    m_subMenuAction(new QAction(QDesignerTaskMenu::tr("Layout Alignment"), parent)),
    m_horizGroup(new QActionGroup(parent)),
    m_verticalGroup(new QActionGroup(parent))
{
    m_horizGroup->setExclusive(true);
    m_verticalGroup->setExclusive(true);

    QMenu *menu = new QMenu;
    m_subMenuAction->setMenu(menu);

    m_actions[HorizNone] = createAction(QDesignerTaskMenu::tr("No Horizontal Alignment"), 0, menu, m_horizGroup);
    m_actions[Left] = createAction(QDesignerTaskMenu::tr("Left"), Qt::AlignLeft, menu, m_horizGroup);
    m_actions[HorizCenter] = createAction(QDesignerTaskMenu::tr("Center Horizontally"), Qt::AlignHCenter, menu, m_horizGroup);
    m_actions[Right] = createAction(QDesignerTaskMenu::tr("Right"), Qt::AlignRight, menu, m_horizGroup);
    menu->addSeparator();
    m_actions[VerticalNone] = createAction(QDesignerTaskMenu::tr("No Vertical Alignment"), 0, menu, m_verticalGroup);
    m_actions[Top] = createAction(QDesignerTaskMenu::tr("Top"), Qt::AlignTop, menu, m_verticalGroup);
    m_actions[VerticalCenter] = createAction(QDesignerTaskMenu::tr("Center Vertically"), Qt::AlignVCenter, menu, m_verticalGroup);
    m_actions[Bottom] = createAction(QDesignerTaskMenu::tr("Bottom"), Qt::AlignBottom, menu, m_verticalGroup);
}

void LayoutAlignmentMenu::connect(QObject *receiver, const char *aSlot)
{
    QObject::connect(m_horizGroup, SIGNAL(triggered(QAction*)), receiver, aSlot);
    QObject::connect(m_verticalGroup, SIGNAL(triggered(QAction*)), receiver, aSlot);
}

bool LayoutAlignmentMenu::setAlignment(const QDesignerFormEditorInterface *core, QWidget *w)
{
    bool enabled;
    const Qt::Alignment alignment = LayoutAlignmentCommand::alignmentOf(core, w, &enabled);
    m_subMenuAction->setEnabled(enabled);
    if (!enabled) {
        m_actions[HorizNone]->setChecked(true);
        m_actions[VerticalNone]->setChecked(true);
        return false;
    }
    // Get alignment
    switch (alignment & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft:
        m_actions[Left]->setChecked(true);
        break;
    case Qt::AlignHCenter:
        m_actions[HorizCenter]->setChecked(true);
        break;
    case Qt::AlignRight:
        m_actions[Right]->setChecked(true);
        break;
    default:
        m_actions[HorizNone]->setChecked(true);
        break;
    }
    switch (alignment & Qt::AlignVertical_Mask) {
    case Qt::AlignTop:
        m_actions[Top]->setChecked(true);
        break;
    case Qt::AlignVCenter:
        m_actions[VerticalCenter]->setChecked(true);
        break;
    case Qt::AlignBottom:
        m_actions[Bottom]->setChecked(true);
        break;
    default:
        m_actions[VerticalNone]->setChecked(true);
        break;
    }
    return true;
}

Qt::Alignment LayoutAlignmentMenu::alignment() const
{
    Qt::Alignment alignment;
    if (const QAction *horizAction = m_horizGroup->checkedAction())
        if (const int horizAlign = horizAction->data().toInt())
            alignment |= static_cast<Qt::Alignment>(horizAlign);
    if (const QAction *vertAction = m_verticalGroup->checkedAction())
        if (const int vertAlign = vertAction->data().toInt())
            alignment |= static_cast<Qt::Alignment>(vertAlign);
    return alignment;
}

// -------------- QDesignerTaskMenuPrivate
class QDesignerTaskMenuPrivate {
public:
    QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent);

    QDesignerTaskMenu *m_q;
    QPointer<QWidget> m_widget;
    QAction *m_separator;
    QAction *m_separator2;
    QAction *m_separator3;
    QAction *m_separator4;
    QAction *m_separator5;
    QAction *m_separator6;
    QAction *m_separator7;
    QAction *m_changeObjectNameAction;
    QAction *m_changeToolTip;
    QAction *m_changeWhatsThis;
    QAction *m_changeStyleSheet;
    MorphMenu *m_morphMenu;
    FormLayoutMenu *m_formLayoutMenu;

    QAction *m_addMenuBar;
    QAction *m_addToolBar;
    QAction *m_addAreaSubMenu;
    QAction *m_addStatusBar;
    QAction *m_removeStatusBar;
    QAction *m_containerFakeMethods;
    QAction *m_navigateToSlot;
    PromotionTaskMenu* m_promotionTaskMenu;
    QActionGroup *m_sizeActionGroup;
    LayoutAlignmentMenu m_layoutAlignmentMenu;
    QAction *m_sizeActionsSubMenu;
};

QDesignerTaskMenuPrivate::QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent) :
    m_q(nullptr),
    m_widget(widget),
    m_separator(createSeparatorHelper(parent)),
    m_separator2(createSeparatorHelper(parent)),
    m_separator3(createSeparatorHelper(parent)),
    m_separator4(createSeparatorHelper(parent)),
    m_separator5(createSeparatorHelper(parent)),
    m_separator6(createSeparatorHelper(parent)),
    m_separator7(createSeparatorHelper(parent)),
    m_changeObjectNameAction(new QAction(QDesignerTaskMenu::tr("Change objectName..."), parent)),
    m_changeToolTip(new QAction(QDesignerTaskMenu::tr("Change toolTip..."), parent)),
    m_changeWhatsThis(new QAction(QDesignerTaskMenu::tr("Change whatsThis..."), parent)),
    m_changeStyleSheet(new QAction(QDesignerTaskMenu::tr("Change styleSheet..."), parent)),
    m_morphMenu(new MorphMenu(parent)),
    m_formLayoutMenu(new FormLayoutMenu(parent)),
    m_addMenuBar(new QAction(QDesignerTaskMenu::tr("Create Menu Bar"), parent)),
    m_addToolBar(new QAction(QDesignerTaskMenu::tr("Add Tool Bar"), parent)),
    m_addAreaSubMenu(new QAction(QDesignerTaskMenu::tr("Add Tool Bar to Other Area"), parent)),
    m_addStatusBar(new QAction(QDesignerTaskMenu::tr("Create Status Bar"), parent)),
    m_removeStatusBar(new QAction(QDesignerTaskMenu::tr("Remove Status Bar"), parent)),
    m_containerFakeMethods(new QAction(QDesignerTaskMenu::tr("Change signals/slots..."), parent)),
    m_navigateToSlot(new QAction(QDesignerTaskMenu::tr("Go to slot..."), parent)),
    m_promotionTaskMenu(new PromotionTaskMenu(widget, PromotionTaskMenu::ModeManagedMultiSelection, parent)),
    m_sizeActionGroup(new QActionGroup(parent)),
    m_layoutAlignmentMenu(parent),
    m_sizeActionsSubMenu(new QAction(QDesignerTaskMenu::tr("Size Constraints"), parent))
{
    QMenu *sizeMenu = new QMenu;
    m_sizeActionsSubMenu->setMenu(sizeMenu);
    QAction *sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Width"));
    sizeAction->setData(ApplyMinimumWidth);
    sizeMenu->addAction(sizeAction);

    sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Height"));
    sizeAction->setData(ApplyMinimumHeight);
    sizeMenu->addAction(sizeAction);

    sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Size"));
    sizeAction->setData(ApplyMinimumWidth|ApplyMinimumHeight);
    sizeMenu->addAction(sizeAction);

    sizeMenu->addSeparator();

    sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Width"));
    sizeAction->setData(ApplyMaximumWidth);
    sizeMenu->addAction(sizeAction);

    sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Height"));
    sizeAction->setData(ApplyMaximumHeight);
    sizeMenu->addAction(sizeAction);

    sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Size"));
    sizeAction->setData(ApplyMaximumWidth|ApplyMaximumHeight);
    sizeMenu->addAction(sizeAction);
}

// --------- QDesignerTaskMenu
QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent) :
    QObject(parent),
    d(new QDesignerTaskMenuPrivate(widget, parent))
{
    d->m_q = this;
    Q_ASSERT(qobject_cast<QDesignerFormWindowInterface*>(widget) == 0);

    connect(d->m_changeObjectNameAction, &QAction::triggered, this, &QDesignerTaskMenu::changeObjectName);
    connect(d->m_changeToolTip, &QAction::triggered, this, &QDesignerTaskMenu::changeToolTip);
    connect(d->m_changeWhatsThis, &QAction::triggered, this, &QDesignerTaskMenu::changeWhatsThis);
    connect(d->m_changeStyleSheet, &QAction::triggered, this, &QDesignerTaskMenu::changeStyleSheet);
    connect(d->m_addMenuBar, &QAction::triggered, this, &QDesignerTaskMenu::createMenuBar);
    connect(d->m_addToolBar, &QAction::triggered, this,
            [this] () { this->addToolBar(Qt::TopToolBarArea); });
    auto areaMenu = new QMenu;
    d->m_addAreaSubMenu->setMenu(areaMenu);
    areaMenu->addAction(QDesignerTaskMenu::tr("Left"),
                        [this] () { this->addToolBar(Qt::LeftToolBarArea); });
    areaMenu->addAction(QDesignerTaskMenu::tr("Right"),
                        [this] () { this->addToolBar(Qt::RightToolBarArea); });
    areaMenu->addAction(QDesignerTaskMenu::tr("Bottom"),
                        [this] () { this->addToolBar(Qt::BottomToolBarArea); });
    connect(d->m_addStatusBar, &QAction::triggered, this, &QDesignerTaskMenu::createStatusBar);
    connect(d->m_removeStatusBar, &QAction::triggered, this, &QDesignerTaskMenu::removeStatusBar);
    connect(d->m_containerFakeMethods, &QAction::triggered, this, &QDesignerTaskMenu::containerFakeMethods);
    connect(d->m_navigateToSlot, &QAction::triggered, this, &QDesignerTaskMenu::slotNavigateToSlot);
    connect(d->m_sizeActionGroup, &QActionGroup::triggered, this, &QDesignerTaskMenu::applySize);
    d->m_layoutAlignmentMenu.connect(this, SLOT(slotLayoutAlignment()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
    delete d;
}

QAction *QDesignerTaskMenu::createSeparator()
{
    return createSeparatorHelper(this);
}

QWidget *QDesignerTaskMenu::widget() const
{
    return d->m_widget;
}

QDesignerFormWindowInterface *QDesignerTaskMenu::formWindow() const
{
    QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(result != nullptr);
    return result;
}

void QDesignerTaskMenu::createMenuBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateMenuBarCommand *cmd = new CreateMenuBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::addToolBar(Qt::ToolBarArea area)
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        AddToolBarCommand *cmd = new AddToolBarCommand(fw);
        cmd->init(mw, area);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::createStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateStatusBarCommand *cmd = new CreateStatusBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::removeStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
        cmd->init(mw->findChild<QStatusBar *>(QString(), Qt::FindDirectChildrenOnly));
        fw->commandHistory()->push(cmd);
    }
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow);

    const bool isMainContainer = formWindow->mainContainer() == widget();

    QList<QAction*> actions;

    if (const QMainWindow *mw = qobject_cast<const QMainWindow*>(formWindow->mainContainer()))  {
        if (isMainContainer || mw->centralWidget() == widget()) {
            if (mw->findChild<QMenuBar *>(QString(), Qt::FindDirectChildrenOnly) == nullptr)
                actions.append(d->m_addMenuBar);

            actions.append(d->m_addToolBar);
            actions.append(d->m_addAreaSubMenu);
            // ### create the status bar
            if (mw->findChild<QStatusBar *>(QString(), Qt::FindDirectChildrenOnly))
                actions.append(d->m_removeStatusBar);
            else
                actions.append(d->m_addStatusBar);

            actions.append(d->m_separator);
        }
    }
    actions.append(d->m_changeObjectNameAction);
    d->m_morphMenu->populate(d->m_widget, formWindow, actions);
    d->m_formLayoutMenu->populate(d->m_widget, formWindow, actions);
    actions.append(d->m_separator2);
    actions.append(d->m_changeToolTip);
    actions.append(d->m_changeWhatsThis);
    actions.append(d->m_changeStyleSheet);
    actions.append(d->m_separator6);
    actions.append(d->m_sizeActionsSubMenu);
    if (d->m_layoutAlignmentMenu.setAlignment(formWindow->core(), d->m_widget))
        actions.append(d->m_layoutAlignmentMenu.subMenuAction());

    d->m_promotionTaskMenu->setMode(formWindow->isManaged(d->m_widget) ?
                                    PromotionTaskMenu::ModeManagedMultiSelection : PromotionTaskMenu::ModeUnmanagedMultiSelection);
    d->m_promotionTaskMenu->addActions(formWindow, PromotionTaskMenu::LeadingSeparator, actions);

    if (isMainContainer && !qt_extension<QDesignerLanguageExtension*>(formWindow->core()->extensionManager(), formWindow->core())) {
        actions.append(d->m_separator5);
        actions.append(d->m_containerFakeMethods);
    }

    if (isSlotNavigationEnabled(formWindow->core())) {
        actions.append(d->m_separator7);
        actions.append(d->m_navigateToSlot);
    }

    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    QDesignerFormWindowInterface *fw = formWindow();
    Q_ASSERT(fw != nullptr);

    const QString oldObjectName = objName(fw->core(), widget());

    ObjectNameDialog dialog(fw, oldObjectName);
    if (dialog.exec() == QDialog::Accepted) {
        const QString newObjectName = dialog.newObjectName();
        if (!newObjectName.isEmpty() && newObjectName  != oldObjectName ) {
            const QString objectNameProperty = QStringLiteral("objectName");
            PropertySheetStringValue objectNameValue;
            objectNameValue.setValue(newObjectName);
            setProperty(fw, CurrentWidgetMode, objectNameProperty, QVariant::fromValue(objectNameValue));
        }
    }
}

void QDesignerTaskMenu::changeTextProperty(const QString &propertyName, const QString &windowTitle, PropertyMode pm, Qt::TextFormat desiredFormat)
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;
    Q_ASSERT(d->m_widget->parentWidget() != nullptr);

    const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(fw->core()->extensionManager(), d->m_widget);
    const int index = sheet->indexOf(propertyName);
    if (index == -1) {
        qDebug() << "** WARNING Invalid property" << propertyName << " passed to changeTextProperty!";
        return;
    }
    PropertySheetStringValue textValue = qvariant_cast<PropertySheetStringValue>(sheet->property(index));
    const QString oldText = textValue.value();
    // Pop up respective dialog
    bool accepted = false;
    QString newText;
    switch (desiredFormat) {
    case Qt::PlainText: {
        PlainTextEditorDialog dlg(fw->core(), fw);
        if (!windowTitle.isEmpty())
            dlg.setWindowTitle(windowTitle);
        dlg.setDefaultFont(d->m_widget->font());
        dlg.setText(oldText);
        accepted = dlg.showDialog() == QDialog::Accepted;
        newText = dlg.text();
    }
        break;
    default: {
        RichTextEditorDialog dlg(fw->core(), fw);
        if (!windowTitle.isEmpty())
            dlg.setWindowTitle(windowTitle);
        dlg.setDefaultFont(d->m_widget->font());
        dlg.setText(oldText);
        accepted = dlg.showDialog() == QDialog::Accepted;
        newText = dlg.text(desiredFormat);
    }
        break;
    }
    // change property
    if (!accepted || oldText == newText)
          return;


    textValue.setValue(newText);
    setProperty(fw, pm, propertyName, QVariant::fromValue(textValue));
}

void QDesignerTaskMenu::changeToolTip()
{
    changeTextProperty(QStringLiteral("toolTip"), tr("Edit ToolTip"), MultiSelectionMode, Qt::AutoText);
}

void QDesignerTaskMenu::changeWhatsThis()
{
    changeTextProperty(QStringLiteral("whatsThis"), tr("Edit WhatsThis"), MultiSelectionMode, Qt::AutoText);
}

void QDesignerTaskMenu::changeStyleSheet()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        StyleSheetPropertyEditorDialog dlg(fw, fw, d->m_widget);
        dlg.exec();
    }
}

void QDesignerTaskMenu::containerFakeMethods()
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;
    SignalSlotDialog::editMetaDataBase(fw, d->m_widget, fw);
}

bool QDesignerTaskMenu::isSlotNavigationEnabled(const QDesignerFormEditorInterface *core)
{
    return core->integration()->hasFeature(QDesignerIntegration::SlotNavigationFeature);
}

void QDesignerTaskMenu::slotNavigateToSlot()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    Q_ASSERT(core);
    navigateToSlot(core, widget());
}

void QDesignerTaskMenu::navigateToSlot(QDesignerFormEditorInterface *core,
                                       QObject *object,
                                       const QString &defaultSignal)
{
    SelectSignalDialog dialog;
    dialog.populate(core, object, defaultSignal);
    if (dialog.exec() != QDialog::Accepted)
        return;
    // TODO: Check whether signal is connected to slot
    const SelectSignalDialog::Method method = dialog.selectedMethod();
    if (method.isValid()) {
        core->integration()->emitNavigateToSlot(objName(core, object),
                                                method.signature,
                                                method.parameterNames);
    }
}

// Add a command that takes over the value of the current geometry as
// minimum/maximum size according to the flags.
static void createSizeCommand(QDesignerFormWindowInterface *fw, QWidget *w, int flags)
{
    const QSize size = w->size();
    if (flags & (ApplyMinimumWidth|ApplyMinimumHeight)) {
        QSize minimumSize = w-> minimumSize();
        if (flags & ApplyMinimumWidth)
            minimumSize.setWidth(size.width());
        if (flags & ApplyMinimumHeight)
             minimumSize.setHeight(size.height());
        SetPropertyCommand* cmd = new SetPropertyCommand(fw);
        cmd->init(w, QStringLiteral("minimumSize"), minimumSize);
        fw->commandHistory()->push(cmd);
    }
    if (flags & (ApplyMaximumWidth|ApplyMaximumHeight)) {
        QSize maximumSize = w-> maximumSize();
        if (flags & ApplyMaximumWidth)
            maximumSize.setWidth(size.width());
        if (flags & ApplyMaximumHeight)
             maximumSize.setHeight(size.height());
        SetPropertyCommand* cmd = new SetPropertyCommand(fw);
        cmd->init(w, QStringLiteral("maximumSize"), maximumSize);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::applySize(QAction *a)
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;

    const QWidgetList selection = applicableWidgets(fw, MultiSelectionMode);
    if (selection.isEmpty())
        return;

    const int mask = a->data().toInt();
    const int size = selection.size();
    fw->commandHistory()->beginMacro(tr("Set size constraint on %n widget(s)", nullptr, size));
    for (int i = 0; i < size; i++)
        createSizeCommand(fw, selection.at(i), mask);
    fw->commandHistory()->endMacro();
}

template <class Container>
    static void getApplicableObjects(const QDesignerFormWindowInterface *fw, QWidget *current,
                                     QDesignerTaskMenu::PropertyMode pm, Container *c)
{
    // Current is always first
    c->push_back(current);
    if (pm == QDesignerTaskMenu::CurrentWidgetMode)
        return;
    QDesignerObjectInspector *designerObjectInspector = qobject_cast<QDesignerObjectInspector *>(fw->core()->objectInspector());
    if (!designerObjectInspector)
        return; // Ooops, someone plugged an old-style Object Inspector
    // Add managed or unmanaged selection according to current type, make current first
    Selection s;
    designerObjectInspector->getSelection(s);
    const QWidgetList &source = fw->isManaged(current) ? s.managed : s.unmanaged;
    const QWidgetList::const_iterator cend = source.constEnd();
    for ( QWidgetList::const_iterator it = source.constBegin(); it != cend; ++it)
        if (*it != current) // was first
            c->push_back(*it);
}

QObjectList QDesignerTaskMenu::applicableObjects(const QDesignerFormWindowInterface *fw, PropertyMode pm) const
{
    QObjectList rc;
    getApplicableObjects(fw, d->m_widget, pm, &rc);
    return rc;
}

QWidgetList QDesignerTaskMenu::applicableWidgets(const QDesignerFormWindowInterface *fw, PropertyMode pm) const
{
    QWidgetList rc;
    getApplicableObjects(fw, d->m_widget, pm, &rc);
    return rc;
}

void QDesignerTaskMenu::setProperty(QDesignerFormWindowInterface *fw,  PropertyMode pm, const QString &name, const QVariant &newValue)
{
    SetPropertyCommand* setPropertyCommand = new SetPropertyCommand(fw);
    if (setPropertyCommand->init(applicableObjects(fw, pm), name, newValue, d->m_widget)) {
        fw->commandHistory()->push(setPropertyCommand);
    } else {
        delete setPropertyCommand;
        qDebug() << "Unable to set property " << name << '.';
    }
}

void QDesignerTaskMenu::slotLayoutAlignment()
{
    QDesignerFormWindowInterface *fw = formWindow();
    const Qt::Alignment newAlignment = d->m_layoutAlignmentMenu.alignment();
    LayoutAlignmentCommand *cmd = new LayoutAlignmentCommand(fw);
    if (cmd->init(d->m_widget, newAlignment)) {
        fw->commandHistory()->push(cmd);
    } else {
        delete cmd;
    }
}
} // namespace qdesigner_internal

QT_END_NAMESPACE
