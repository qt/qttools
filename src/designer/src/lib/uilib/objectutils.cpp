// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "objectutils_p.h"

#include <QtWidgets>
#ifdef QT_OPENGLWIDGETS_LIB
#include <QtOpenGLWidgets/qopenglwidget.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

template <typename ...T>
struct TypeList {};

using Layouts = TypeList<
        QGridLayout
      , QHBoxLayout
      , QStackedLayout
      , QVBoxLayout
#if QT_CONFIG(formlayout)
      , QFormLayout
#endif
    >;

using Widgets = TypeList<
        QWidget
      , QDialog
      , QDialogButtonBox
      , QFrame
      , QLabel
#if QT_CONFIG(abstractslider)
      , QAbstractSlider
#endif
#if QT_CONFIG(calendarwidget)
      , QCalendarWidget
#endif
#if QT_CONFIG(checkbox)
      , QCheckBox
#endif
#if QT_CONFIG(columnview)
      , QColumnView
#endif
#if QT_CONFIG(combobox)
      , QComboBox
#endif
#if QT_CONFIG(commandlinkbutton)
      , QCommandLinkButton
#endif
#if QT_CONFIG(datetimeedit)
      , QDateEdit
      , QDateTimeEdit
      , QTimeEdit
#endif
#if QT_CONFIG(dial)
      , QDial
#endif
#if QT_CONFIG(dockwidget)
      , QDockWidget
#endif
#if QT_CONFIG(fontcombobox)
      , QFontComboBox
#endif
#if QT_CONFIG(groupbox)
      , QGroupBox
#endif
#if QT_CONFIG(keysequenceedit)
      , QKeySequenceEdit
#endif
#if QT_CONFIG(lcdnumber)
      , QLCDNumber
#endif
#if QT_CONFIG(lineedit)
      , QLineEdit
#endif
#if QT_CONFIG(listview)
      , QListView
#endif
#if QT_CONFIG(listwidget)
      , QListWidget
#endif
#if QT_CONFIG(mainwindow)
      , QMainWindow
#endif
#if QT_CONFIG(mdiarea)
      , QMdiArea
#endif
#if QT_CONFIG(menu)
      , QMenu
#endif
#if QT_CONFIG(menubar)
      , QMenuBar
#endif
#if QT_CONFIG(progressbar)
      , QProgressBar
#endif
#if QT_CONFIG(pushbutton)
      , QPushButton
#endif
#if QT_CONFIG(radiobutton)
      , QRadioButton
#endif
#if QT_CONFIG(scrollarea)
      , QAbstractScrollArea
      , QScrollArea
#endif
#if QT_CONFIG(scrollbar)
      , QScrollBar
#endif
#if QT_CONFIG(slider)
      , QSlider
#endif
#if QT_CONFIG(spinbox)
      , QAbstractSpinBox
      , QDoubleSpinBox
      , QSpinBox
#endif
#if QT_CONFIG(splitter)
      , QSplitter
#endif
#if QT_CONFIG(stackedwidget)
      , QStackedWidget
#endif
#if QT_CONFIG(statusbar)
      , QStatusBar
#endif
#if QT_CONFIG(tableview)
      , QTableView
#endif
#if QT_CONFIG(tablewidget)
      , QTableWidget
#endif
#if QT_CONFIG(tabwidget)
      , QTabWidget
#endif
#if QT_CONFIG(textbrowser)
      , QTextBrowser
#endif
#if QT_CONFIG(textedit)
      , QPlainTextEdit
      , QTextEdit
#endif
#if QT_CONFIG(toolbar)
      , QToolBar
#endif
#if QT_CONFIG(toolbox)
      , QToolBox
#endif
#if QT_CONFIG(toolbutton)
      , QToolButton
#endif
#if QT_CONFIG(treeview)
      , QTreeView
#endif
#if QT_CONFIG(treewidget)
      , QTreeWidget
#endif
#if QT_CONFIG(undoview)
      , QUndoView
#endif
#if QT_CONFIG(wizard)
      , QWizard
      , QWizardPage
#endif
#ifdef QT_OPENGLWIDGETS_LIB
      , QOpenGLWidget
#endif
#if !defined(QT_NO_GRAPHICSVIEW)
      , QGraphicsView
#endif
    >;

template <typename ...T>
static QStringList objectNamesImpl(TypeList<T...>)
{
    QStringList result;
    // Fold expression: executes the lambda for each type in the list
    ( [&result] { result.append(QString::fromUtf8(T::staticMetaObject.className())); }() , ... );
    return result;
}

QStringList layoutNames()
{
    return objectNamesImpl(Layouts());
}

QStringList widgetNames()
{
    return objectNamesImpl(Widgets());
}

template <typename ...T>
static QList<const QMetaObject *> metaObjectsImpl(TypeList<T...>)
{
    QList<const QMetaObject *> result;
    // Fold expression: executes the lambda for each type in the list
    ( [&result] { result.append(&T::staticMetaObject); }() , ... );
    return result;
}

QList<const QMetaObject *> widgetMetaObjects()
{
    return metaObjectsImpl(Widgets());
}

template <typename Base, typename T>
static Base *createObject(const QString &className, QWidget *parent)
{
    return className == QLatin1StringView(T::staticMetaObject.className()) ? new T(parent) : nullptr;
}

template <typename Base, typename ...T>
static Base *createObjectImpl(const QString &className, QWidget *parent, TypeList<T...>)
{
    Base *result = nullptr;
    // Left fold over the comma operator
    // The expression inside the parenthesis runs for every type sequentially
    ( (result == nullptr ? (result = createObject<Base, T>(className, parent)) : nullptr), ... );
    return result;
}

QLayout *createLayoutInstance(const QString &className, QWidget *parent)
{
    return createObjectImpl<QLayout>(className, parent, Layouts());
}

QWidget *createWidgetInstance(const QString &className, QWidget *parent)
{
    return createObjectImpl<QWidget>(className, parent, Widgets());
}

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE
