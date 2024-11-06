// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "paletteeditor.h"

#include <qdesigner_utils_p.h>
#include <iconloader_p.h>
#include <qtcolorbutton_p.h>

#include <private/formbuilderextra_p.h>
#include <private/ui4_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qstyle.h>

#include <QtGui/qaction.h>
#if QT_CONFIG(clipboard)
#  include <QtGui/qclipboard.h>
#endif
#include <QtGui/qguiapplication.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscreen.h>

#include <QtCore/qfile.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

enum { BrushRole = 33 };

PaletteEditor::PaletteEditor(QDesignerFormEditorInterface *core, QWidget *parent) :
    QDialog(parent),
    m_paletteModel(new PaletteModel(this)),
    m_core(core)
{
    ui.setupUi(this);
    auto saveButton = ui.buttonBox->addButton(tr("Save..."), QDialogButtonBox::ActionRole);
    connect(saveButton, &QPushButton::clicked, this, &PaletteEditor::save);
    auto loadButton = ui.buttonBox->addButton(tr("Load..."), QDialogButtonBox::ActionRole);
    connect(loadButton, &QPushButton::clicked, this, &PaletteEditor::load);

    connect(ui.buildButton, &QtColorButton::colorChanged,
            this, &PaletteEditor::buildButtonColorChanged);
    connect(ui.activeRadio, &QAbstractButton::clicked,
            this, &PaletteEditor::activeRadioClicked);
    connect(ui.inactiveRadio,  &QAbstractButton::clicked,
            this, &PaletteEditor::inactiveRadioClicked);
    connect(ui.disabledRadio, &QAbstractButton::clicked,
            this, &PaletteEditor::disabledRadioClicked);
    connect(ui.computeRadio, &QAbstractButton::clicked,
            this, &PaletteEditor::computeRadioClicked);
    connect(ui.detailsRadio, &QAbstractButton::clicked,
            this, &PaletteEditor::detailsRadioClicked);

    ui.paletteView->setModel(m_paletteModel);
    ui.previewGroupBox->setTitle(tr("Preview (%1)").arg(style()->objectName()));
    updatePreviewPalette();
    updateStyledButton();
    ui.paletteView->setModel(m_paletteModel);
    ColorDelegate *delegate = new ColorDelegate(core, this);
    ui.paletteView->setItemDelegate(delegate);
    ui.paletteView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    connect(m_paletteModel, &PaletteModel::paletteChanged,
                this, &PaletteEditor::paletteChanged);
    ui.paletteView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.paletteView->setDragEnabled(true);
    ui.paletteView->setDropIndicatorShown(true);
    ui.paletteView->setRootIsDecorated(false);
    ui.paletteView->setColumnHidden(2, true);
    ui.paletteView->setColumnHidden(3, true);
    ui.paletteView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.paletteView, &QWidget::customContextMenuRequested,
            this, &PaletteEditor::viewContextMenuRequested);

    const auto itemRect = ui.paletteView->visualRect(m_paletteModel->index(0, 0));
    const int minHeight = qMin(itemRect.height() * QPalette::NColorRoles,
                               (screen()->geometry().height() * 2) / 3);
    ui.paletteView->setMinimumSize({itemRect.width() * 4, minHeight});
}

PaletteEditor::~PaletteEditor() = default;

QPalette PaletteEditor::palette() const
{
    return m_editPalette;
}

void PaletteEditor::setPalette(const QPalette &palette)
{
    m_editPalette = palette;
    for (int r = 0; r < static_cast<int>(QPalette::NColorRoles); ++r) {
        for (int g = 0; g < static_cast<int>(QPalette::NColorGroups); ++g) {
            const auto role = static_cast<QPalette::ColorRole>(r);
            const auto group = static_cast<QPalette::ColorGroup>(g);
            if (!palette.isBrushSet(group, role))
                m_editPalette.setBrush(group, role, m_parentPalette.brush(group, role));
        }
    }
    m_editPalette.setResolveMask(palette.resolveMask());
    updatePreviewPalette();
    updateStyledButton();
    m_paletteUpdated = true;
    if (!m_modelUpdated)
        m_paletteModel->setPalette(m_editPalette, m_parentPalette);
    m_paletteUpdated = false;
}

