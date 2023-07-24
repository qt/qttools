// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "richtexteditor_p.h"
#include "htmlhighlighter_p.h"
#include "iconselector_p.h"
#include "ui_addlinkdialog.h"

#include "iconloader_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractsettings.h>

#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qtabwidget.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialogbuttonbox.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qevent.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qicon.h>
#include <QtGui/qpainter.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextobject.h>

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtCore/qxmlstream.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const char RichTextDialogGroupC[] = "RichTextDialog";
static const char GeometryKeyC[] = "Geometry";
static const char TabKeyC[] = "Tab";

const bool simplifyRichTextDefault = true;

namespace qdesigner_internal {

// Richtext simplification filter helpers: Elements to be discarded
static inline bool filterElement(QStringView name)
{
    return name != "meta"_L1 && name != "style"_L1;
}

// Richtext simplification filter helpers: Filter attributes of elements
static inline void filterAttributes(QStringView name,
                                    QXmlStreamAttributes *atts,
                                    bool *paragraphAlignmentFound)
{
    if (atts->isEmpty())
        return;

     // No style attributes for <body>
    if (name == "body"_L1) {
        atts->clear();
        return;
    }

    // Clean out everything except 'align' for 'p'
    if (name == "p"_L1) {
        for (auto it = atts->begin(); it != atts->end(); ) {
            if (it->name() == "align"_L1) {
                ++it;
                *paragraphAlignmentFound = true;
            } else {
                it = atts->erase(it);
            }
        }
        return;
    }
}

// Richtext simplification filter helpers: Check for blank QStringView.
static inline bool isWhiteSpace(QStringView in)
{
    return std::all_of(in.cbegin(), in.cend(),
                       [](QChar c) { return c.isSpace(); });
}

// Richtext simplification filter: Remove hard-coded font settings,
// <style> elements, <p> attributes other than 'align' and
// and unnecessary meta-information.
QString simplifyRichTextFilter(const QString &in, bool *isPlainTextPtr = nullptr)
{
    unsigned elementCount = 0;
    bool paragraphAlignmentFound = false;
    QString out;
    QXmlStreamReader reader(in);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(false);
    writer.setAutoFormattingIndent(0);

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            elementCount++;
            if (filterElement(reader.name())) {
                const auto name = reader.name();
                QXmlStreamAttributes attributes = reader.attributes();
                filterAttributes(name, &attributes, &paragraphAlignmentFound);
                writer.writeStartElement(name.toString());
                if (!attributes.isEmpty())
                    writer.writeAttributes(attributes);
            } else {
                reader.readElementText(); // Skip away all nested elements and characters.
            }
            break;
        case QXmlStreamReader::Characters:
            if (!isWhiteSpace(reader.text()))
                writer.writeCharacters(reader.text().toString());
            break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        default:
            break;
        }
    }
    // Check for plain text (no spans, just <html><head><body><p>)
    if (isPlainTextPtr)
        *isPlainTextPtr = !paragraphAlignmentFound && elementCount == 4u; //
    return out;
}

class RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit RichTextEditor(QWidget *parent = nullptr);
    void setDefaultFont(QFont font);

    QToolBar *createToolBar(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

    QString text(Qt::TextFormat format) const;

    bool simplifyRichText() const      { return m_simplifyRichText; }

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);
    void setSimplifyRichText(bool v);

signals:
    void stateChanged();
    void simplifyRichTextChanged(bool);

private:
    bool m_simplifyRichText;
};

class AddLinkDialog : public QDialog
{
    Q_OBJECT

public:
    AddLinkDialog(RichTextEditor *editor, QWidget *parent = nullptr);
    ~AddLinkDialog() override;

    int showDialog();

public slots:
    void accept() override;

private:
    RichTextEditor *m_editor;
    QT_PREPEND_NAMESPACE(Ui)::AddLinkDialog *m_ui;
};

AddLinkDialog::AddLinkDialog(RichTextEditor *editor, QWidget *parent) :
    QDialog(parent),
    m_ui(new QT_PREPEND_NAMESPACE(Ui)::AddLinkDialog)
{
    m_ui->setupUi(this);

    m_editor = editor;
}

