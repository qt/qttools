// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "iconselector_p.h"
#include "qdesigner_utils_p.h"
#include "qtresourcemodel_p.h"
#include "qtresourceview_p.h"
#include "iconloader_p.h"
#include "formwindowbase_p.h"

#include <abstractdialoggui_p.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractresourcebrowser.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtGui/qimagereader.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlabel.h>

#include <QtGui/qaction.h>
#include <QtGui/qvalidator.h>

#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

// Validator for theme line edit, accepts empty or non-blank strings.
class BlankSuppressingValidator : public QValidator {
public:
    explicit BlankSuppressingValidator(QObject * parent = nullptr) : QValidator(parent) {}
    State validate(QString &input, int &pos) const override
    {
        const auto blankPos = input.indexOf(u' ');
        if (blankPos != -1) {
            pos = blankPos;
            return Invalid;
        }
        return Acceptable;
    }
};

// -------------------- LanguageResourceDialogPrivate
class LanguageResourceDialogPrivate {
    LanguageResourceDialog *q_ptr;
    Q_DECLARE_PUBLIC(LanguageResourceDialog)

public:
    LanguageResourceDialogPrivate(QDesignerResourceBrowserInterface *rb);
    void init(LanguageResourceDialog *p);

    void setCurrentPath(const QString &filePath);
    QString currentPath() const;

    void slotAccepted();
    void slotPathChanged(const QString &);

private:
    void setOkButtonEnabled(bool v)         { m_dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(v); }
    static bool checkPath(const QString &p);

    QDesignerResourceBrowserInterface *m_browser;
    QDialogButtonBox *m_dialogButtonBox;
};

LanguageResourceDialogPrivate::LanguageResourceDialogPrivate(QDesignerResourceBrowserInterface *rb) :
    q_ptr(nullptr),
    m_browser(rb),
    m_dialogButtonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel))
{
     setOkButtonEnabled(false);
}

void LanguageResourceDialogPrivate::init(LanguageResourceDialog *p)
{
    q_ptr = p;
    QLayout *layout = new QVBoxLayout(p);
    layout->addWidget(m_browser);
    layout->addWidget(m_dialogButtonBox);
    QObject::connect(m_dialogButtonBox, &QDialogButtonBox::accepted, p, [this] { slotAccepted(); });
    QObject::connect(m_dialogButtonBox, &QDialogButtonBox::rejected, p, &QDialog::reject);
    QObject::connect(m_browser, &QDesignerResourceBrowserInterface::currentPathChanged,
                     p, [this](const QString &fileName) { slotPathChanged(fileName); });
    QObject::connect(m_browser, &QDesignerResourceBrowserInterface::pathActivated,
                     p, [this] { slotAccepted(); });
    p->setModal(true);
    p->setWindowTitle(LanguageResourceDialog::tr("Choose Resource"));
    setOkButtonEnabled(false);
}

void LanguageResourceDialogPrivate::setCurrentPath(const QString &filePath)
{
    m_browser->setCurrentPath(filePath);
    setOkButtonEnabled(checkPath(filePath));
}

QString LanguageResourceDialogPrivate::currentPath() const
{
    return m_browser->currentPath();
}

bool LanguageResourceDialogPrivate::checkPath(const QString &p)
{
    return p.isEmpty() ? false : IconSelector::checkPixmap(p, IconSelector::CheckFast);
}

void LanguageResourceDialogPrivate::slotAccepted()
{
    if (checkPath(currentPath()))
        q_ptr->accept();
}

void LanguageResourceDialogPrivate::slotPathChanged(const QString &p)
{
    setOkButtonEnabled(checkPath(p));
}

// ------------ LanguageResourceDialog
LanguageResourceDialog::LanguageResourceDialog(QDesignerResourceBrowserInterface *rb, QWidget *parent) :
    QDialog(parent),
    d_ptr(new LanguageResourceDialogPrivate(rb))
{
    d_ptr->init( this);
}

LanguageResourceDialog::~LanguageResourceDialog() = default;