void PaletteEditor::setPalette(const QPalette &palette, const QPalette &parentPalette)
{
    m_parentPalette = parentPalette;
    setPalette(palette);
}

void PaletteEditor::buildButtonColorChanged()
{
    buildPalette();
}

void PaletteEditor::activeRadioClicked()
{
    m_currentColorGroup = QPalette::Active;
    updatePreviewPalette();
}

void PaletteEditor::inactiveRadioClicked()
{
    m_currentColorGroup = QPalette::Inactive;
    updatePreviewPalette();
}

void PaletteEditor::disabledRadioClicked()
{
    m_currentColorGroup = QPalette::Disabled;
    updatePreviewPalette();
}

void PaletteEditor::computeRadioClicked()
{
    if (m_compute)
        return;
    ui.paletteView->setColumnHidden(2, true);
    ui.paletteView->setColumnHidden(3, true);
    m_compute = true;
    m_paletteModel->setCompute(true);
}

void PaletteEditor::detailsRadioClicked()
{
    if (!m_compute)
        return;
    const int w = ui.paletteView->columnWidth(1);
    ui.paletteView->setColumnHidden(2, false);
    ui.paletteView->setColumnHidden(3, false);
    QHeaderView *header = ui.paletteView->header();
    header->resizeSection(1, w / 3);
    header->resizeSection(2, w / 3);
    header->resizeSection(3, w / 3);
    m_compute = false;
    m_paletteModel->setCompute(false);
}

void PaletteEditor::paletteChanged(const QPalette &palette)
{
    m_modelUpdated = true;
    if (!m_paletteUpdated)
        setPalette(palette);
    m_modelUpdated = false;
}

void PaletteEditor::buildPalette()
{
    const QColor btn = ui.buildButton->color();
    const QPalette temp = QPalette(btn);
    setPalette(temp);
}

void PaletteEditor::updatePreviewPalette()
{
    const QPalette::ColorGroup g = currentColorGroup();
    // build the preview palette
    const QPalette currentPalette = palette();
    QPalette previewPalette;
    for (int i = QPalette::WindowText; i < QPalette::NColorRoles; i++) {
        const QPalette::ColorRole r = static_cast<QPalette::ColorRole>(i);
        const QBrush &br = currentPalette.brush(g, r);
        previewPalette.setBrush(QPalette::Active, r, br);
        previewPalette.setBrush(QPalette::Inactive, r, br);
        previewPalette.setBrush(QPalette::Disabled, r, br);
    }
    ui.previewFrame->setPreviewPalette(previewPalette);

    const bool enabled = g != QPalette::Disabled;
    ui.previewFrame->setEnabled(enabled);
    ui.previewFrame->setSubWindowActive(g != QPalette::Inactive);
}

void PaletteEditor::updateStyledButton()
{
    ui.buildButton->setColor(palette().color(QPalette::Active, QPalette::Button));
}

QPalette PaletteEditor::getPalette(QDesignerFormEditorInterface *core, QWidget* parent, const QPalette &init,
            const QPalette &parentPal, int *ok)
{
    PaletteEditor dlg(core, parent);
    QPalette parentPalette(parentPal);
    for (int r = 0; r < static_cast<int>(QPalette::NColorRoles); ++r) {
        for (int g = 0; g < static_cast<int>(QPalette::NColorGroups); ++g) {
            const auto role = static_cast<QPalette::ColorRole>(r);
            const auto group = static_cast<QPalette::ColorGroup>(g);
            if (!init.isBrushSet(group, role))
                parentPalette.setBrush(group, role, init.brush(group, role));
        }
    }
    dlg.setPalette(init, parentPalette);

    const int result = dlg.exec();
    if (ok) *ok = result;

    return result == QDialog::Accepted ? dlg.palette() : init;
}

