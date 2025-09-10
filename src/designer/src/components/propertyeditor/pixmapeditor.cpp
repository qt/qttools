// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "pixmapeditor.h"
#include <iconloader_p.h>
#include <iconselector_p.h>
#include <qdesigner_utils_p.h>

#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmenu.h>

#include <QtGui/qaction.h>
#if QT_CONFIG(clipboard)
#include <QtGui/qclipboard.h>
#endif
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr QSize ICON_SIZE{16, 16};

namespace qdesigner_internal {

static void createIconThemeDialog(QDialog *topLevel, const QString &labelText,
                                  QWidget *themeEditor)
{
    QVBoxLayout *layout = new QVBoxLayout(topLevel);
    QLabel *label = new QLabel(labelText, topLevel);
    QDialogButtonBox *buttons = new QDialogButtonBox(topLevel);
    buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttons, &QDialogButtonBox::accepted, topLevel, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, topLevel, &QDialog::reject);

    layout->addWidget(label);
    layout->addWidget(themeEditor);
    layout->addWidget(buttons);
}

IconThemeDialog::IconThemeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Set Icon From XDG Theme"));
    m_editor = new IconThemeEditor(this);
    createIconThemeDialog(this, tr("Select icon name from XDG theme:"), m_editor);
}

std::optional<QString> IconThemeDialog::getTheme(QWidget *parent, const QString &theme)
{
    IconThemeDialog dlg(parent);
    dlg.m_editor->setTheme(theme);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.m_editor->theme();
    return std::nullopt;
}

IconThemeEnumDialog::IconThemeEnumDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Set Icon From Theme"));
    m_editor = new IconThemeEnumEditor(this);
    createIconThemeDialog(this, tr("Select icon name from theme:"), m_editor);
}

std::optional<int> IconThemeEnumDialog::getTheme(QWidget *parent, int theme)
{
    IconThemeEnumDialog dlg(parent);
    dlg.m_editor->setThemeEnum(theme);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.m_editor->themeEnum();
    return std::nullopt;
}

PixmapEditor::PixmapEditor(QDesignerFormEditorInterface *core, QWidget *parent) :
    QWidget(parent),
    m_iconThemeModeEnabled(false),
    m_core(core),
    m_pixmapLabel(new QLabel(this)),
    m_pathLabel(new QLabel(this)),
    m_button(new QToolButton(this)),
    m_resourceAction(new QAction(tr("Choose Resource..."), this)),
    m_fileAction(new QAction(tr("Choose File..."), this)),
    m_themeEnumAction(new QAction(tr("Set Icon From Theme..."), this)),
    m_themeAction(new QAction(tr("Set Icon From XDG Theme..."), this)),
    m_copyAction(new QAction(createIconSet(QIcon::ThemeIcon::EditCopy, "editcopy.png"_L1),
                             tr("Copy Path"), this)),
    m_pasteAction(new QAction(createIconSet(QIcon::ThemeIcon::EditPaste, "editpaste.png"_L1),
                              tr("Paste Path"), this)),
    m_layout(new QHBoxLayout(this)),
    m_pixmapCache(nullptr)
{
    m_layout->addWidget(m_pixmapLabel);
    m_layout->addWidget(m_pathLabel);
    m_button->setText(tr("..."));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(30);
    m_button->setPopupMode(QToolButton::MenuButtonPopup);
    m_layout->addWidget(m_button);
    m_layout->setContentsMargins(QMargins());
    m_layout->setSpacing(0);
    m_pixmapLabel->setFixedWidth(ICON_SIZE.width());
    m_pixmapLabel->setAlignment(Qt::AlignCenter);
    m_pathLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    m_themeAction->setVisible(false);
    m_themeEnumAction->setVisible(false);

    QMenu *menu = new QMenu(this);
    menu->addAction(m_resourceAction);
    menu->addAction(m_fileAction);
    menu->addAction(m_themeEnumAction);
    menu->addAction(m_themeAction);

    m_button->setMenu(menu);
    m_button->setText(tr("..."));

    connect(m_button, &QAbstractButton::clicked, this, &PixmapEditor::defaultActionActivated);
    connect(m_resourceAction, &QAction::triggered, this, &PixmapEditor::resourceActionActivated);
    connect(m_fileAction, &QAction::triggered, this, &PixmapEditor::fileActionActivated);
    connect(m_themeEnumAction, &QAction::triggered, this, &PixmapEditor::themeEnumActionActivated);
    connect(m_themeAction, &QAction::triggered, this, &PixmapEditor::themeActionActivated);
#if QT_CONFIG(clipboard)
    connect(m_copyAction, &QAction::triggered, this, &PixmapEditor::copyActionActivated);
    connect(m_pasteAction, &QAction::triggered, this, &PixmapEditor::pasteActionActivated);
#endif
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));
    setFocusProxy(m_button);