void LanguageResourceDialog::setCurrentPath(const QString &filePath)
{
    d_ptr->setCurrentPath(filePath);
}

QString LanguageResourceDialog::currentPath() const
{
    return d_ptr->currentPath();
}

LanguageResourceDialog* LanguageResourceDialog::create(QDesignerFormEditorInterface *core, QWidget *parent)
{
    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core))
        if (QDesignerResourceBrowserInterface *rb = lang->createResourceBrowser(nullptr))
            return new LanguageResourceDialog(rb, parent);
    if (QDesignerResourceBrowserInterface *rb = core->integration()->createResourceBrowser(nullptr))
        return new LanguageResourceDialog(rb, parent);
    return nullptr;
}

// ------------ IconSelectorPrivate

class IconSelectorPrivate
{
    IconSelector *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(IconSelector)
public:
    IconSelectorPrivate() = default;

    void slotStateActivated();
    void slotSetActivated();
    void slotSetResourceActivated();
    void slotSetFileActivated();
    void slotResetActivated();
    void slotResetAllActivated();
    void slotUpdate();

    QList<QPair<QPair<QIcon::Mode, QIcon::State>, QString> > m_stateToName; // could be static map

    QMap<QPair<QIcon::Mode, QIcon::State>, int>  m_stateToIndex;
    QMap<int, QPair<QIcon::Mode, QIcon::State> > m_indexToState;

    const QIcon m_emptyIcon;
    QComboBox *m_stateComboBox = nullptr;
    QToolButton *m_iconButton = nullptr;
    QAction *m_resetAction = nullptr;
    QAction *m_resetAllAction = nullptr;
    PropertySheetIconValue m_icon;
    DesignerIconCache *m_iconCache = nullptr;
    DesignerPixmapCache *m_pixmapCache = nullptr;
    QtResourceModel *m_resourceModel = nullptr;
    QDesignerFormEditorInterface *m_core = nullptr;
};

void IconSelectorPrivate::slotUpdate()
{
    QIcon icon;
    if (m_iconCache)
        icon = m_iconCache->icon(m_icon);

    QMap<QPair<QIcon::Mode, QIcon::State>, PropertySheetPixmapValue> paths = m_icon.paths();
    for (auto itIndex = m_stateToIndex.cbegin(), end = m_stateToIndex.cend(); itIndex != end; ++itIndex) {
        const QPair<QIcon::Mode, QIcon::State> state = itIndex.key();
        const PropertySheetPixmapValue pixmap = paths.value(state);
        const int index = itIndex.value();

        QIcon pixmapIcon = QIcon(icon.pixmap(16, 16, state.first, state.second));
        if (pixmapIcon.isNull())
            pixmapIcon = m_emptyIcon;
        m_stateComboBox->setItemIcon(index, pixmapIcon);
        QFont font = q_ptr->font();
        if (!pixmap.path().isEmpty())
            font.setBold(true);
        m_stateComboBox->setItemData(index, font, Qt::FontRole);
    }

    QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());
    PropertySheetPixmapValue currentPixmap = paths.value(state);
    m_resetAction->setEnabled(!currentPixmap.path().isEmpty());
    m_resetAllAction->setEnabled(!paths.isEmpty());
    m_stateComboBox->update();
}

void IconSelectorPrivate::slotStateActivated()
{
    slotUpdate();
}

void IconSelectorPrivate::slotSetActivated()
{
    QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());
    const PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
    // Default to resource
    const PropertySheetPixmapValue::PixmapSource ps = pixmap.path().isEmpty() ? PropertySheetPixmapValue::ResourcePixmap : pixmap.pixmapSource(m_core);
    switch (ps) {
    case PropertySheetPixmapValue::LanguageResourcePixmap:
    case PropertySheetPixmapValue::ResourcePixmap:
        slotSetResourceActivated();
        break;
    case PropertySheetPixmapValue::FilePixmap:
        slotSetFileActivated();
        break;
    }
}