AddLinkDialog::~AddLinkDialog()
{
    delete m_ui;
}

int AddLinkDialog::showDialog()
{
    // Set initial focus
    const QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        m_ui->titleInput->setText(cursor.selectedText());
        m_ui->urlInput->setFocus();
    } else {
        m_ui->titleInput->setFocus();
    }

    return exec();
}

void AddLinkDialog::accept()
{
    const QString title = m_ui->titleInput->text();
    const QString url = m_ui->urlInput->text();

    if (!title.isEmpty()) {
        const QString html = "<a href=\""_L1 + url + "\">"_L1 + title + "</a>"_L1;
        m_editor->insertHtml(html);
    }

    m_ui->titleInput->clear();
    m_ui->urlInput->clear();

    QDialog::accept();
}

class HtmlTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    HtmlTextEdit(QWidget *parent = nullptr)
        : QTextEdit(parent)
    {}

    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void actionTriggered(QAction *action);
};

void HtmlTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    QMenu *htmlMenu = new QMenu(tr("Insert HTML entity"), menu);

    typedef struct {
        const char *text;
        const char *entity;
    } Entry;

    const Entry entries[] = {
        { "&&amp; (&&)", "&amp;" },
        { "&&nbsp;", "&nbsp;" },
        { "&&lt; (<)", "&lt;" },
        { "&&gt; (>)", "&gt;" },
        { "&&copy; (Copyright)", "&copy;" },
        { "&&reg; (Trade Mark)", "&reg;" },
    };

    for (const Entry &e : entries) {
        QAction *entityAction = new QAction(QLatin1StringView(e.text),
                                            htmlMenu);
        entityAction->setData(QLatin1StringView(e.entity));
        htmlMenu->addAction(entityAction);
    }

    menu->addMenu(htmlMenu);
    connect(htmlMenu, &QMenu::triggered, this, &HtmlTextEdit::actionTriggered);
    menu->exec(event->globalPos());
    delete menu;
}

void HtmlTextEdit::actionTriggered(QAction *action)
{
    insertPlainText(action->data().toString());
}

class ColorAction : public QAction
{
    Q_OBJECT

public:
    ColorAction(QObject *parent);

    const QColor& color() const { return m_color; }
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void chooseColor();

private:
    QColor m_color;
};

ColorAction::ColorAction(QObject *parent):
    QAction(parent)
{
    setText(tr("Text Color"));
    setColor(Qt::black);
    connect(this, &QAction::triggered, this, &ColorAction::chooseColor);
}

void ColorAction::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    QPixmap pix(24, 24);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(pix.rect(), m_color);
    painter.setPen(m_color.darker());
    painter.drawRect(pix.rect().adjusted(0, 0, -1, -1));
    setIcon(pix);
}

void ColorAction::chooseColor()
{
    const QColor col = QColorDialog::getColor(m_color, nullptr);
    if (col.isValid() && col != m_color) {
        setColor(col);
        emit colorChanged(m_color);
    }
}

class RichTextEditorToolBar : public QToolBar
{
    Q_OBJECT
public:
    RichTextEditorToolBar(QDesignerFormEditorInterface *core,
                          RichTextEditor *editor,
                          QWidget *parent = nullptr);

public slots:
    void updateActions();

private slots:
    void alignmentActionTriggered(QAction *action);
    void sizeInputActivated(const QString &size);
    void colorChanged(const QColor &color);
    void setVAlignSuper(bool super);
    void setVAlignSub(bool sub);
    void insertLink();
    void insertImage();
    void layoutDirectionChanged();

private:
    QAction *m_bold_action;
    QAction *m_italic_action;
    QAction *m_underline_action;
    QAction *m_valign_sup_action;
    QAction *m_valign_sub_action;
    QAction *m_align_left_action;
    QAction *m_align_center_action;
    QAction *m_align_right_action;
    QAction *m_align_justify_action;
    QAction *m_layoutDirectionAction;
    QAction *m_link_action;
    QAction *m_image_action;
    QAction *m_simplify_richtext_action;
    ColorAction *m_color_action;
    QComboBox *m_font_size_input;

    QDesignerFormEditorInterface *m_core;
    QPointer<RichTextEditor> m_editor;
};

