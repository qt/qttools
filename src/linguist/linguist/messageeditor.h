// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGEEDITOR_H
#define MESSAGEEDITOR_H

#include "messagemodel.h"

#include <QtCore/QLocale>
#include <QtCore/QTimer>

#include <QtWidgets/QFrame>
#include <QtWidgets/QScrollArea>

QT_BEGIN_NAMESPACE

class QBoxLayout;
class QMainWindow;
class QTextEdit;

class MessageEditor;
class FormatTextEdit;
class FormWidget;
class FormMultiWidget;

struct MessageEditorData {
    QWidget *container;
    FormWidget *transCommentText;
    QList<FormMultiWidget *> transTexts;
    QString invariantForm;
    QString firstForm;
    qreal fontSize;
    bool pluralEditMode;
};

class MessageEditor : public QScrollArea
{
    Q_OBJECT

public:
    MessageEditor(MultiDataModel *dataModel, QMainWindow *parent = 0);
    ~MessageEditor();

    void showNothing();
    void showMessage(const MultiDataIndex &index);
    void setNumerusForms(int model, const QStringList &numerusForms);
    bool eventFilter(QObject *, QEvent *) override;
    void setNumerusTranslation(int model, const QString &translation, int numerus);
    int activeModel() const { return (m_editors.size() != 1) ? m_currentModel : 0; }
    void setEditorFocusForModel(int model);
    void setUnfinishedEditorFocus();
    bool focusNextUnfinished();
    void setVisualizeWhitespace(bool value);
    void setFontSize(const float fontSize);
    float fontSize();

signals:
    void translationChanged(const QStringList &translations);
    void translatorCommentChanged(const QString &comment);
    void activeModelChanged(int model);

    void undoAvailable(bool avail);
    void redoAvailable(bool avail);
#ifndef QT_NO_CLIPBOARD
    void cutAvailable(bool avail);
    void copyAvailable(bool avail);
    void pasteAvailable(bool avail);
#endif
    void beginFromSourceAvailable(bool enable);

public slots:
    void undo();
    void redo();
#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif
    void selectAll();
    void beginFromSource();
    void setEditorFocus();
    void setTranslation(int latestModel, const QString &translation);
    void setLengthVariants(bool on);
    void increaseFontSize();
    void decreaseFontSize();
    void resetFontSize();

private slots:
    void editorCreated(QTextEdit *);
    void editorDestroyed();
    void selectionChanged(QTextEdit *);
    void resetHoverSelection();
    void emitTranslationChanged(QTextEdit *);
    void emitTranslatorCommentChanged(QTextEdit *);
#ifndef QT_NO_CLIPBOARD
    void updateCanPaste();
    void clipboardChanged();
#endif
    void messageModelAppended();
    void messageModelDeleted(int model);
    void allModelsDeleted();
    void setTargetLanguage(int model);
    void reallyFixTabOrder();

private:
    void setupEditorPage();
    void setEditingEnabled(int model, bool enabled);
    bool focusNextUnfinished(int start);
    void resetSelection();
    void grabFocus(QWidget *widget);
    void trackFocus(QWidget *widget);
    void activeModelAndNumerus(int *model, int *numerus) const;
    QTextEdit *activeTranslation() const;
    QTextEdit *activeOr1stTranslation() const;
    QTextEdit *activeTransComment() const;
    QTextEdit *activeEditor() const;
    QTextEdit *activeOr1stEditor() const;
    MessageEditorData *modelForWidget(const QObject *o);
    int activeTranslationNumerus() const;
    QStringList translations(int model) const;
    void updateBeginFromSource();
    void updateUndoRedo();
#ifndef QT_NO_CLIPBOARD
    void updateCanCutCopy();
#endif
    void addPluralForm(int model, const QString &label, bool writable);
    void fixTabOrder();
    QPalette paletteForModel(int model) const;
    void applyFontSize();

    MultiDataModel *m_dataModel;

    MultiDataIndex m_currentIndex;
    int m_currentModel;
    int m_currentNumerus;

    bool m_lengthVariants;
    float m_fontSize;

    bool m_undoAvail;
    bool m_redoAvail;
    bool m_cutAvail;
    bool m_copyAvail;

    bool m_clipboardEmpty;
    bool m_visualizeWhitespace;

    QTextEdit *m_selectionHolder;
    QWidget *m_focusWidget;
    QBoxLayout *m_layout;
    FormWidget *m_source;
    FormWidget *m_pluralSource;
    FormWidget *m_commentText;
    QList<MessageEditorData> m_editors;

    QTimer m_tabOrderTimer;
};

QT_END_NAMESPACE

#endif // MESSAGEEDITOR_H