// Choose a pixmap from resource; use language-dependent resource browser if present
QString IconSelector::choosePixmapResource(QDesignerFormEditorInterface *core, QtResourceModel *resourceModel, const QString &oldPath, QWidget *parent)
{
    Q_UNUSED(resourceModel);
    QString rc;

    if (LanguageResourceDialog* ldlg = LanguageResourceDialog::create(core, parent)) {
        ldlg->setCurrentPath(oldPath);
        if (ldlg->exec() == QDialog::Accepted)
            rc = ldlg->currentPath();
        delete ldlg;
    } else {
        QtResourceViewDialog dlg(core, parent);
        dlg.setResourceEditingEnabled(core->integration()->hasFeature(QDesignerIntegration::ResourceEditorFeature));

        dlg.selectResource(oldPath);
        if (dlg.exec() == QDialog::Accepted)
            rc = dlg.selectedResource();
    }
    return rc;
}

void IconSelectorPrivate::slotSetResourceActivated()
{
    const QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

    PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
    const QString oldPath = pixmap.path();
    const QString newPath = IconSelector::choosePixmapResource(m_core, m_resourceModel, oldPath, q_ptr);
    if (newPath.isEmpty() || newPath == oldPath)
        return;
    const PropertySheetPixmapValue newPixmap = PropertySheetPixmapValue(newPath);
    if (newPixmap != pixmap) {
        m_icon.setPixmap(state.first, state.second, newPixmap);
        slotUpdate();
        emit q_ptr->iconChanged(m_icon);
    }
}

// Helpers for choosing image files: Check for valid image.
bool IconSelector::checkPixmap(const QString &fileName, CheckMode cm, QString *errorMessage)
{
    const QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
        if (errorMessage)
            *errorMessage = tr("The pixmap file '%1' cannot be read.").arg(fileName);
        return false;
    }
    QImageReader reader(fileName);
    if (!reader.canRead()) {
        if (errorMessage)
            *errorMessage = tr("The file '%1' does not appear to be a valid pixmap file: %2")
                              .arg(fileName, reader.errorString());
        return false;
    }
    if (cm == CheckFast)
        return true;

    const QImage image = reader.read();
    if (image.isNull()) {
        if (errorMessage)
            *errorMessage = tr("The file '%1' could not be read: %2")
                               .arg(fileName, reader.errorString());
        return false;
    }
    return true;
}

// Helpers for choosing image files: Return an image filter for QFileDialog, courtesy of StyledButton
static QString imageFilter()
{
    QString filter = QApplication::translate("IconSelector", "All Pixmaps (");
    const auto supportedImageFormats = QImageReader::supportedImageFormats();
    const qsizetype count = supportedImageFormats.size();
    for (qsizetype i = 0; i < count; ++i) {
        if (i)
            filter += u' ';
        filter += "*."_L1;
        const QString outputFormat = QString::fromUtf8(supportedImageFormats.at(i));
        if (outputFormat != "JPEG"_L1)
            filter += outputFormat.toLower();
        else
            filter += "jpg *.jpeg"_L1;
    }
    filter += u')';
    return filter;
}

// Helpers for choosing image files: Choose a file
QString IconSelector::choosePixmapFile(const QString &directory, QDesignerDialogGuiInterface *dlgGui,QWidget *parent)
{
    QString errorMessage;
    QString newPath;
    do {
        const QString title = tr("Choose a Pixmap");
        static const  QString filter = imageFilter();
        newPath =  dlgGui->getOpenImageFileName(parent, title, directory, filter);
        if (newPath.isEmpty())
            break;
        if (checkPixmap(newPath, CheckFully, &errorMessage))
            break;
        dlgGui->message(parent, QDesignerDialogGuiInterface::ResourceEditorMessage, QMessageBox::Warning, tr("Pixmap Read Error"), errorMessage);
    } while(true);
    return  newPath;
}