void PaletteEditor::viewContextMenuRequested(QPoint pos)
{
    const auto index = ui.paletteView->indexAt(pos);
    if (!index.isValid())
        return;
    auto brush = m_paletteModel->brushAt(index);
    const auto color = brush.color();
    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
        m_lighterAction = m_contextMenu->addAction(tr("Lighter"));
        m_darkerAction = m_contextMenu->addAction(tr("Darker"));
        m_copyColorAction = m_contextMenu->addAction(QString());
    }
    const auto rgb = color.rgb() & 0xffffffu;
    const bool isBlack = rgb == 0u;
    m_lighterAction->setEnabled(rgb != 0xffffffu);
    m_darkerAction->setDisabled(isBlack);
    m_copyColorAction->setText(tr("Copy color %1").arg(color.name()));
    auto action = m_contextMenu->exec(ui.paletteView->viewport()->mapToGlobal(pos));
    if (!action)
        return;
    if (action == m_copyColorAction) {
#if QT_CONFIG(clipboard)
        QGuiApplication::clipboard()->setText(color.name());
#endif
        return;
    }
    // Fall through to darker/lighter. Note: black cannot be made lighter due
    // to QTBUG-9343.
    enum : int { factor = 120 };
    const QColor newColor = action == m_darkerAction
        ? color.darker(factor)
        : (isBlack ? QColor(0x404040u) : color.lighter(factor));
    brush.setColor(newColor);
    m_paletteModel->setData(index, QVariant(brush), BrushRole);
}

static inline QString paletteFilter()
{
    return PaletteEditor::tr("QPalette UI file (*.xml)");
}

static bool savePalette(const QString &fileName, const QPalette &pal, QString *errorMessage)
{
    QSaveFile file;
    file.setFileName(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        *errorMessage = PaletteEditor::tr("Cannot open %1 for writing: %2")
                        .arg(QDir::toNativeSeparators(fileName), file.errorString());
        return false;
    }
    {
        QScopedPointer<DomPalette> domPalette(QFormBuilderExtra::savePalette(pal));
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(1);
        writer.writeStartDocument();
        domPalette->write(writer);
        writer.writeEndDocument();
    }
    const bool result = file.commit();
    if (!result) {
        *errorMessage = PaletteEditor::tr("Cannot write %1: %2")
                        .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    return result;
}

static QString msgCannotReadPalette(const QString &fileName, const QXmlStreamReader &reader,
                                    const QString &why)
{
    return PaletteEditor::tr("Cannot read palette from %1:%2:%3")
           .arg(QDir::toNativeSeparators(fileName)).arg(reader.lineNumber()).arg(why);
}

static inline QString msgCannotReadPalette(const QString &fileName, const QXmlStreamReader &reader)
{
    return msgCannotReadPalette(fileName, reader, reader.errorString());
}

static bool loadPalette(const QString &fileName, QPalette *pal, QString *errorMessage)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        *errorMessage = PaletteEditor::tr("Cannot open %1 for reading: %2")
                        .arg(QDir::toNativeSeparators(fileName), file.errorString());
        return false;
    }
    QXmlStreamReader reader(&file);
    if (!reader.readNextStartElement()) {
        *errorMessage = msgCannotReadPalette(fileName, reader);
        return false;
    }
    if (reader.name() != "palette"_L1) {
        const auto why = PaletteEditor::tr("Invalid element \"%1\", expected \"palette\".")
                         .arg(reader.name().toString());
        *errorMessage = msgCannotReadPalette(fileName, reader, why);
        return false;
    }
    QScopedPointer<DomPalette> domPalette(new DomPalette);
    domPalette->read(reader);
    if (reader.hasError()) {
        *errorMessage = msgCannotReadPalette(fileName, reader);
        return false;
    }
    *pal = QFormBuilderExtra::loadPalette(domPalette.data());
    return true;
}

void PaletteEditor::save()
{
    QFileDialog dialog(this, tr("Save Palette"), QString(), paletteFilter());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(u"xml"_s);
    while (dialog.exec() == QDialog::Accepted) {
        QString errorMessage;
        if (savePalette(dialog.selectedFiles().constFirst(), palette(), &errorMessage))
            break;
        QMessageBox::warning(this, tr("Error Writing Palette"), errorMessage);
    }
}

