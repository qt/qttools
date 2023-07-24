// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "stylesheeteditor_p.h"
#include "csshighlighter_p.h"
#include "iconselector_p.h"
#include "qtgradientmanager.h"
#include "qtgradientviewdialog.h"
#include "qtgradientutils.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/qextensionmanager.h>

#include <texteditfindwidget.h>

#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qfontdialog.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qboxlayout.h>

#include <QtGui/qaction.h>
#include <QtGui/qevent.h>
#include <QtGui/qtextdocument.h>

#include <private/qcssparser_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const char styleSheetProperty[] = "styleSheet";
static const char StyleSheetDialogC[] = "StyleSheetDialog";
static const char seGeometry[] = "Geometry";

namespace qdesigner_internal {

StyleSheetEditor::StyleSheetEditor(QWidget *parent)
    : QTextEdit(parent)
{
    enum : int { DarkThreshold = 200 }; // Observed 239 on KDE/Dark

    setTabStopDistance(fontMetrics().horizontalAdvance(u' ') * 4);
    setAcceptRichText(false);

    const QColor textColor = palette().color(QPalette::WindowText);
    const bool darkMode = textColor.red() > DarkThreshold
        && textColor.green() > DarkThreshold
        && textColor.blue() > DarkThreshold;

    CssHighlightColors colors;
    colors.selector = darkMode ? QColor(Qt::red).lighter() : QColor(Qt::darkRed);
    const QColor blue(Qt::blue);
    colors.property = darkMode ? blue.lighter() : blue;
    colors.pseudo1 = colors.pseudo2 = colors.value = textColor;
    colors.quote = darkMode ? Qt::magenta : Qt::darkMagenta;
    colors.comment = darkMode ? Qt::green : Qt::darkGreen;

    new CssHighlighter(colors, document());
}

// --- StyleSheetEditorDialog
StyleSheetEditorDialog::StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent, Mode mode):
    QDialog(parent),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help)),
    m_editor(new StyleSheetEditor),
    m_findWidget(new TextEditFindWidget),
    m_validityLabel(new QLabel(tr("Valid Style Sheet"))),
    m_core(core),
    m_addResourceAction(new QAction(tr("Add Resource..."), this)),
    m_addGradientAction(new QAction(tr("Add Gradient..."), this)),
    m_addColorAction(new QAction(tr("Add Color..."), this)),
    m_addFontAction(new QAction(tr("Add Font..."), this))
{
    setWindowTitle(tr("Edit Style Sheet"));

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_buttonBox, &QDialogButtonBox::helpRequested,
            this, &StyleSheetEditorDialog::slotRequestHelp);
    m_buttonBox->button(QDialogButtonBox::Help)->setShortcut(QKeySequence::HelpContents);

    connect(m_editor, &QTextEdit::textChanged, this, &StyleSheetEditorDialog::validateStyleSheet);
    m_findWidget->setTextEdit(m_editor);

    QToolBar *toolBar = new QToolBar;

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(toolBar, 0, 0, 1, 2);
    layout->addWidget(m_editor, 1, 0, 1, 2);
    layout->addWidget(m_findWidget, 2, 0, 1, 2);
    layout->addWidget(m_validityLabel, 3, 0, 1, 1);
    layout->addWidget(m_buttonBox, 3, 1, 1, 1);
    setLayout(layout);

    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, &QWidget::customContextMenuRequested,
            this, &StyleSheetEditorDialog::slotContextMenuRequested);

    connect(m_addResourceAction, &QAction::triggered,
            this, [this] { this->slotAddResource(QString()); });
    connect(m_addGradientAction, &QAction::triggered,
            this, [this] { this->slotAddGradient(QString()); });
    connect(m_addColorAction, &QAction::triggered,
            this, [this] { this->slotAddColor(QString()); });
    connect(m_addFontAction, &QAction::triggered, this, &StyleSheetEditorDialog::slotAddFont);

    m_addResourceAction->setEnabled(mode == ModePerForm);

    const char * const resourceProperties[] = {
        "background-image",
        "border-image",
        "image",
        nullptr
    };

    const char * const colorProperties[] = {
        "color",
        "background-color",
        "alternate-background-color",
        "border-color",
        "border-top-color",
        "border-right-color",
        "border-bottom-color",
        "border-left-color",
        "gridline-color",
        "selection-color",
        "selection-background-color",
        nullptr
    };

    QMenu *resourceActionMenu = new QMenu(this);
    QMenu *gradientActionMenu = new QMenu(this);
    QMenu *colorActionMenu = new QMenu(this);

    for (int resourceProperty = 0; resourceProperties[resourceProperty]; ++resourceProperty) {
        const QString resourcePropertyName = QLatin1StringView(resourceProperties[resourceProperty]);
        resourceActionMenu->addAction(resourcePropertyName,
                                      this, [this, resourcePropertyName] { this->slotAddResource(resourcePropertyName); });
    }

    for (int colorProperty = 0; colorProperties[colorProperty]; ++colorProperty) {
        const QString colorPropertyName = QLatin1StringView(colorProperties[colorProperty]);
        colorActionMenu->addAction(colorPropertyName,
                                   this, [this, colorPropertyName] { this->slotAddColor(colorPropertyName); });
        gradientActionMenu->addAction(colorPropertyName,
                                      this, [this, colorPropertyName] { this->slotAddGradient(colorPropertyName); } );
    }

    m_addResourceAction->setMenu(resourceActionMenu);
    m_addGradientAction->setMenu(gradientActionMenu);
    m_addColorAction->setMenu(colorActionMenu);


    toolBar->addAction(m_addResourceAction);
    toolBar->addAction(m_addGradientAction);
    toolBar->addAction(m_addColorAction);
    toolBar->addAction(m_addFontAction);
    m_findAction = m_findWidget->createFindAction(toolBar);
    toolBar->addAction(m_findAction);

    m_editor->setFocus();

    QDesignerSettingsInterface *settings = core->settingsManager();
    settings->beginGroup(QLatin1StringView(StyleSheetDialogC));

    if (settings->contains(QLatin1StringView(seGeometry)))
        restoreGeometry(settings->value(QLatin1StringView(seGeometry)).toByteArray());

    settings->endGroup();
}

StyleSheetEditorDialog::~StyleSheetEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(QLatin1StringView(StyleSheetDialogC));

    settings->setValue(QLatin1StringView(seGeometry), saveGeometry());
    settings->endGroup();
}

void StyleSheetEditorDialog::setOkButtonEnabled(bool v)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(v);
    if (QPushButton *applyButton = m_buttonBox->button(QDialogButtonBox::Apply))
        applyButton->setEnabled(v);
}

void StyleSheetEditorDialog::slotContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = m_editor->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(m_findAction);
    menu->addSeparator();
    menu->addAction(m_addResourceAction);
    menu->addAction(m_addGradientAction);
    menu->exec(mapToGlobal(pos));
    delete menu;
}

void StyleSheetEditorDialog::slotAddResource(const QString &property)
{
    const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);
    if (!path.isEmpty())
        insertCssProperty(property, "url("_L1 + path + u')');
}

void StyleSheetEditorDialog::slotAddGradient(const QString &property)
{
    bool ok;
    const QGradient grad = QtGradientViewDialog::getGradient(&ok, m_core->gradientManager(), this);
    if (ok)
        insertCssProperty(property, QtGradientUtils::styleSheetCode(grad));
}

void StyleSheetEditorDialog::slotAddColor(const QString &property)
{
    const QColor color = QColorDialog::getColor(0xffffffff, this, QString(), QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;

    QString colorStr;

    if (color.alpha() == 255) {
        colorStr = QString::asprintf("rgb(%d, %d, %d)",
                                     color.red(), color.green(), color.blue());
    } else {
        colorStr = QString::asprintf("rgba(%d, %d, %d, %d)",
                                     color.red(), color.green(), color.blue(),
                                     color.alpha());
    }

    insertCssProperty(property, colorStr);
}

void StyleSheetEditorDialog::slotAddFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        QString fontStr;
        if (font.weight() != QFont::Normal)
            fontStr += QString::number(font.weight()) + u' ';

        switch (font.style()) {
        case QFont::StyleItalic:
            fontStr += "italic "_L1;
            break;
        case QFont::StyleOblique:
            fontStr += "oblique "_L1;
            break;
        default:
            break;
        }
        fontStr += QString::number(font.pointSize());
        fontStr += "pt \""_L1;
        fontStr += font.family();
        fontStr += u'"';

        insertCssProperty(u"font"_s, fontStr);
        QString decoration;
        if (font.underline())
            decoration += "underline"_L1;
        if (font.strikeOut()) {
            if (!decoration.isEmpty())
                decoration += u' ';
            decoration += "line-through"_L1;
        }
        insertCssProperty(u"text-decoration"_s, decoration);
    }
}