void IconSelectorPrivate::slotSetFileActivated()
{
    QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

    PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
    const QString newPath = IconSelector::choosePixmapFile(pixmap.path(), m_core->dialogGui(), q_ptr);
    if (!newPath.isEmpty()) {
        const PropertySheetPixmapValue newPixmap = PropertySheetPixmapValue(newPath);
        if (!(newPixmap == pixmap)) {
            m_icon.setPixmap(state.first, state.second, newPixmap);
            slotUpdate();
            emit q_ptr->iconChanged(m_icon);
        }
    }
}

void IconSelectorPrivate::slotResetActivated()
{
    QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

    PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
    const PropertySheetPixmapValue newPixmap;
    if (!(newPixmap == pixmap)) {
        m_icon.setPixmap(state.first, state.second, newPixmap);
        slotUpdate();
        emit q_ptr->iconChanged(m_icon);
    }
}

void IconSelectorPrivate::slotResetAllActivated()
{
    const PropertySheetIconValue newIcon;
    if (!(m_icon == newIcon)) {
        m_icon = newIcon;
        slotUpdate();
        emit q_ptr->iconChanged(m_icon);
    }
}

// ------------- IconSelector
IconSelector::IconSelector(QWidget *parent) :
    QWidget(parent), d_ptr(new IconSelectorPrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_stateComboBox = new QComboBox(this);

    QHBoxLayout *l = new QHBoxLayout(this);
    d_ptr->m_iconButton = new QToolButton(this);
    d_ptr->m_iconButton->setText(tr("..."));
    d_ptr->m_iconButton->setPopupMode(QToolButton::MenuButtonPopup);
    l->addWidget(d_ptr->m_stateComboBox);
    l->addWidget(d_ptr->m_iconButton);
    l->setContentsMargins(QMargins());

    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Normal,   QIcon::Off), tr("Normal Off")   );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Normal,   QIcon::On),  tr("Normal On")    );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Disabled, QIcon::Off), tr("Disabled Off") );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Disabled, QIcon::On),  tr("Disabled On")  );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Active,   QIcon::Off), tr("Active Off")   );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Active,   QIcon::On),  tr("Active On")    );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Selected, QIcon::Off), tr("Selected Off") );
    d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Selected, QIcon::On),  tr("Selected On")  );

    QMenu *setMenu = new QMenu(this);

    QAction *setResourceAction = new QAction(tr("Choose Resource..."), this);
    QAction *setFileAction = new QAction(tr("Choose File..."), this);
    d_ptr->m_resetAction = new QAction(tr("Reset"), this);
    d_ptr->m_resetAllAction = new QAction(tr("Reset All"), this);
    d_ptr->m_resetAction->setEnabled(false);
    d_ptr->m_resetAllAction->setEnabled(false);
    //d_ptr->m_resetAction->setIcon(createIconSet(u"resetproperty.png"_s));

    setMenu->addAction(setResourceAction);
    setMenu->addAction(setFileAction);
    setMenu->addSeparator();
    setMenu->addAction(d_ptr->m_resetAction);
    setMenu->addAction(d_ptr->m_resetAllAction);

    int index = 0;
    QStringList items;
    for (const auto &item : std::as_const(d_ptr->m_stateToName)) {
        const QPair<QIcon::Mode, QIcon::State> state = item.first;
        const QString name = item.second;

        items.append(name);
        d_ptr->m_stateToIndex[state] = index;
        d_ptr->m_indexToState[index] = state;
        index++;
    }
    d_ptr->m_stateComboBox->addItems(items);

    d_ptr->m_iconButton->setMenu(setMenu);

    connect(d_ptr->m_stateComboBox, &QComboBox::activated,
            this, [this] { d_ptr->slotStateActivated(); });
    connect(d_ptr->m_iconButton, &QAbstractButton::clicked,
            this, [this] { d_ptr->slotSetActivated(); });
    connect(setResourceAction, &QAction::triggered,
            this, [this] { d_ptr->slotSetResourceActivated(); });
    connect(setFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotSetFileActivated(); });
    connect(d_ptr->m_resetAction, &QAction::triggered,
            this, [this] { d_ptr->slotResetActivated(); });
    connect(d_ptr->m_resetAllAction, &QAction::triggered,
            this, [this] { d_ptr->slotResetAllActivated(); });
    d_ptr->slotUpdate();
}