void PaletteEditor::load()
{
    QFileDialog dialog(this, tr("Load Palette"), QString(), paletteFilter());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    while (dialog.exec() == QDialog::Accepted) {
        QPalette pal;
        QString errorMessage;
        if (loadPalette(dialog.selectedFiles().constFirst(), &pal, &errorMessage)) {
            setPalette(pal);
            break;
        }
        QMessageBox::warning(this, tr("Error Reading Palette"), errorMessage);
    }
}

//////////////////////
// Column 0: Role name and reset button. Uses a boolean value indicating
//           whether the role is modified for the edit role.
// Column 1: Color group Active
// Column 2: Color group Inactive (visibility depending on m_compute/detail radio group)
// Column 3: Color group Disabled

PaletteModel::PaletteModel(QObject *parent)  :
    QAbstractTableModel(parent)
{
    const QMetaObject *meta = metaObject();
    const int index = meta->indexOfProperty("colorRole");
    const QMetaProperty p = meta->property(index);
    const QMetaEnum e = p.enumerator();
    m_roleEntries.reserve(QPalette::NColorRoles);
    for (int r = QPalette::WindowText; r < QPalette::NColorRoles; r++) {
        const auto role = static_cast<QPalette::ColorRole>(r);
        if (role != QPalette::NoRole)
            m_roleEntries.append({QLatin1StringView(e.key(r)), role});
    }
}

int PaletteModel::rowCount(const QModelIndex &) const
{
    return m_roleEntries.size();
}

int PaletteModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QBrush PaletteModel::brushAt(const QModelIndex &index) const
{
    return m_palette.brush(columnToGroup(index.column()), roleAt(index.row()));
}

// Palette resolve mask with all group bits for a row/role
quint64 PaletteModel::rowMask(const QModelIndex &index) const
{
    return paletteResolveMask(roleAt(index.row()));
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= m_roleEntries.size())
        return QVariant();
    if (index.column() < 0 || index.column() >= 4)
        return QVariant();

    if (index.column() == 0) { // Role name/bold print if changed
        if (role == Qt::DisplayRole)
            return m_roleEntries.at(index.row()).name;
        if (role == Qt::EditRole)
            return (rowMask(index) & m_palette.resolveMask()) != 0;
        return QVariant();
    }
    if (role == Qt::ToolTipRole)
        return brushAt(index).color().name();
    if (role == BrushRole)
        return brushAt(index);
    return QVariant();
}

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    const int row = index.row();
    const auto colorRole = roleAt(row);

    if (index.column() != 0 && role == BrushRole) {
        const QBrush br = qvariant_cast<QBrush>(value);
        const QPalette::ColorGroup g = columnToGroup(index.column());
        m_palette.setBrush(g, colorRole, br);

        QModelIndex idxBegin = PaletteModel::index(row, 0);
        QModelIndex idxEnd = PaletteModel::index(row, 3);
        if (m_compute) {
            m_palette.setBrush(QPalette::Inactive, colorRole, br);
            switch (colorRole) {
                case QPalette::WindowText:
                case QPalette::Text:
                case QPalette::ButtonText:
                case QPalette::Base:
                    break;
                case QPalette::Dark:
                    m_palette.setBrush(QPalette::Disabled, QPalette::WindowText, br);
                    m_palette.setBrush(QPalette::Disabled, QPalette::Dark, br);
                    m_palette.setBrush(QPalette::Disabled, QPalette::Text, br);
                    m_palette.setBrush(QPalette::Disabled, QPalette::ButtonText, br);
                    idxBegin = PaletteModel::index(0, 0);
                    idxEnd = PaletteModel::index(m_roleEntries.size() - 1, 3);
                    break;
                case QPalette::Window:
                    m_palette.setBrush(QPalette::Disabled, QPalette::Base, br);
                    m_palette.setBrush(QPalette::Disabled, QPalette::Window, br);
                    idxBegin = PaletteModel::index(rowOf(QPalette::Base), 0);
                    break;
                case QPalette::Highlight:
                    //m_palette.setBrush(QPalette::Disabled, QPalette::Highlight, c.dark(120));
                    break;
                default:
                    m_palette.setBrush(QPalette::Disabled, colorRole, br);
                    break;
            }
        }
        emit paletteChanged(m_palette);
        emit dataChanged(idxBegin, idxEnd);
        return true;
    }
    if (index.column() == 0 && role == Qt::EditRole) {
        auto mask = m_palette.resolveMask();
        const bool isMask = qvariant_cast<bool>(value);
        const auto bitMask = rowMask(index);
        if (isMask) {
            mask |= bitMask;
        } else {
            m_palette.setBrush(QPalette::Active, colorRole,
                               m_parentPalette.brush(QPalette::Active, colorRole));
            m_palette.setBrush(QPalette::Inactive, colorRole,
                               m_parentPalette.brush(QPalette::Inactive, colorRole));
            m_palette.setBrush(QPalette::Disabled, colorRole,
                               m_parentPalette.brush(QPalette::Disabled, colorRole));

            mask &= ~bitMask;
        }
        m_palette.setResolveMask(mask);
        emit paletteChanged(m_palette);
        const QModelIndex idxEnd = PaletteModel::index(row, 3);
        emit dataChanged(index, idxEnd);
        return true;
    }
    return false;
}

