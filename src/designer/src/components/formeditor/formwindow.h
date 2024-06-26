// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "formeditor_global.h"
#include <formwindowbase_p.h>

// Qt
#include <QtGui/qundostack.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qset.h>
#include <QtCore/qpointer.h>

#include <utility>

QT_BEGIN_NAMESPACE

class QDesignerDnDItemInterface;
class QDesignerTaskMenuExtension;
class DomConnections;
class DomUI;

class QWidget;
class QAction;
class QLabel;
class QTimer;
class QAction;
class QMenu;
class QRubberBand;

namespace qdesigner_internal {

class FormEditor;
class FormWindowCursor;
class WidgetEditorTool;
class FormWindowWidgetStack;
class FormWindowManager;
class FormWindowDnDItem;
class SetPropertyCommand;

class QT_FORMEDITOR_EXPORT FormWindow: public FormWindowBase
{
    Q_OBJECT

public:
    enum HandleOperation
    {
        NoHandleOperation,
        ResizeHandleOperation,
        ChangeLayoutSpanHandleOperation
    };

    explicit FormWindow(FormEditor *core, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~FormWindow() override;

    QDesignerFormEditorInterface *core() const override;

    QDesignerFormWindowCursorInterface *cursor() const override;

    // Overwritten: FormWindowBase
    QWidget *formContainer() const override;

    int toolCount() const override;
    int currentTool() const override;
    void setCurrentTool(int index) override;
    QDesignerFormWindowToolInterface *tool(int index) const override;
    void registerTool(QDesignerFormWindowToolInterface *tool) override;

    QString author() const override;
    void setAuthor(const QString &author) override;

    QString comment() const override;
    void setComment(const QString &comment) override;

    void layoutDefault(int *margin, int *spacing) override;
    void setLayoutDefault(int margin, int spacing) override;

    void layoutFunction(QString *margin, QString *spacing) override;
    void setLayoutFunction(const QString &margin, const QString &spacing) override;

    QString pixmapFunction() const override;
    void setPixmapFunction(const QString &pixmapFunction) override;

    QString exportMacro() const override;
    void setExportMacro(const QString &exportMacro) override;

    QStringList includeHints() const override;
    void setIncludeHints(const QStringList &includeHints) override;

    QString fileName() const override;
    void setFileName(const QString &fileName) override;

    QString contents() const override;
    bool setContents(QIODevice *dev, QString *errorMessage = nullptr) override;
    bool setContents(const QString &) override;

    QDir absoluteDir() const override;

    void simplifySelection(QWidgetList *sel) const override;

    void ensureUniqueObjectName(QObject *object) override;

    QWidget *mainContainer() const override;
    void setMainContainer(QWidget *mainContainer) override;
    bool isMainContainer(const QWidget *w) const;

    QWidget *currentWidget() const;

    bool hasInsertedChildren(QWidget *w) const;

    QWidgetList selectedWidgets() const;
    void clearSelection(bool changePropertyDisplay = true) override;
    bool isWidgetSelected(QWidget *w) const;
    void selectWidget(QWidget *w, bool select = true) override;

    void selectWidgets();
    void repaintSelection();
    void updateSelection(QWidget *w);
    void updateChildSelections(QWidget *w);
    void raiseChildSelections(QWidget *w);
    void raiseSelection(QWidget *w);

    inline const QWidgetList& widgets() const { return m_widgets; }
    inline int widgetCount() const { return m_widgets.size(); }
    inline QWidget *widgetAt(int index) const { return m_widgets.at(index); }

    QWidgetList widgets(QWidget *widget) const;

    QWidget *createWidget(DomUI *ui, QRect rect, QWidget *target);

    bool isManaged(QWidget *w) const override;

    void manageWidget(QWidget *w) override;
    void unmanageWidget(QWidget *w) override;

    QUndoStack *commandHistory() const override;
    void beginCommand(const QString &description) override;
    void endCommand() override;

    bool blockSelectionChanged(bool blocked) override;
    void emitSelectionChanged() override;

    bool unify(QObject *w, QString &s, bool changeIt);

    bool isDirty() const override;
    void setDirty(bool dirty) override;

    static FormWindow *findFormWindow(QWidget *w);

    virtual QWidget *containerAt(const QPoint &pos);
    QWidget *widgetAt(const QPoint &pos) override;
    void highlightWidget(QWidget *w, const QPoint &pos, HighlightMode mode = Highlight) override;

    void updateOrderIndicators();

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    QStringList resourceFiles() const override;
    void addResourceFile(const QString &path) override;
    void removeResourceFile(const QString &path) override;

    void resizeWidget(QWidget *widget, QRect geometry);

    bool dropDockWidget(QDesignerDnDItemInterface *item, QPoint global_mouse_pos);
    bool dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, QWidget *target,
                     const QPoint &global_mouse_pos) override;

    QWidget *findContainer(QWidget *w, bool excludeLayout) const override;
    // for WidgetSelection only.
    QWidget *designerWidget(QWidget *w) const;