static QAction *createCheckableAction(const QIcon &icon, const QString &text,
                                      QObject *parent = nullptr)
{
    QAction *result = new QAction(parent);
    result->setIcon(icon);
    result->setText(text);
    result->setCheckable(true);
    result->setChecked(false);
    return result;
}

RichTextEditorToolBar::RichTextEditorToolBar(QDesignerFormEditorInterface *core,
                                             RichTextEditor *editor,
                                             QWidget *parent) :
    QToolBar(parent),
    m_link_action(new QAction(this)),
    m_image_action(new QAction(this)),
    m_color_action(new ColorAction(this)),
    m_font_size_input(new QComboBox),
    m_core(core),
    m_editor(editor)
{
    // Font size combo box
    m_font_size_input->setEditable(false);
    const auto font_sizes = QFontDatabase::standardSizes();
    for (int font_size : font_sizes)
        m_font_size_input->addItem(QString::number(font_size));

    connect(m_font_size_input, &QComboBox::textActivated,
            this, &RichTextEditorToolBar::sizeInputActivated);
    addWidget(m_font_size_input);

    addSeparator();

    // Bold, italic and underline buttons

    m_bold_action = createCheckableAction(
            createIconSet(u"textbold.png"_s), tr("Bold"), this);
    connect(m_bold_action, &QAction::triggered, editor, &RichTextEditor::setFontBold);
    m_bold_action->setShortcut(tr("CTRL+B"));
    addAction(m_bold_action);

    m_italic_action = createCheckableAction(
            createIconSet(u"textitalic.png"_s), tr("Italic"), this);
    connect(m_italic_action, &QAction::triggered, editor, &RichTextEditor::setFontItalic);
    m_italic_action->setShortcut(tr("CTRL+I"));
    addAction(m_italic_action);

    m_underline_action = createCheckableAction(
            createIconSet(u"textunder.png"_s), tr("Underline"), this);
    connect(m_underline_action, &QAction::triggered, editor, &RichTextEditor::setFontUnderline);
    m_underline_action->setShortcut(tr("CTRL+U"));
    addAction(m_underline_action);

    addSeparator();

    // Left, center, right and justified alignment buttons

    QActionGroup *alignment_group = new QActionGroup(this);
    connect(alignment_group, &QActionGroup::triggered,
            this, &RichTextEditorToolBar::alignmentActionTriggered);

    m_align_left_action = createCheckableAction(
            createIconSet(u"textleft.png"_s), tr("Left Align"), alignment_group);
    addAction(m_align_left_action);

    m_align_center_action = createCheckableAction(
            createIconSet(u"textcenter.png"_s), tr("Center"), alignment_group);
    addAction(m_align_center_action);

    m_align_right_action = createCheckableAction(
            createIconSet(u"textright.png"_s), tr("Right Align"), alignment_group);
    addAction(m_align_right_action);

    m_align_justify_action = createCheckableAction(
            createIconSet(u"textjustify.png"_s), tr("Justify"), alignment_group);
    addAction(m_align_justify_action);

    m_layoutDirectionAction = createCheckableAction(
            createIconSet(u"righttoleft.png"_s), tr("Right to Left"));
    connect(m_layoutDirectionAction, &QAction::triggered,
            this, &RichTextEditorToolBar::layoutDirectionChanged);
    addAction(m_layoutDirectionAction);

    addSeparator();

    // Superscript and subscript buttons

    m_valign_sup_action = createCheckableAction(
            createIconSet(u"textsuperscript.png"_s), tr("Superscript"), this);
    connect(m_valign_sup_action, &QAction::triggered,
            this, &RichTextEditorToolBar::setVAlignSuper);
    addAction(m_valign_sup_action);

    m_valign_sub_action = createCheckableAction(
            createIconSet(u"textsubscript.png"_s), tr("Subscript"), this);
    connect(m_valign_sub_action, &QAction::triggered,
            this, &RichTextEditorToolBar::setVAlignSub);
    addAction(m_valign_sub_action);

    addSeparator();

    // Insert hyperlink and image buttons

    m_link_action->setIcon(createIconSet(u"textanchor.png"_s));
    m_link_action->setText(tr("Insert &Link"));
    connect(m_link_action, &QAction::triggered, this, &RichTextEditorToolBar::insertLink);
    addAction(m_link_action);

    m_image_action->setIcon(createIconSet(u"insertimage.png"_s));
    m_image_action->setText(tr("Insert &Image"));
    connect(m_image_action, &QAction::triggered, this, &RichTextEditorToolBar::insertImage);
    addAction(m_image_action);

    addSeparator();

    // Text color button
    connect(m_color_action, &ColorAction::colorChanged,
            this, &RichTextEditorToolBar::colorChanged);
    addAction(m_color_action);

    addSeparator();

    // Simplify rich text
    m_simplify_richtext_action
        = createCheckableAction(createIconSet(u"simplifyrichtext.png"_s), tr("Simplify Rich Text"));
    connect(m_simplify_richtext_action, &QAction::triggered,
            m_editor, &RichTextEditor::setSimplifyRichText);
    m_simplify_richtext_action->setChecked(m_editor->simplifyRichText());
    connect(m_editor.data(), &RichTextEditor::simplifyRichTextChanged,
            m_simplify_richtext_action, &QAction::setChecked);
    addAction(m_simplify_richtext_action);

    connect(editor, &QTextEdit::textChanged, this, &RichTextEditorToolBar::updateActions);
    connect(editor, &RichTextEditor::stateChanged, this, &RichTextEditorToolBar::updateActions);

    updateActions();
}