Qt::ItemFlags PaletteModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation,
                int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Color Role");
        if (section == groupToColumn(QPalette::Active))
            return tr("Active");
        if (section == groupToColumn(QPalette::Inactive))
            return tr("Inactive");
        if (section == groupToColumn(QPalette::Disabled))
            return tr("Disabled");
    }
    return QVariant();
}

QPalette PaletteModel::getPalette() const
{
    return m_palette;
}

void PaletteModel::setPalette(const QPalette &palette, const QPalette &parentPalette)
{
    m_parentPalette = parentPalette;
    m_palette = palette;
    const QModelIndex idxBegin = index(0, 0);
    const QModelIndex idxEnd = index(m_roleEntries.size() - 1, 3);
    emit dataChanged(idxBegin, idxEnd);
}

QPalette::ColorGroup PaletteModel::columnToGroup(int index) const
{
    if (index == 1)
        return QPalette::Active;
    if (index == 2)
        return QPalette::Inactive;
    return QPalette::Disabled;
}

int PaletteModel::groupToColumn(QPalette::ColorGroup group) const
{
    if (group == QPalette::Active)
        return 1;
    if (group == QPalette::Inactive)
        return 2;
    return 3;
}

int PaletteModel::rowOf(QPalette::ColorRole role) const
{
    for (qsizetype row = 0, size = m_roleEntries.size(); row < size; ++row) {
        if (m_roleEntries.at(row).role == role)
            return row;
    }
    return -1;
}

//////////////////////////

BrushEditor::BrushEditor(QDesignerFormEditorInterface *core, QWidget *parent) :
    QWidget(parent),
    m_button(new QtColorButton(this)),
    m_core(core)
{
    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->addWidget(m_button);
    connect(m_button, &QtColorButton::colorChanged, this, &BrushEditor::brushChanged);
    setFocusProxy(m_button);
}

void BrushEditor::setBrush(const QBrush &brush)
{
    m_button->setColor(brush.color());
    m_changed = false;
}

QBrush BrushEditor::brush() const
{
    return QBrush(m_button->color());
}

void BrushEditor::brushChanged()
{
    m_changed = true;
    emit changed(this);
}

bool BrushEditor::changed() const
{
    return m_changed;
}

//////////////////////////