#if QT_CONFIG(clipboard)
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &PixmapEditor::clipboardDataChanged);
    clipboardDataChanged();
#endif
}

void PixmapEditor::setPixmapCache(DesignerPixmapCache *cache)
{
    m_pixmapCache = cache;
}

void PixmapEditor::setIconThemeModeEnabled(bool enabled)
{
    if (m_iconThemeModeEnabled == enabled)
        return;
    m_iconThemeModeEnabled = enabled;
    m_themeAction->setVisible(enabled);
    m_themeEnumAction->setVisible(enabled);
}

void PixmapEditor::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

void PixmapEditor::setPath(const QString &path)
{
    m_path = path;
    updateLabels();
}

void PixmapEditor::setTheme(const QString &theme)
{
    m_theme = theme;
    updateLabels();
}

QString PixmapEditor::msgThemeIcon(const QString &t)
{
    return tr("[Theme] %1").arg(t);
}

QString PixmapEditor::msgMissingThemeIcon(const QString &t)
{
    return tr("[Theme] %1 (missing)").arg(t);
}

void PixmapEditor::setThemeEnum(int e)
{
    m_themeEnum = e;
    updateLabels();
}

void PixmapEditor::updateLabels()
{
    m_pathLabel->setText(displayText(m_themeEnum, m_theme, m_path));
    switch (state()) {
    case State::Empty:
    case State::MissingXdgTheme:
    case State::MissingThemeEnum:
        m_pixmapLabel->setPixmap(m_defaultPixmap);
        m_copyAction->setEnabled(false);
        break;
    case State::ThemeEnum:
        m_pixmapLabel->setPixmap(QIcon::fromTheme(static_cast<QIcon::ThemeIcon>(m_themeEnum)).pixmap(ICON_SIZE));
        m_copyAction->setEnabled(true);
        break;
    case State::XdgTheme:
        m_pixmapLabel->setPixmap(QIcon::fromTheme(m_theme).pixmap(ICON_SIZE));
        m_copyAction->setEnabled(true);
        break;
    case State::Path:
    case State::PathFallback:
        if (m_pixmapCache) {
            auto pixmap = m_pixmapCache->pixmap(PropertySheetPixmapValue(m_path));
            m_pixmapLabel->setPixmap(QIcon(pixmap).pixmap(ICON_SIZE));
        }
        m_copyAction->setEnabled(true);
        break;
    }
}

void PixmapEditor::setDefaultPixmapIcon(const QIcon &icon)
{
    m_defaultPixmap = icon.pixmap(ICON_SIZE);
    if (state() == State::Empty)
        m_pixmapLabel->setPixmap(m_defaultPixmap);
}

void PixmapEditor::setDefaultPixmap(const QPixmap &pixmap)
{
    setDefaultPixmapIcon(QIcon(pixmap));
}

void PixmapEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_copyAction);
    menu.addAction(m_pasteAction);
    menu.exec(event->globalPos());
    event->accept();
}

void PixmapEditor::defaultActionActivated()
{
    if (m_iconThemeModeEnabled) {
        themeEnumActionActivated();
        return;
    }
    // Default to resource
    const PropertySheetPixmapValue::PixmapSource ps = m_path.isEmpty()
            ? PropertySheetPixmapValue::ResourcePixmap
            : PropertySheetPixmapValue::getPixmapSource(m_core, m_path);
    switch (ps) {
    case PropertySheetPixmapValue::LanguageResourcePixmap:
    case PropertySheetPixmapValue::ResourcePixmap:
        resourceActionActivated();
        break;
    case PropertySheetPixmapValue::FilePixmap:
        fileActionActivated();
        break;
    }
}

void PixmapEditor::resourceActionActivated()
{
    const QString oldPath = m_path;
    const  QString newPath = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(),
                                                                oldPath, this);
    if (!newPath.isEmpty() && newPath != oldPath) {
        setTheme({});
        setThemeEnum(-1);
        setPath(newPath);
        emit pathChanged(newPath);
    }
}