void RichTextEditorToolBar::alignmentActionTriggered(QAction *action)
{
    Qt::Alignment new_alignment;

    if (action == m_align_left_action) {
        new_alignment = Qt::AlignLeft;
    } else if (action == m_align_center_action) {
        new_alignment = Qt::AlignCenter;
    } else if (action == m_align_right_action) {
        new_alignment = Qt::AlignRight;
    } else {
        new_alignment = Qt::AlignJustify;
    }

    m_editor->setAlignment(new_alignment);
}

void RichTextEditorToolBar::colorChanged(const QColor &color)
{
    m_editor->setTextColor(color);
    m_editor->setFocus();
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
    bool ok;
    int i = size.toInt(&ok);
    if (!ok)
        return;

    m_editor->setFontPointSize(i);
    m_editor->setFocus();
}

void RichTextEditorToolBar::setVAlignSuper(bool super)
{
    const QTextCharFormat::VerticalAlignment align = super ?
        QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sub_action->setChecked(false);
}

void RichTextEditorToolBar::setVAlignSub(bool sub)
{
    const QTextCharFormat::VerticalAlignment align = sub ?
        QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sup_action->setChecked(false);
}

void RichTextEditorToolBar::insertLink()
{
    AddLinkDialog linkDialog(m_editor, this);
    linkDialog.showDialog();
    m_editor->setFocus();
}

void RichTextEditorToolBar::insertImage()
{
    const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);
    if (!path.isEmpty())
        m_editor->insertHtml(QStringLiteral("<img src=\"") + path + QStringLiteral("\"/>"));
}

void RichTextEditorToolBar::layoutDirectionChanged()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextBlock block = cursor.block();
    if (block.isValid()) {
        QTextBlockFormat format = block.blockFormat();
        const Qt::LayoutDirection newDirection = m_layoutDirectionAction->isChecked() ? Qt::RightToLeft : Qt::LeftToRight;
        if (format.layoutDirection() != newDirection) {
            format.setLayoutDirection(newDirection);
            cursor.setBlockFormat(format);
        }
    }
}