RoleEditor::RoleEditor(QWidget *parent) :
    QWidget(parent),
    m_label(new QLabel(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    layout->addWidget(m_label);
    m_label->setAutoFillBackground(true);
    m_label->setIndent(3); // ### hardcode it should have the same value of textMargin in QItemDelegate
    setFocusProxy(m_label);

    QToolButton *button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIcon(createIconSet("resetproperty.png"_L1));
    button->setIconSize(QSize(8,8));
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    layout->addWidget(button);
    connect(button, &QAbstractButton::clicked, this, &RoleEditor::emitResetProperty);
}

void RoleEditor::setLabel(const QString &label)
{
    m_label->setText(label);
}

void RoleEditor::setEdited(bool on)
{
    QFont font;
    if (on)
        font.setBold(on);
    m_label->setFont(font);
    m_edited = on;
}

bool RoleEditor::edited() const
{
    return m_edited;
}

void RoleEditor::emitResetProperty()
{
    setEdited(false);
    emit changed(this);
}

//////////////////////////
ColorDelegate::ColorDelegate(QDesignerFormEditorInterface *core, QObject *parent) :
    QStyledItemDelegate(parent),
    m_core(core)
{
}

QWidget *ColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                const QModelIndex &index) const
{
    QWidget *ed = nullptr;
    if (index.column() == 0) {
        RoleEditor *editor = new RoleEditor(parent);
        connect(editor, &RoleEditor::changed, this, &ColorDelegate::commitData);
        //editor->setFocusPolicy(Qt::NoFocus);
        //editor->installEventFilter(const_cast<ColorDelegate *>(this));
        ed = editor;
    } else {
        BrushEditor *editor = new BrushEditor(m_core, parent);
        connect(editor, QOverload<QWidget *>::of(&BrushEditor::changed),
                this, &ColorDelegate::commitData);
        editor->setFocusPolicy(Qt::NoFocus);
        editor->installEventFilter(const_cast<ColorDelegate *>(this));
        ed = editor;
    }
    return ed;
}

void ColorDelegate::setEditorData(QWidget *ed, const QModelIndex &index) const
{
    if (index.column() == 0) {
        const bool mask = qvariant_cast<bool>(index.model()->data(index, Qt::EditRole));
        RoleEditor *editor = static_cast<RoleEditor *>(ed);
        editor->setEdited(mask);
        const QString colorName = qvariant_cast<QString>(index.model()->data(index, Qt::DisplayRole));
        editor->setLabel(colorName);
    } else {
        const QBrush br = qvariant_cast<QBrush>(index.model()->data(index, BrushRole));
        BrushEditor *editor = static_cast<BrushEditor *>(ed);
        editor->setBrush(br);
    }
}

void ColorDelegate::setModelData(QWidget *ed, QAbstractItemModel *model,
                const QModelIndex &index) const
{
    if (index.column() == 0) {
        RoleEditor *editor = static_cast<RoleEditor *>(ed);
        const bool mask = editor->edited();
        model->setData(index, mask, Qt::EditRole);
    } else {
        BrushEditor *editor = static_cast<BrushEditor *>(ed);
        if (editor->changed()) {
            QBrush br = editor->brush();
            model->setData(index, br, BrushRole);
        }
    }
}

void ColorDelegate::updateEditorGeometry(QWidget *ed,
                const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(ed, option, index);
    ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
}

void ColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
            const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    const bool mask = qvariant_cast<bool>(index.model()->data(index, Qt::EditRole));
    if (index.column() == 0 && mask) {
        option.font.setBold(true);
    }
    QBrush br = qvariant_cast<QBrush>(index.model()->data(index, BrushRole));
    if (br.style() == Qt::LinearGradientPattern ||
            br.style() == Qt::RadialGradientPattern ||
            br.style() == Qt::ConicalGradientPattern) {
        painter->save();
        painter->translate(option.rect.x(), option.rect.y());
        painter->scale(option.rect.width(), option.rect.height());
        QGradient gr = *(br.gradient());
        gr.setCoordinateMode(QGradient::LogicalMode);
        br = QBrush(gr);
        painter->fillRect(0, 0, 1, 1, br);
        painter->restore();
    } else {
        painter->save();
        painter->setBrushOrigin(option.rect.x(), option.rect.y());
        painter->fillRect(option.rect, br);
        painter->restore();
    }
    QStyledItemDelegate::paint(painter, option, index);


    const QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    const QPen oldPen = painter->pen();
    painter->setPen(QPen(color));

    painter->drawLine(option.rect.right(), option.rect.y(),
            option.rect.right(), option.rect.bottom());
    painter->drawLine(option.rect.x(), option.rect.bottom(),
            option.rect.right(), option.rect.bottom());
    painter->setPen(oldPen);
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}
}

QT_END_NAMESPACE