void PixmapEditor::fileActionActivated()
{
    const QString newPath = IconSelector::choosePixmapFile(m_path, m_core->dialogGui(), this);
    if (!newPath.isEmpty() && newPath != m_path) {
        setTheme({});
        setThemeEnum(-1);
        setPath(newPath);
        emit pathChanged(newPath);
    }
}

void PixmapEditor::themeEnumActionActivated()
{
    const auto newThemeO = IconThemeEnumDialog::getTheme(this, {});
    if (newThemeO.has_value()) {
        const int newTheme = newThemeO.value();
        if (newTheme != m_themeEnum) {
            setThemeEnum(newTheme);
            setTheme({});
            setPath({});
            emit themeEnumChanged(newTheme);
        }
    }
}

void PixmapEditor::themeActionActivated()
{
    const auto newThemeO = IconThemeDialog::getTheme(this, m_theme);
    if (newThemeO.has_value()) {
        const QString newTheme = newThemeO.value();
        if (newTheme != m_theme) {
            setTheme(newTheme);
            setThemeEnum(-1);
            setPath({});
            emit themeChanged(newTheme);
        }
    }
}

PixmapEditor::State PixmapEditor::stateFromData(int themeEnum, const QString &xdgTheme,
                                                const QString &path)
{
    if (themeEnum != -1) {
        if (QIcon::hasThemeIcon(static_cast<QIcon::ThemeIcon>(themeEnum)))
            return State::ThemeEnum;
        return path.isEmpty() ? State::MissingThemeEnum : State::PathFallback;
    }
    if (!xdgTheme.isEmpty()) {
        if (QIcon::hasThemeIcon(xdgTheme))
            return State::XdgTheme;
        return path.isEmpty() ? State::MissingXdgTheme : State::PathFallback;
    }
    return path.isEmpty() ? State::Empty : State::Path;
}

PixmapEditor::State PixmapEditor::state() const
{
    return stateFromData(m_themeEnum, m_theme, m_path);
}

QString PixmapEditor::displayText(int themeEnum, const QString &xdgTheme, const QString &path)
{
    switch (stateFromData(themeEnum, xdgTheme, path)) {
    case State::ThemeEnum:
        return msgThemeIcon(IconThemeEnumEditor::iconName(themeEnum));
    case State::MissingThemeEnum:
        return msgMissingThemeIcon(IconThemeEnumEditor::iconName(themeEnum));
    case State::XdgTheme:
        return msgThemeIcon(xdgTheme);
    case State::MissingXdgTheme:
        return msgMissingThemeIcon(xdgTheme);
    case State::Path:
        return QFileInfo(path).fileName();
    case State::PathFallback:
        return tr("%1 (fallback)").arg(QFileInfo(path).fileName());
    case State::Empty:
        break;
    }
    return {};
}

QString PixmapEditor::displayText(const PropertySheetIconValue &icon)
{
    const auto &paths = icon.paths();
    const auto &it = paths.constFind({QIcon::Normal, QIcon::Off});
    const QString path = it != paths.constEnd() ? it.value().path() : QString{};
    return displayText(icon.themeEnum(), icon.theme(), path);
}

#if QT_CONFIG(clipboard)
void PixmapEditor::copyActionActivated()
{
    QClipboard *clipboard = QApplication::clipboard();
    switch (state()) {
    case State::ThemeEnum:
    case State::MissingThemeEnum:
        clipboard->setText(IconThemeEnumEditor::iconName(m_themeEnum));
        break;
    case State::XdgTheme:
    case State::MissingXdgTheme:
        clipboard->setText(m_theme);
        break;
    case State::Path:
    case State::PathFallback:
        clipboard->setText(m_path);
        break;
    case State::Empty:
        break;
    }
}

void PixmapEditor::pasteActionActivated()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString subtype = u"plain"_s;
    QString text = clipboard->text(subtype);
    if (!text.isNull()) {
        QStringList list = text.split(u'\n');
        if (!list.isEmpty()) {
            text = list.at(0);
            if (m_iconThemeModeEnabled && QIcon::hasThemeIcon(text)) {
                setTheme(text);
                setPath(QString());
                emit themeChanged(text);
            } else {
                setPath(text);
                setTheme(QString());
                emit pathChanged(text);
            }
        }
    }
}

void PixmapEditor::clipboardDataChanged()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString subtype = u"plain"_s;
    const QString text = clipboard->text(subtype);
    m_pasteAction->setEnabled(!text.isNull());
}
#endif // QT_CONFIG(clipboard)

} // qdesigner_internal

QT_END_NAMESPACE