IconSelector::~IconSelector() = default;

void IconSelector::setIcon(const PropertySheetIconValue &icon)
{
    if (d_ptr->m_icon == icon)
        return;

    d_ptr->m_icon = icon;
    d_ptr->slotUpdate();
}

PropertySheetIconValue IconSelector::icon() const
{
    return d_ptr->m_icon;
}

void IconSelector::setFormEditor(QDesignerFormEditorInterface *core)
{
    d_ptr->m_core = core;
    d_ptr->m_resourceModel = core->resourceModel();
    d_ptr->slotUpdate();
}

void IconSelector::setIconCache(DesignerIconCache *iconCache)
{
    d_ptr->m_iconCache = iconCache;
    connect(iconCache, &DesignerIconCache::reloaded, this, [this] { d_ptr->slotUpdate(); });
    d_ptr->slotUpdate();
}

void IconSelector::setPixmapCache(DesignerPixmapCache *pixmapCache)
{
    d_ptr->m_pixmapCache = pixmapCache;
    connect(pixmapCache, &DesignerPixmapCache::reloaded, this, [this] { d_ptr->slotUpdate(); });
    d_ptr->slotUpdate();
}

// --- IconThemeEditor

static const QMap<QString, QIcon> &themeIcons()
{
   static QMap<QString, QIcon> result;
   if (result.isEmpty()) {
       QFile file(u":/qt-project.org/designer/icon-naming-spec.txt"_s);
       if (file.open(QIODevice::ReadOnly)) {
           while (!file.atEnd()) {
               const auto line = file.readLine().trimmed();
               if (line.isEmpty() || line.startsWith('#'))
                   continue;
               const auto iconName = QString::fromUtf8(line);
               result.insert(iconName, QIcon::fromTheme(iconName));
           }
           file.close();
       }
   }
   return result;
}

struct IconThemeEditorPrivate {
    IconThemeEditorPrivate();

    QComboBox *m_themeComboBox;
};

IconThemeEditorPrivate::IconThemeEditorPrivate() :
    m_themeComboBox(new QComboBox)
{
}

IconThemeEditor::IconThemeEditor(QWidget *parent, bool wantResetButton) :
    QWidget (parent), d(new IconThemeEditorPrivate)
{
    QHBoxLayout *mainHLayout = new QHBoxLayout;
    mainHLayout->setContentsMargins(QMargins());

    const auto icons = themeIcons();
    for (auto i = icons.constBegin(); i != icons.constEnd(); ++i)
        d->m_themeComboBox->addItem(i.value(), i.key());
    d->m_themeComboBox->setCurrentIndex(-1);
    d->m_themeComboBox->setEditable(true);
    d->m_themeComboBox->lineEdit()->setValidator(new BlankSuppressingValidator(this));
    connect(d->m_themeComboBox, &QComboBox::currentTextChanged, this, &IconThemeEditor::edited);
    mainHLayout->addWidget(d->m_themeComboBox);

    if (wantResetButton) {
        QToolButton *themeResetButton = new QToolButton;
        themeResetButton->setIcon(createIconSet(u"resetproperty.png"_s));
        connect(themeResetButton, &QAbstractButton::clicked, this, &IconThemeEditor::reset);
        mainHLayout->addWidget(themeResetButton);
    }

    setLayout(mainHLayout);
    setFocusProxy(d->m_themeComboBox);
}

IconThemeEditor::~IconThemeEditor() = default;

void IconThemeEditor::reset()
{
    d->m_themeComboBox->setCurrentIndex(-1);
    emit edited(QString());
}

QString IconThemeEditor::theme() const
{
    return d->m_themeComboBox->currentText();
}

void IconThemeEditor::setTheme(const QString &t)
{
    d->m_themeComboBox->setCurrentText(t);
}

} // qdesigner_internal

QT_END_NAMESPACE

#include "moc_iconselector_p.cpp"