void StyleSheetEditorDialog::insertCssProperty(const QString &name, const QString &value)
{
    if (!value.isEmpty()) {
        QTextCursor cursor = m_editor->textCursor();
        if (!name.isEmpty()) {
            cursor.beginEditBlock();
            cursor.removeSelectedText();
            cursor.movePosition(QTextCursor::EndOfLine);

            // Simple check to see if we're in a selector scope
            const QTextDocument *doc = m_editor->document();
            const QTextCursor closing = doc->find(u"}"_s, cursor, QTextDocument::FindBackward);
            const QTextCursor opening = doc->find(u"{"_s, cursor, QTextDocument::FindBackward);
            const bool inSelector = !opening.isNull() && (closing.isNull() ||
                                                          closing.position() < opening.position());
            QString insertion;
            if (m_editor->textCursor().block().length() != 1)
                insertion += u'\n';
            if (inSelector)
                insertion += u'\t';
            insertion += name;
            insertion += ": "_L1;
            insertion += value;
            insertion += u';';
            cursor.insertText(insertion);
            cursor.endEditBlock();
        } else {
            cursor.insertText(value);
        }
    }
}

void StyleSheetEditorDialog::slotRequestHelp()
{
    m_core->integration()->emitHelpRequested(u"qtwidgets"_s,
                                             u"stylesheet-reference.html"_s);
}

// See QDialog::keyPressEvent()
static inline bool isEnter(const QKeyEvent *e)
{
    const bool isEnter = e->key() == Qt::Key_Enter;
    const bool isReturn = e->key() == Qt::Key_Return;
    return (e->modifiers() == Qt::KeyboardModifiers() && (isEnter || isReturn))
        || (e->modifiers().testFlag(Qt::KeypadModifier) && isEnter);
}

void StyleSheetEditorDialog::keyPressEvent(QKeyEvent *e)
{
    // As long as the find widget is visible, suppress the default button
    // behavior (close on Enter) of QDialog.
    if (!(m_findWidget->isVisible() && isEnter(e)))
        QDialog::keyPressEvent(e);
}

QDialogButtonBox * StyleSheetEditorDialog::buttonBox() const
{
   return m_buttonBox;
}

QString StyleSheetEditorDialog::text() const
{
    return m_editor->toPlainText();
}

void StyleSheetEditorDialog::setText(const QString &t)
{
    m_editor->setText(t);
}

bool StyleSheetEditorDialog::isStyleSheetValid(const QString &styleSheet)
{
    QCss::Parser parser(styleSheet);
    QCss::StyleSheet sheet;
    if (parser.parse(&sheet))
        return true;
    QCss::Parser parser2("* { "_L1 + styleSheet + '}'_L1);
    return parser2.parse(&sheet);
}

void StyleSheetEditorDialog::validateStyleSheet()
{
    const bool valid = isStyleSheetValid(m_editor->toPlainText());
    setOkButtonEnabled(valid);
    if (valid) {
        m_validityLabel->setText(tr("Valid Style Sheet"));
        m_validityLabel->setStyleSheet(u"color: green"_s);
    } else {
        m_validityLabel->setText(tr("Invalid Style Sheet"));
        m_validityLabel->setStyleSheet(u"color: red"_s);
    }
}

// --- StyleSheetPropertyEditorDialog
StyleSheetPropertyEditorDialog::StyleSheetPropertyEditorDialog(QWidget *parent,
                                               QDesignerFormWindowInterface *fw,
                                               QWidget *widget):
    StyleSheetEditorDialog(fw->core(), parent),
    m_fw(fw),
    m_widget(widget)
{
    Q_ASSERT(m_fw != nullptr);

    QPushButton *apply = buttonBox()->addButton(QDialogButtonBox::Apply);
    QObject::connect(apply, &QAbstractButton::clicked,
                     this, &StyleSheetPropertyEditorDialog::applyStyleSheet);
    QObject::connect(buttonBox(), &QDialogButtonBox::accepted,
                     this, &StyleSheetPropertyEditorDialog::applyStyleSheet);

    QDesignerPropertySheetExtension *sheet =
            qt_extension<QDesignerPropertySheetExtension*>(m_fw->core()->extensionManager(), m_widget);
    Q_ASSERT(sheet != nullptr);
    const int index = sheet->indexOf(QLatin1StringView(styleSheetProperty));
    const PropertySheetStringValue value = qvariant_cast<PropertySheetStringValue>(sheet->property(index));
    setText(value.value());
}

void StyleSheetPropertyEditorDialog::applyStyleSheet()
{
    const PropertySheetStringValue value(text(), false);
    m_fw->cursor()->setWidgetProperty(m_widget, QLatin1StringView(styleSheetProperty), QVariant::fromValue(value));
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