void RichTextEditorToolBar::updateActions()
{
    if (m_editor == nullptr) {
        setEnabled(false);
        return;
    }

    const Qt::Alignment alignment = m_editor->alignment();
    const QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat charFormat = cursor.charFormat();
    const QFont font = charFormat.font();
    const QTextCharFormat::VerticalAlignment valign =
        charFormat.verticalAlignment();
    const bool superScript = valign == QTextCharFormat::AlignSuperScript;
    const bool subScript = valign == QTextCharFormat::AlignSubScript;

    if (alignment & Qt::AlignLeft) {
        m_align_left_action->setChecked(true);
    } else if (alignment & Qt::AlignRight) {
        m_align_right_action->setChecked(true);
    } else if (alignment & Qt::AlignHCenter) {
        m_align_center_action->setChecked(true);
    } else {
        m_align_justify_action->setChecked(true);
    }
    m_layoutDirectionAction->setChecked(cursor.blockFormat().layoutDirection() == Qt::RightToLeft);

    m_bold_action->setChecked(font.bold());
    m_italic_action->setChecked(font.italic());
    m_underline_action->setChecked(font.underline());
    m_valign_sup_action->setChecked(superScript);
    m_valign_sub_action->setChecked(subScript);

    const int size = font.pointSize();
    const int idx = m_font_size_input->findText(QString::number(size));
    if (idx != -1)
        m_font_size_input->setCurrentIndex(idx);

    m_color_action->setColor(m_editor->textColor());
}

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent), m_simplifyRichText(simplifyRichTextDefault)
{
    connect(this, &RichTextEditor::currentCharFormatChanged,
            this, &RichTextEditor::stateChanged);
    connect(this, &RichTextEditor::cursorPositionChanged,
            this, &RichTextEditor::stateChanged);
}

QToolBar *RichTextEditor::createToolBar(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new RichTextEditorToolBar(core, this, parent);
}

void RichTextEditor::setFontBold(bool b)
{
    if (b)
        setFontWeight(QFont::Bold);
    else
        setFontWeight(QFont::Normal);
}

void RichTextEditor::setFontPointSize(double d)
{
    QTextEdit::setFontPointSize(qreal(d));
}

void RichTextEditor::setText(const QString &text)
{

    if (Qt::mightBeRichText(text))
        setHtml(text);
    else
        setPlainText(text);
}

void RichTextEditor::setSimplifyRichText(bool v)
{
    if (v != m_simplifyRichText) {
        m_simplifyRichText = v;
        emit simplifyRichTextChanged(v);
    }
}

void RichTextEditor::setDefaultFont(QFont font)
{
    // Some default fonts on Windows have a default size of 7.8,
    // which results in complicated rich text generated by toHtml().
    // Use an integer value.
    const int pointSize = qRound(font.pointSizeF());
    if (pointSize > 0 && !qFuzzyCompare(qreal(pointSize), font.pointSizeF())) {
        font.setPointSize(pointSize);
    }

    document()->setDefaultFont(font);
    if (font.pointSize() > 0)
        setFontPointSize(font.pointSize());
    else
        setFontPointSize(QFontInfo(font).pointSize());
    emit textChanged();
}

QString RichTextEditor::text(Qt::TextFormat format) const
{
    switch (format) {
    case Qt::PlainText:
        return toPlainText();
    case Qt::RichText:
        return m_simplifyRichText ? simplifyRichTextFilter(toHtml()) : toHtml();
    default:
        break;
    }
    const QString html = toHtml();
    bool isPlainText;
    const QString simplifiedHtml = simplifyRichTextFilter(html, &isPlainText);
    if (isPlainText)
        return toPlainText();
    return m_simplifyRichText ? simplifiedHtml : html;
}

