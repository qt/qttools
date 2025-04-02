// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGEEDITORWIDGETS_H
#define MESSAGEEDITORWIDGETS_H

#include <QAbstractButton>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QMap>
#include <QTextEdit>
#include <QUrl>
#include <QWidget>

QT_BEGIN_NAMESPACE

class QAction;
class QContextMenuEvent;
class QKeyEvent;
class QMenu;
class QSizeF;
class QString;
class QVariant;

class MessageHighlighter;

/*
  Automatically adapt height to document contents
 */
class ExpandingTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    ExpandingTextEdit(QWidget *parent = 0);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void updateHeight(const QSizeF &documentSize);
    void reallyEnsureCursorVisible();

private:
    int m_minimumHeight;
};

/*
  Format markup & control characters
*/
class FormatTextEdit : public ExpandingTextEdit
{
    Q_OBJECT
public:
    FormatTextEdit(QWidget *parent = 0);
    ~FormatTextEdit();
    void setEditable(bool editable);

signals:
    void editorDestroyed();

public slots:
    void setPlainText(const QString & text, bool userAction);
    void setVisualizeWhitespace(bool value);

private:
    bool event(QEvent *event) override;

    MessageHighlighter *m_highlighter = nullptr;
};

/*
  Displays text field & associated label
*/
class FormWidget : public QWidget
{
    Q_OBJECT
public:
    FormWidget(const QString &label, bool isEditable, QWidget *parent = 0);
    void setLabel(const QString &label) { m_label->setText(label); }
    void setTranslation(const QString &text, bool userAction = false);
    void clearTranslation() { setTranslation(QString(), false); }
    QString getTranslation() { return m_editor->toPlainText(); }
    void setEditingEnabled(bool enable);
    void setHideWhenEmpty(bool optional) { m_hideWhenEmpty = optional; }
    FormatTextEdit *getEditor() { return m_editor; }

signals:
    void textChanged(QTextEdit *);
    void selectionChanged(QTextEdit *);
    void cursorPositionChanged();

private slots:
    void slotSelectionChanged();
    void slotTextChanged();

private:
    QLabel *m_label;
    FormatTextEdit *m_editor;
    bool m_hideWhenEmpty;
};

/*
  Displays text fields & associated label
*/
class FormMultiWidget : public QWidget
{
    Q_OBJECT
public:
    FormMultiWidget(const QString &label, QWidget *parent = 0);
    void setLabel(const QString &label) { m_label->setText(label); }
    void setTranslation(const QString &text, bool userAction = false);
    void clearTranslation() { setTranslation(QString(), false); }
    QString getTranslation() const;
    void setEditingEnabled(bool enable);
    void setMultiEnabled(bool enable);
    void setHideWhenEmpty(bool optional) { m_hideWhenEmpty = optional; }
    const QList<FormatTextEdit *> &getEditors() const { return m_editors; }

signals:
    void editorCreated(QTextEdit *);
    void textChanged(QTextEdit *);
    void selectionChanged(QTextEdit *);
    void cursorPositionChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotTextChanged();
    void slotSelectionChanged();
    void minusButtonClicked();
    void plusButtonClicked();
    void updateIcons();
private:
    void addEditor(int idx);
    void updateLayout();

    template<typename Func>
    QAbstractButton *makeButton(const QIcon &icon, Func slot)
    {
        auto *button = makeButton(icon);
        connect(button, &QAbstractButton::clicked,
                this, slot);
        return button;
    }
    QAbstractButton *makeButton(const QIcon &icon);
    void insertEditor(int idx);
    void deleteEditor(int idx);

    QLabel *m_label;
    QList<FormatTextEdit *> m_editors;
    QList<QWidget *> m_plusButtons;
    QList<QAbstractButton *> m_minusButtons;
    bool m_hideWhenEmpty;
    bool m_multiEnabled;
    QIcon m_plusIcon, m_minusIcon;
};

QT_END_NAMESPACE

#endif // MESSAGEEDITORWIDGETS_H