    //  Initialize and return a popup menu for a managed widget
    QMenu *initializePopupMenu(QWidget *managedWidget) override;

#if QT_CONFIG(clipboard)
    void paste(PasteMode pasteMode) override;
#endif
    QEditorFormBuilder *createFormBuilder() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    HandleOperation handleOperation() const { return m_handleOperation; }
    void setHandleOperation(HandleOperation o) { m_handleOperation = o; }

signals:
    void contextMenuRequested(QMenu *menu, QWidget *widget);

public slots:
    void deleteWidgets();
    void raiseWidgets();
    void lowerWidgets();
#if QT_CONFIG(clipboard)
    void copy();
    void cut();
    void paste();
#endif
    void selectAll();

    void createLayout(int type, QWidget *container = nullptr);
    void morphLayout(QWidget *container, int newType);
    void breakLayout(QWidget *w);

    void editContents();

protected:
    virtual QMenu *createPopupMenu(QWidget *w);
    void resizeEvent(QResizeEvent *e) override;

    void insertWidget(QWidget *w, QRect rect, QWidget *target, bool already_in_form = false);

private slots:
    void selectionChangedTimerDone();
    void checkSelection();
    void checkSelectionNow();
    void slotSelectWidget(QAction *);
    void slotCleanChanged(bool);

private:
    enum MouseState {
        NoMouseState,
        // Double click received
        MouseDoubleClicked,
        // Drawing selection rubber band rectangle
        MouseDrawRubber,
        // Started a move operation
        MouseMoveDrag,
        // Click on a widget whose parent is selected. Defer selection to release
        MouseDeferredSelection
    };
    MouseState m_mouseState;
    QPointer<QWidget> m_lastClickedWidget;

    void init();
    void initializeCoreTools();

    int getValue(QRect rect, int key, bool size) const;
    int calcValue(int val, bool forward, bool snap, int snapOffset) const;
    void handleClickSelection(QWidget *managedWidget, unsigned mouseFlags);

    bool frameNeeded(QWidget *w) const;

    enum RectType { Insert, Rubber };

    void startRectDraw(QPoint global, QWidget *, RectType t);
    void continueRectDraw(QPoint global, QWidget *, RectType t);
    void endRectDraw();

    QWidget *containerAt(QPoint pos, QWidget *notParentOf);

    void checkPreviewGeometry(QRect &r);

    bool handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e);
    bool handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
    bool handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);

    bool isCentralWidget(QWidget *w) const;

    bool setCurrentWidget(QWidget *currentWidget);
    bool trySelectWidget(QWidget *w, bool select);

    void dragWidgetWithinForm(QWidget *widget, QRect targetGeometry, QWidget *targetContainer);

    void setCursorToAll(const QCursor &c, QWidget *start);

    QPoint mapToForm(const QWidget *w, QPoint pos) const;
    bool canBeBuddy(QWidget *w) const;

    QWidget *findTargetContainer(QWidget *widget) const;

    void clearMainContainer();

    static int widgetDepth(const QWidget *w);
    static bool isChildOf(const QWidget *c, const QWidget *p);

    void editWidgets() override;

    void updateWidgets();

    void handleArrowKeyEvent(int key, Qt::KeyboardModifiers modifiers);

    void layoutSelection(int type);
    void layoutContainer(QWidget *w, int type);

private:
    QWidget *innerContainer(QWidget *outerContainer) const;
    QWidget *containerForPaste() const;
    QAction *createSelectAncestorSubMenu(QWidget *w);
    void selectSingleWidget(QWidget *w);

    FormEditor *m_core;
    FormWindowCursor *m_cursor;
    QWidget *m_mainContainer = nullptr;
    QWidget *m_currentWidget = nullptr;

    bool m_blockSelectionChanged = false;

    QPoint m_rectAnchor;
    QRect m_currRect;

    QWidgetList m_widgets;
    QSet<QWidget*> m_insertedWidgets;

    class Selection;
    Selection *m_selection;

    QPoint m_startPos;

    QUndoStack m_undoStack;

    QString m_fileName;

    using PaletteAndFill = std::pair<QPalette, bool>;
    QHash<QWidget *, PaletteAndFill> m_palettesBeforeHighlight;

    QRubberBand *m_rubberBand = nullptr;

    QTimer *m_selectionChangedTimer = nullptr;
    QTimer *m_checkSelectionTimer = nullptr;
    QTimer *m_geometryChangedTimer = nullptr;

    FormWindowWidgetStack *m_widgetStack;
    WidgetEditorTool *m_widgetEditor = nullptr;

    QStringList m_resourceFiles;

    QString m_comment;
    QString m_author;
    QString m_pixmapFunction;
    int m_defaultMargin, m_defaultSpacing;
    QString m_marginFunction, m_spacingFunction;
    QString m_exportMacro;
    QStringList m_includeHints;

    QPoint m_contextMenuPosition;
    HandleOperation m_handleOperation = NoHandleOperation;

private:
    friend class WidgetEditorTool;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOW_H