RichTextEditorDialog::RichTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
    QDialog(parent),
    m_editor(new RichTextEditor()),
    m_text_edit(new HtmlTextEdit),
    m_tab_widget(new QTabWidget),
    m_state(Clean),
    m_core(core),
    m_initialTab(RichTextIndex)
{
    setWindowTitle(tr("Edit text"));

    // Read settings
    const QDesignerSettingsInterface *settings = core->settingsManager();
    const QString rootKey = QLatin1StringView(RichTextDialogGroupC) + u'/';
    const QByteArray lastGeometry = settings->value(rootKey + QLatin1StringView(GeometryKeyC)).toByteArray();
    const int initialTab = settings->value(rootKey + QLatin1StringView(TabKeyC), QVariant(m_initialTab)).toInt();
    if (initialTab == RichTextIndex || initialTab == SourceIndex)
        m_initialTab = initialTab;

    m_text_edit->setAcceptRichText(false);
    new HtmlHighlighter(m_text_edit);

    connect(m_editor, &QTextEdit::textChanged, this, &RichTextEditorDialog::richTextChanged);
    connect(m_editor, &RichTextEditor::simplifyRichTextChanged,
            this, &RichTextEditorDialog::richTextChanged);
    connect(m_text_edit, &QTextEdit::textChanged, this, &RichTextEditorDialog::sourceChanged);

    // The toolbar needs to be created after the RichTextEditor
    QToolBar *tool_bar = m_editor->createToolBar(core);
    tool_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QWidget *rich_edit = new QWidget;
    QVBoxLayout *rich_edit_layout = new QVBoxLayout(rich_edit);
    rich_edit_layout->addWidget(tool_bar);
    rich_edit_layout->addWidget(m_editor);

    QWidget *plain_edit = new QWidget;
    QVBoxLayout *plain_edit_layout = new QVBoxLayout(plain_edit);
    plain_edit_layout->addWidget(m_text_edit);

    m_tab_widget->setTabPosition(QTabWidget::South);
    m_tab_widget->addTab(rich_edit, tr("Rich Text"));
    m_tab_widget->addTab(plain_edit, tr("Source"));
    connect(m_tab_widget, &QTabWidget::currentChanged,
            this, &RichTextEditorDialog::tabIndexChanged);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setText(tr("&OK"));
    ok_button->setDefault(true);
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tab_widget);
    layout->addWidget(buttonBox);

    if (!lastGeometry.isEmpty())
        restoreGeometry(lastGeometry);
}

RichTextEditorDialog::~RichTextEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(QLatin1StringView(RichTextDialogGroupC));

    settings->setValue(QLatin1StringView(GeometryKeyC), saveGeometry());
    settings->setValue(QLatin1StringView(TabKeyC), m_tab_widget->currentIndex());
    settings->endGroup();
}

int RichTextEditorDialog::showDialog()
{
    m_tab_widget->setCurrentIndex(m_initialTab);
    switch (m_initialTab) {
    case RichTextIndex:
        m_editor->selectAll();
        m_editor->setFocus();
        break;
    case SourceIndex:
        m_text_edit->selectAll();
        m_text_edit->setFocus();
        break;
    }
    return exec();
}

void RichTextEditorDialog::setDefaultFont(const QFont &font)
{
    m_editor->setDefaultFont(font);
}

void RichTextEditorDialog::setText(const QString &text)
{
    // Generally simplify rich text unless verbose text is found.
    const bool isSimplifiedRichText = !text.startsWith(QStringLiteral("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"));
    m_editor->setSimplifyRichText(isSimplifiedRichText);
    m_editor->setText(text);
    m_text_edit->setPlainText(text);
    m_state = Clean;
}

QString RichTextEditorDialog::text(Qt::TextFormat format) const
{
    // In autotext mode, if the user has changed the source, use that
    if (format == Qt::AutoText && (m_state == Clean || m_state == SourceChanged))
        return m_text_edit->toPlainText();
    // If the plain text HTML editor is selected, first copy its contents over
    // to the rich text editor so that it is converted to Qt-HTML or actual
    // plain text.
    if (m_tab_widget->currentIndex() == SourceIndex && m_state == SourceChanged)
        m_editor->setHtml(m_text_edit->toPlainText());
    return m_editor->text(format);
}

void RichTextEditorDialog::tabIndexChanged(int newIndex)
{
    // Anything changed, is there a need for a conversion?
    if (newIndex == SourceIndex && m_state != RichTextChanged)
        return;
    if (newIndex == RichTextIndex && m_state != SourceChanged)
        return;
    const State oldState = m_state;
    // Remember the cursor position, since it is invalidated by setPlainText
    QTextEdit *new_edit = (newIndex == SourceIndex) ? m_text_edit : m_editor;
    const int position = new_edit->textCursor().position();

    if (newIndex == SourceIndex)
        m_text_edit->setPlainText(m_editor->text(Qt::RichText));
    else
        m_editor->setHtml(m_text_edit->toPlainText());

    QTextCursor cursor = new_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (cursor.position() > position) {
        cursor.setPosition(position);
    }
    new_edit->setTextCursor(cursor);
    m_state = oldState; // Changed is triggered by setting the text
}

void RichTextEditorDialog::richTextChanged()
{
    m_state = RichTextChanged;
}

void RichTextEditorDialog::sourceChanged()
{
    m_state = SourceChanged;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

#include "richtexteditor.moc"
