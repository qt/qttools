// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qteditorfactory_p.h"
#include "qtpropertybrowserutils_p.h"

#include <QtWidgets/qabstractitemview.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qdatetimeedit.h>
#include <QtWidgets/qfontdialog.h>
#include <QtWidgets/qkeysequenceedit.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayoutitem.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qtoolbutton.h>

#include <QtGui/qevent.h>
#include <QtGui/qvalidator.h>

#include <QtCore/qhash.h>
#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

// Set a hard coded left margin to account for the indentation
// of the tree view icon when switching to an editor

static inline void setupTreeViewEditorMargin(QLayout *lt)
{
    enum { DecorationMargin = 4 };
    if (QApplication::layoutDirection() == Qt::LeftToRight)
        lt->setContentsMargins(DecorationMargin, 0, 0, 0);
    else
        lt->setContentsMargins(0, 0, DecorationMargin, 0);
}

// ---------- EditorFactoryPrivate :
// Base class for editor factory private classes. Manages mapping of properties to editors and vice versa.

template <class Editor>
class EditorFactoryPrivate
{
public:

    using EditorList = QList<Editor *>;
    using PropertyToEditorListMap = QHash<QtProperty *, EditorList>;
    using EditorToPropertyMap = QHash<Editor *, QtProperty *>;

    Editor *createEditor(QtProperty *property, QWidget *parent);
    void initializeEditor(QtProperty *property, Editor *e);
    void slotEditorDestroyed(QObject *object);

    void deleteEditors();

    PropertyToEditorListMap  m_createdEditors;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
    auto *editor = new Editor(parent);
    initializeEditor(property, editor);
    return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
    auto it = m_createdEditors.find(property);
    if (it == m_createdEditors.end())
        it = m_createdEditors.insert(property, EditorList());
    it.value().append(editor);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
    auto pred = [object] (const Editor *e) { return object == e; };
    for (auto it = m_createdEditors.begin(), end = m_createdEditors.end(); it != end; ++it) {
        auto &editorList = it.value();
        auto listIt = std::find_if(editorList.cbegin(), editorList.cend(), pred);
        if (listIt != editorList.cend()) {
            editorList.erase(listIt);
            if (editorList.isEmpty())
               m_createdEditors.erase(it) ;
            break;
        }
    }
}

template <class Editor>
void EditorFactoryPrivate<Editor>::deleteEditors()
{
    for (auto it = m_createdEditors.begin(), end = m_createdEditors.end(); it != end; ++it)
        qDeleteAll(it.value());
    m_createdEditors.clear();
}

// ------------ QtSpinBoxFactory

class QtSpinBoxFactoryPrivate : public EditorFactoryPrivate<QSpinBox>
{
    QtSpinBoxFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtSpinBoxFactory)
public:

    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSingleStepChanged(QtProperty *property, int step);
    void slotSetValue(QtProperty *property, int value);
};

void QtSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QSpinBox *editor : it.value()) {
        if (editor->value() != value) {
            editor->blockSignals(true);
            editor->setValue(value);
            editor->blockSignals(false);
        }
    }
}

void QtSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QSpinBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtSpinBoxFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QSpinBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setSingleStep(step);
        editor->blockSignals(false);
    }
}

void QtSpinBoxFactoryPrivate::slotSetValue(QtProperty *property, int value)
{
    if (QtIntPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtSpinBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtSpinBoxFactory class provides QSpinBox widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtSpinBoxFactory::QtSpinBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtSpinBoxFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtSpinBoxFactory::~QtSpinBoxFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSpinBoxFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, &QtIntPropertyManager::valueChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtIntPropertyManager::rangeChanged,
            this, [this](QtProperty *property, int min, int max)
            { d_ptr->slotRangeChanged(property, min, max); });
    connect(manager, &QtIntPropertyManager::singleStepChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotSingleStepChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtSpinBoxFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QSpinBox *editor = d_ptr->createEditor(property, parent);
    editor->setSingleStep(manager->singleStep(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    editor->setKeyboardTracking(false);

    connect(editor, &QSpinBox::valueChanged,
            this, [this, property](int value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSpinBoxFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, &QtIntPropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::rangeChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::singleStepChanged, this, nullptr);
}

// QtSliderFactory

class QtSliderFactoryPrivate : public EditorFactoryPrivate<QSlider>
{
    QtSliderFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtSliderFactory)
public:
    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSingleStepChanged(QtProperty *property, int step);
    void slotSetValue(QtProperty *property, int value);
};

void QtSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QSlider *editor : it.value()) {
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtSliderFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QSlider *editor : it.value()) {
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtSliderFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QSlider *editor : it.value()) {
        editor->blockSignals(true);
        editor->setSingleStep(step);
        editor->blockSignals(false);
    }
}

void QtSliderFactoryPrivate::slotSetValue(QtProperty *property, int value)
{
    if (QtIntPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtSliderFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtSliderFactory class provides QSlider widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtSliderFactory::QtSliderFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtSliderFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtSliderFactory::~QtSliderFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSliderFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, &QtIntPropertyManager::valueChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtIntPropertyManager::rangeChanged,
            this, [this](QtProperty *property, int min, int max)
            { d_ptr->slotRangeChanged(property, min, max); });
    connect(manager, &QtIntPropertyManager::singleStepChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotSingleStepChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtSliderFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    auto *editor = new QSlider(Qt::Horizontal, parent);
    d_ptr->initializeEditor(property, editor);
    editor->setSingleStep(manager->singleStep(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));

    connect(editor, &QSlider::valueChanged,
            this, [this, property](int value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSliderFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, &QtIntPropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::rangeChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::singleStepChanged, this, nullptr);
}

// QtSliderFactory

class QtScrollBarFactoryPrivate : public  EditorFactoryPrivate<QScrollBar>
{
    QtScrollBarFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtScrollBarFactory)
public:
    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSingleStepChanged(QtProperty *property, int step);
    void slotSetValue(QtProperty *property, int value);
};

void QtScrollBarFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    for (QScrollBar *editor : it.value()) {
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtScrollBarFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QScrollBar *editor : it.value()) {
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtScrollBarFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QScrollBar *editor : it.value()) {
        editor->blockSignals(true);
        editor->setSingleStep(step);
        editor->blockSignals(false);
    }
}

void QtScrollBarFactoryPrivate::slotSetValue(QtProperty *property, int value)
{
    if (QtIntPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtScrollBarFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtScrollBarFactory class provides QScrollBar widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtScrollBarFactory::QtScrollBarFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtScrollBarFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtScrollBarFactory::~QtScrollBarFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtScrollBarFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, &QtIntPropertyManager::valueChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtIntPropertyManager::rangeChanged,
            this, [this](QtProperty *property, int min, int max)
            { d_ptr->slotRangeChanged(property, min, max); });
    connect(manager, &QtIntPropertyManager::singleStepChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotSingleStepChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtScrollBarFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    auto *editor = new QScrollBar(Qt::Horizontal, parent);
    d_ptr->initializeEditor(property, editor);
    editor->setSingleStep(manager->singleStep(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    connect(editor, &QScrollBar::valueChanged,
            this, [this, property](int value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtScrollBarFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, &QtIntPropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::rangeChanged, this, nullptr);
    disconnect(manager, &QtIntPropertyManager::singleStepChanged, this, nullptr);
}

// QtCheckBoxFactory

class QtCheckBoxFactoryPrivate : public EditorFactoryPrivate<QtBoolEdit>
{
    QtCheckBoxFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtCheckBoxFactory)
public:
    void slotPropertyChanged(QtProperty *property, bool value);
    void slotSetValue(QtProperty *property, bool value);
};

void QtCheckBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, bool value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    for (QtBoolEdit *editor : it.value()) {
        editor->blockCheckBoxSignals(true);
        editor->setChecked(value);
        editor->blockCheckBoxSignals(false);
    }
}

void QtCheckBoxFactoryPrivate::slotSetValue(QtProperty *property, bool value)
{
    if (QtBoolPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtCheckBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtCheckBoxFactory class provides QCheckBox widgets for
    properties created by QtBoolPropertyManager objects.

    \sa QtAbstractEditorFactory, QtBoolPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCheckBoxFactory::QtCheckBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtBoolPropertyManager>(parent), d_ptr(new QtCheckBoxFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCheckBoxFactory::~QtCheckBoxFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCheckBoxFactory::connectPropertyManager(QtBoolPropertyManager *manager)
{
    connect(manager, &QtBoolPropertyManager::valueChanged,
            this, [this](QtProperty *property, bool value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCheckBoxFactory::createEditor(QtBoolPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QtBoolEdit *editor = d_ptr->createEditor(property, parent);
    editor->setChecked(manager->value(property));

    connect(editor, &QtBoolEdit::toggled,
            this, [this, property](bool value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCheckBoxFactory::disconnectPropertyManager(QtBoolPropertyManager *manager)
{
    disconnect(manager, &QtBoolPropertyManager::valueChanged, this, nullptr);
}

// QtDoubleSpinBoxFactory

class QtDoubleSpinBoxFactoryPrivate : public EditorFactoryPrivate<QDoubleSpinBox>
{
    QtDoubleSpinBoxFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtDoubleSpinBoxFactory)
public:

    void slotPropertyChanged(QtProperty *property, double value);
    void slotRangeChanged(QtProperty *property, double min, double max);
    void slotSingleStepChanged(QtProperty *property, double step);
    void slotDecimalsChanged(QtProperty *property, int prec);
    void slotSetValue(QtProperty *property, double value);
};

void QtDoubleSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;
    for (QDoubleSpinBox *editor : it.value()) {
        if (editor->value() != value) {
            editor->blockSignals(true);
            editor->setValue(value);
            editor->blockSignals(false);
        }
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property,
            double min, double max)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QDoubleSpinBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotSingleStepChanged(QtProperty *property, double step)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.cend())
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QDoubleSpinBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setSingleStep(step);
        editor->blockSignals(false);
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QDoubleSpinBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setDecimals(prec);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotSetValue(QtProperty *property, double value)
{
    if (QtDoublePropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*! \class QtDoubleSpinBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtDoubleSpinBoxFactory class provides QDoubleSpinBox
    widgets for properties created by QtDoublePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDoublePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDoubleSpinBoxFactory::QtDoubleSpinBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDoublePropertyManager>(parent), d_ptr(new QtDoubleSpinBoxFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDoubleSpinBoxFactory::~QtDoubleSpinBoxFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDoubleSpinBoxFactory::connectPropertyManager(QtDoublePropertyManager *manager)
{
    connect(manager, &QtDoublePropertyManager::valueChanged,
            this, [this](QtProperty *property, double value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtDoublePropertyManager::rangeChanged,
            this, [this](QtProperty *property, double min, double max)
            { d_ptr->slotRangeChanged(property, min, max); });
    connect(manager, &QtDoublePropertyManager::singleStepChanged,
            this, [this](QtProperty *property, double value)
            { d_ptr->slotSingleStepChanged(property, value); });
    connect(manager, &QtDoublePropertyManager::decimalsChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotDecimalsChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDoubleSpinBoxFactory::createEditor(QtDoublePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QDoubleSpinBox *editor = d_ptr->createEditor(property, parent);
    editor->setSingleStep(manager->singleStep(property));
    editor->setDecimals(manager->decimals(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    editor->setKeyboardTracking(false);

    connect(editor, &QDoubleSpinBox::valueChanged,
            this, [this, property](double value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDoubleSpinBoxFactory::disconnectPropertyManager(QtDoublePropertyManager *manager)
{
    disconnect(manager, &QtDoublePropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtDoublePropertyManager::rangeChanged, this, nullptr);
    disconnect(manager, &QtDoublePropertyManager::singleStepChanged, this, nullptr);
    disconnect(manager, &QtDoublePropertyManager::decimalsChanged, this, nullptr);
}

// QtLineEditFactory

class QtLineEditFactoryPrivate : public EditorFactoryPrivate<QLineEdit>
{
    QtLineEditFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtLineEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QString &value);
    void slotRegExpChanged(QtProperty *property, const QRegularExpression &regExp);
    void slotSetValue(QtProperty *property, const QString &value);
};

void QtLineEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
                const QString &value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QLineEdit *editor : it.value()) {
        if (editor->text() != value)
            editor->setText(value);
    }
}

void QtLineEditFactoryPrivate::slotRegExpChanged(QtProperty *property,
            const QRegularExpression &regExp)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    QtStringPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QLineEdit *editor : it.value()) {
        editor->blockSignals(true);
        const QValidator *oldValidator = editor->validator();
        QValidator *newValidator = nullptr;
        if (regExp.isValid()) {
            newValidator = new QRegularExpressionValidator(regExp, editor);
        }
        editor->setValidator(newValidator);
        delete oldValidator;
        editor->blockSignals(false);
    }
}

void QtLineEditFactoryPrivate::slotSetValue(QtProperty *property, const QString &value)
{
    if (QtStringPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtLineEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtLineEditFactory class provides QLineEdit widgets for
    properties created by QtStringPropertyManager objects.

    \sa QtAbstractEditorFactory, QtStringPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtLineEditFactory::QtLineEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtStringPropertyManager>(parent), d_ptr(new QtLineEditFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtLineEditFactory::~QtLineEditFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtLineEditFactory::connectPropertyManager(QtStringPropertyManager *manager)
{
    connect(manager, &QtStringPropertyManager::valueChanged,
            this, [this](QtProperty *property, const QString &value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtStringPropertyManager::regExpChanged,
            this, [this](QtProperty *property, const QRegularExpression &value)
            { d_ptr->slotRegExpChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtLineEditFactory::createEditor(QtStringPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QLineEdit *editor = d_ptr->createEditor(property, parent);
    QRegularExpression regExp = manager->regExp(property);
    if (regExp.isValid() && !regExp.pattern().isEmpty()) {
        auto *validator = new QRegularExpressionValidator(regExp, editor);
        editor->setValidator(validator);
    }
    editor->setText(manager->value(property));

    connect(editor, &QLineEdit::textEdited,
            this, [this, property](const QString &value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtLineEditFactory::disconnectPropertyManager(QtStringPropertyManager *manager)
{
    disconnect(manager, &QtStringPropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtStringPropertyManager::regExpChanged, this, nullptr);
}

// QtDateEditFactory

class QtDateEditFactoryPrivate : public EditorFactoryPrivate<QDateEdit>
{
    QtDateEditFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtDateEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, QDate value);
    void slotRangeChanged(QtProperty *property, QDate min, QDate max);
    void slotSetValue(QtProperty *property, QDate value);
};

void QtDateEditFactoryPrivate::slotPropertyChanged(QtProperty *property, QDate value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;
    for (QDateEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setDate(value);
        editor->blockSignals(false);
    }
}

void QtDateEditFactoryPrivate::slotRangeChanged(QtProperty *property, QDate min, QDate max)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    QtDatePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    for (QDateEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setDateRange(min, max);
        editor->setDate(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDateEditFactoryPrivate::slotSetValue(QtProperty *property, QDate value)
{
    if (QtDatePropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtDateEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtDateEditFactory class provides QDateEdit widgets for
    properties created by QtDatePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDatePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDateEditFactory::QtDateEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDatePropertyManager>(parent), d_ptr(new QtDateEditFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDateEditFactory::~QtDateEditFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateEditFactory::connectPropertyManager(QtDatePropertyManager *manager)
{
    connect(manager, &QtDatePropertyManager::valueChanged,
            this, [this](QtProperty *property, QDate value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtDatePropertyManager::rangeChanged,
            this, [this](QtProperty *property, QDate min, QDate max)
            { d_ptr->slotRangeChanged(property, min, max); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDateEditFactory::createEditor(QtDatePropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QDateEdit *editor = d_ptr->createEditor(property, parent);
    editor->setDisplayFormat(QtPropertyBrowserUtils::dateFormat());
    editor->setCalendarPopup(true);
    editor->setDateRange(manager->minimum(property), manager->maximum(property));
    editor->setDate(manager->value(property));

    connect(editor, &QDateEdit::dateChanged,
            this, [this, property](QDate value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateEditFactory::disconnectPropertyManager(QtDatePropertyManager *manager)
{
    disconnect(manager, &QtDatePropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtDatePropertyManager::rangeChanged, this, nullptr);
}

// QtTimeEditFactory

class QtTimeEditFactoryPrivate : public EditorFactoryPrivate<QTimeEdit>
{
    QtTimeEditFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtTimeEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, QTime value);
    void slotSetValue(QtProperty *property, QTime value);
};

void QtTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property, QTime value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;
    for (QTimeEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setTime(value);
        editor->blockSignals(false);
    }
}

void QtTimeEditFactoryPrivate::slotSetValue(QtProperty *property, QTime value)
{
    if (QtTimePropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtTimeEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtTimeEditFactory class provides QTimeEdit widgets for
    properties created by QtTimePropertyManager objects.

    \sa QtAbstractEditorFactory, QtTimePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtTimeEditFactory::QtTimeEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtTimePropertyManager>(parent), d_ptr(new QtTimeEditFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtTimeEditFactory::~QtTimeEditFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtTimeEditFactory::connectPropertyManager(QtTimePropertyManager *manager)
{
    connect(manager, &QtTimePropertyManager::valueChanged,
            this, [this](QtProperty *property, QTime value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtTimeEditFactory::createEditor(QtTimePropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QTimeEdit *editor = d_ptr->createEditor(property, parent);
    editor->setDisplayFormat(QtPropertyBrowserUtils::timeFormat());
    editor->setTime(manager->value(property));

    connect(editor, &QTimeEdit::timeChanged,
            this, [this, property](QTime value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtTimeEditFactory::disconnectPropertyManager(QtTimePropertyManager *manager)
{
    disconnect(manager, &QtTimePropertyManager::valueChanged, this, nullptr);
}

// QtDateTimeEditFactory

class QtDateTimeEditFactoryPrivate : public EditorFactoryPrivate<QDateTimeEdit>
{
    QtDateTimeEditFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtDateTimeEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QDateTime &value);
    void slotSetValue(QtProperty *property, const QDateTime &value);

};

void QtDateTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QDateTime &value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QDateTimeEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setDateTime(value);
        editor->blockSignals(false);
    }
}

void QtDateTimeEditFactoryPrivate::slotSetValue(QtProperty *property, const QDateTime &value)
{
    if (QtDateTimePropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtDateTimeEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtDateTimeEditFactory class provides QDateTimeEdit
    widgets for properties created by QtDateTimePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDateTimePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDateTimeEditFactory::QtDateTimeEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDateTimePropertyManager>(parent), d_ptr(new QtDateTimeEditFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDateTimeEditFactory::~QtDateTimeEditFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateTimeEditFactory::connectPropertyManager(QtDateTimePropertyManager *manager)
{
    connect(manager, &QtDateTimePropertyManager::valueChanged,
            this, [this](QtProperty *property, const QDateTime &value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDateTimeEditFactory::createEditor(QtDateTimePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QDateTimeEdit *editor =  d_ptr->createEditor(property, parent);
    editor->setDisplayFormat(QtPropertyBrowserUtils::dateTimeFormat());
    editor->setDateTime(manager->value(property));

    connect(editor, &QDateTimeEdit::dateTimeChanged,
            this, [this, property](const QDateTime &value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateTimeEditFactory::disconnectPropertyManager(QtDateTimePropertyManager *manager)
{
    disconnect(manager, &QtDateTimePropertyManager::valueChanged, this, nullptr);
}

// QtKeySequenceEditorFactory

class QtKeySequenceEditorFactoryPrivate : public EditorFactoryPrivate<QKeySequenceEdit>
{
    QtKeySequenceEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtKeySequenceEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QKeySequence &value);
    void slotSetValue(QtProperty *property, const QKeySequence &value);
};

void QtKeySequenceEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QKeySequence &value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QKeySequenceEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setKeySequence(value);
        editor->blockSignals(false);
    }
}

void QtKeySequenceEditorFactoryPrivate::slotSetValue(QtProperty *property, const QKeySequence &value)
{
    if (QtKeySequencePropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtKeySequenceEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtKeySequenceEditorFactory class provides editor
    widgets for properties created by QtKeySequencePropertyManager objects.

    \sa QtAbstractEditorFactory
*/

/*!
    Creates a factory with the given \a parent.
*/
QtKeySequenceEditorFactory::QtKeySequenceEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtKeySequencePropertyManager>(parent), d_ptr(new QtKeySequenceEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtKeySequenceEditorFactory::~QtKeySequenceEditorFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtKeySequenceEditorFactory::connectPropertyManager(QtKeySequencePropertyManager *manager)
{
    connect(manager, &QtKeySequencePropertyManager::valueChanged,
            this, [this](QtProperty *property, const QKeySequence &value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtKeySequenceEditorFactory::createEditor(QtKeySequencePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QKeySequenceEdit *editor = d_ptr->createEditor(property, parent);
    editor->setKeySequence(manager->value(property));

    connect(editor, &QKeySequenceEdit::keySequenceChanged,
            this, [this, property](const QKeySequence &value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtKeySequenceEditorFactory::disconnectPropertyManager(QtKeySequencePropertyManager *manager)
{
    disconnect(manager, &QtKeySequencePropertyManager::valueChanged, this, nullptr);
}

// QtCharEdit

class QtCharEdit : public QWidget
{
    Q_OBJECT
public:
    QtCharEdit(QWidget *parent = nullptr);

    QChar value() const;
    bool eventFilter(QObject *o, QEvent *e) override;
public Q_SLOTS:
    void setValue(const QChar &value);
Q_SIGNALS:
    void valueChanged(const QChar &value);
protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    bool event(QEvent *e) override;
private slots:
    void slotClearChar();
private:
    void handleKeyEvent(QKeyEvent *e);

    QChar m_value;
    QLineEdit *m_lineEdit;
};

QtCharEdit::QtCharEdit(QWidget *parent)
    : QWidget(parent),  m_lineEdit(new QLineEdit(this))
{
    auto *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->setContentsMargins(QMargins());
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setFocusProxy(this);
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
}

bool QtCharEdit::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_lineEdit && e->type() == QEvent::ContextMenu) {
        QContextMenuEvent *c = static_cast<QContextMenuEvent *>(e);
        QMenu *menu = m_lineEdit->createStandardContextMenu();
        const auto actions = menu->actions();
        for (QAction *action : actions) {
            action->setShortcut(QKeySequence());
            QString actionString = action->text();
            const auto pos = actionString.lastIndexOf(QLatin1Char('\t'));
            if (pos > 0)
                actionString = actionString.remove(pos, actionString.size() - pos);
            action->setText(actionString);
        }
        QAction *actionBefore = nullptr;
        if (!actions.empty())
            actionBefore = actions[0];
        auto *clearAction = new QAction(tr("Clear Char"), menu);
        menu->insertAction(actionBefore, clearAction);
        menu->insertSeparator(actionBefore);
        clearAction->setEnabled(!m_value.isNull());
        connect(clearAction, &QAction::triggered, this, &QtCharEdit::slotClearChar);
        menu->exec(c->globalPos());
        delete menu;
        e->accept();
        return true;
    }

    return QWidget::eventFilter(o, e);
}

void QtCharEdit::slotClearChar()
{
    if (m_value.isNull())
        return;
    setValue(QChar());
    emit valueChanged(m_value);
}

void QtCharEdit::handleKeyEvent(QKeyEvent *e)
{
    const int key = e->key();
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_Super_L:
    case Qt::Key_Return:
        return;
    default:
        break;
    }

    const QString text = e->text();
    if (text.size() != 1)
        return;

    const QChar c = text.at(0);
    if (!c.isPrint())
        return;

    if (m_value == c)
        return;

    m_value = c;
    const QString str = m_value.isNull() ? QString() : QString(m_value);
    m_lineEdit->setText(str);
    e->accept();
    emit valueChanged(m_value);
}

void QtCharEdit::setValue(const QChar &value)
{
    if (value == m_value)
        return;

    m_value = value;
    QString str = value.isNull() ? QString() : QString(value);
    m_lineEdit->setText(str);
}

QChar QtCharEdit::value() const
{
    return m_value;
}

void QtCharEdit::focusInEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    m_lineEdit->selectAll();
    QWidget::focusInEvent(e);
}

void QtCharEdit::focusOutEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void QtCharEdit::keyPressEvent(QKeyEvent *e)
{
    handleKeyEvent(e);
    e->accept();
}

void QtCharEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_lineEdit->event(e);
}

bool QtCharEdit::event(QEvent *e)
{
    switch(e->type()) {
    case QEvent::Shortcut:
    case QEvent::ShortcutOverride:
    case QEvent::KeyRelease:
        e->accept();
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

// QtCharEditorFactory

class QtCharEditorFactoryPrivate : public EditorFactoryPrivate<QtCharEdit>
{
    QtCharEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtCharEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QChar &value);
    void slotSetValue(QtProperty *property, const QChar &value);

};

void QtCharEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QChar &value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QtCharEdit *editor : it.value()) {
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtCharEditorFactoryPrivate::slotSetValue(QtProperty *property, const QChar &value)
{
    if (QtCharPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtCharEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtCharEditorFactory class provides editor
    widgets for properties created by QtCharPropertyManager objects.

    \sa QtAbstractEditorFactory
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCharEditorFactory::QtCharEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtCharPropertyManager>(parent), d_ptr(new QtCharEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCharEditorFactory::~QtCharEditorFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCharEditorFactory::connectPropertyManager(QtCharPropertyManager *manager)
{
    connect(manager, &QtCharPropertyManager::valueChanged,
            this, [this](QtProperty *property, const QChar &value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCharEditorFactory::createEditor(QtCharPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtCharEdit *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));

    connect(editor, &QtCharEdit::valueChanged,
            this, [this, property](const QChar &value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCharEditorFactory::disconnectPropertyManager(QtCharPropertyManager *manager)
{
    disconnect(manager, &QtCharPropertyManager::valueChanged, this, nullptr);
}

// QtEnumEditorFactory

class QtEnumEditorFactoryPrivate : public EditorFactoryPrivate<QComboBox>
{
    QtEnumEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtEnumEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, int value);
    void slotEnumNamesChanged(QtProperty *property, const QStringList &);
    void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &);
    void slotSetValue(QtProperty *property, int value);
};

void QtEnumEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QComboBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->setCurrentIndex(value);
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotEnumNamesChanged(QtProperty *property,
                const QStringList &enumNames)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QMap<int, QIcon> enumIcons = manager->enumIcons(property);

    for (QComboBox *editor : it.value()) {
        editor->blockSignals(true);
        editor->clear();
        editor->addItems(enumNames);
        const auto nameCount = enumNames.size();
        for (qsizetype i = 0; i < nameCount; i++)
            editor->setItemIcon(i, enumIcons.value(i));
        editor->setCurrentIndex(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotEnumIconsChanged(QtProperty *property,
                const QMap<int, QIcon> &enumIcons)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    const QStringList enumNames = manager->enumNames(property);
    for (QComboBox *editor : it.value()) {
        editor->blockSignals(true);
        const auto nameCount = enumNames.size();
        for (qsizetype i = 0; i < nameCount; i++)
            editor->setItemIcon(i, enumIcons.value(i));
        editor->setCurrentIndex(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotSetValue(QtProperty *property, int value)
{
    if (QtEnumPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtEnumEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtEnumEditorFactory class provides QComboBox widgets for
    properties created by QtEnumPropertyManager objects.

    \sa QtAbstractEditorFactory, QtEnumPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtEnumEditorFactory::QtEnumEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtEnumPropertyManager>(parent), d_ptr(new QtEnumEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtEnumEditorFactory::~QtEnumEditorFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtEnumEditorFactory::connectPropertyManager(QtEnumPropertyManager *manager)
{
    connect(manager, &QtEnumPropertyManager::valueChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotPropertyChanged(property, value); });
    connect(manager, &QtEnumPropertyManager::enumNamesChanged,
            this, [this](QtProperty *property, const QStringList &value)
            { d_ptr->slotEnumNamesChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtEnumEditorFactory::createEditor(QtEnumPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QComboBox *editor = d_ptr->createEditor(property, parent);
    editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    editor->view()->setTextElideMode(Qt::ElideRight);
    QStringList enumNames = manager->enumNames(property);
    editor->addItems(enumNames);
    QMap<int, QIcon> enumIcons = manager->enumIcons(property);
    const auto enumNamesCount = enumNames.size();
    for (qsizetype i = 0; i < enumNamesCount; i++)
        editor->setItemIcon(i, enumIcons.value(i));
    editor->setCurrentIndex(manager->value(property));

    connect(editor, &QComboBox::currentIndexChanged,
            this, [this, property](int value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtEnumEditorFactory::disconnectPropertyManager(QtEnumPropertyManager *manager)
{
    disconnect(manager, &QtEnumPropertyManager::valueChanged, this, nullptr);
    disconnect(manager, &QtEnumPropertyManager::enumNamesChanged, this, nullptr);
}

// QtCursorEditorFactory

class QtCursorEditorFactoryPrivate
{
    QtCursorEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtCursorEditorFactory)
public:
    void slotPropertyChanged(QtProperty *property, const QCursor &cursor);
    void slotEnumChanged(QtProperty *property, int value);
    void slotEditorDestroyed(QObject *object);

    QtEnumEditorFactory *m_enumEditorFactory = nullptr;
    QtEnumPropertyManager *m_enumPropertyManager = nullptr;

    QHash<QtProperty *, QtProperty *> m_propertyToEnum;
    QHash<QtProperty *, QtProperty *> m_enumToProperty;
    QHash<QtProperty *, QWidgetList > m_enumToEditors;
    QHash<QWidget *, QtProperty *> m_editorToEnum;
    bool m_updatingEnum = false;
};

void QtCursorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, const QCursor &cursor)
{
    // update enum property
    QtProperty *enumProp = m_propertyToEnum.value(property);
    if (!enumProp)
        return;

    m_updatingEnum = true;
    auto *cdb = QtCursorDatabase::instance();
    m_enumPropertyManager->setValue(enumProp, cdb->cursorToValue(cursor));
    m_updatingEnum = false;
}

void QtCursorEditorFactoryPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_updatingEnum)
        return;
    // update cursor property
    QtProperty *prop = m_enumToProperty.value(property);
    if (!prop)
        return;
    QtCursorPropertyManager *cursorManager = q_ptr->propertyManager(prop);
    if (!cursorManager)
        return;
#ifndef QT_NO_CURSOR
    auto *cdb = QtCursorDatabase::instance();
    cursorManager->setValue(prop, QCursor(cdb->valueToCursor(value)));
#endif
}

void QtCursorEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    // remove from m_editorToEnum map;
    // remove from m_enumToEditors map;
    // if m_enumToEditors doesn't contains more editors delete enum property;
    for (auto itEditor = m_editorToEnum.cbegin(), ecend = m_editorToEnum.cend(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QWidget *editor = itEditor.key();
            QtProperty *enumProp = itEditor.value();
            m_editorToEnum.remove(editor);
            m_enumToEditors[enumProp].removeAll(editor);
            if (m_enumToEditors[enumProp].isEmpty()) {
                m_enumToEditors.remove(enumProp);
                QtProperty *property = m_enumToProperty.value(enumProp);
                m_enumToProperty.remove(enumProp);
                m_propertyToEnum.remove(property);
                delete enumProp;
            }
            return;
        }
}

/*!
    \class QtCursorEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtCursorEditorFactory class provides QComboBox widgets for
    properties created by QtCursorPropertyManager objects.

    \sa QtAbstractEditorFactory, QtCursorPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCursorEditorFactory::QtCursorEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtCursorPropertyManager>(parent), d_ptr(new QtCursorEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_enumEditorFactory = new QtEnumEditorFactory(this);
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::valueChanged,
            this, [this](QtProperty *property, int value)
            { d_ptr->slotEnumChanged(property, value); });
    d_ptr->m_enumEditorFactory->addPropertyManager(d_ptr->m_enumPropertyManager);
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCursorEditorFactory::~QtCursorEditorFactory() = default;

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCursorEditorFactory::connectPropertyManager(QtCursorPropertyManager *manager)
{
    connect(manager, &QtCursorPropertyManager::valueChanged,
            this, [this](QtProperty *property, const QCursor &value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCursorEditorFactory::createEditor(QtCursorPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QtProperty *enumProp = d_ptr->m_propertyToEnum.value(property, nullptr);
    if (enumProp == nullptr) {
        enumProp = d_ptr->m_enumPropertyManager->addProperty(property->propertyName());
        auto *cdb = QtCursorDatabase::instance();
        d_ptr->m_enumPropertyManager->setEnumNames(enumProp, cdb->cursorShapeNames());
        d_ptr->m_enumPropertyManager->setEnumIcons(enumProp, cdb->cursorShapeIcons());
#ifndef QT_NO_CURSOR
        d_ptr->m_enumPropertyManager->setValue(enumProp, cdb->cursorToValue(manager->value(property)));
#endif
        d_ptr->m_propertyToEnum[property] = enumProp;
        d_ptr->m_enumToProperty[enumProp] = property;
    }
    QtAbstractEditorFactoryBase *af = d_ptr->m_enumEditorFactory;
    QWidget *editor = af->createEditor(enumProp, parent);
    d_ptr->m_enumToEditors[enumProp].append(editor);
    d_ptr->m_editorToEnum[editor] = enumProp;
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCursorEditorFactory::disconnectPropertyManager(QtCursorPropertyManager *manager)
{
    disconnect(manager, &QtCursorPropertyManager::valueChanged, this, nullptr);
}

// QtColorEditWidget

class QtColorEditWidget : public QWidget {
    Q_OBJECT

public:
    QtColorEditWidget(QWidget *parent);

    bool eventFilter(QObject *obj, QEvent *ev) override;

public Q_SLOTS:
    void setValue(QColor value);

private Q_SLOTS:
    void buttonClicked();

Q_SIGNALS:
    void valueChanged(const QColor &value);

private:
    void updateColor();

    QColor m_color;
    QLabel *m_pixmapLabel;
    QLabel *m_label;
    QToolButton *m_button;
};

QtColorEditWidget::QtColorEditWidget(QWidget *parent) :
    QWidget(parent),
    m_pixmapLabel(new QLabel),
    m_label(new QLabel),
    m_button(new QToolButton)
{
    auto *lt = new QHBoxLayout(this);
    setupTreeViewEditorMargin(lt);
    lt->setSpacing(0);
    lt->addWidget(m_pixmapLabel);
    lt->addWidget(m_label);
    lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    setFocusProxy(m_button);
    setFocusPolicy(m_button->focusPolicy());
    m_button->setText(tr("..."));
    m_button->installEventFilter(this);
    connect(m_button, &QAbstractButton::clicked, this, &QtColorEditWidget::buttonClicked);
    lt->addWidget(m_button);
    updateColor();
}

void QtColorEditWidget::setValue(QColor c)
{
    if (m_color != c) {
        m_color = c;
        updateColor();
    }
}

void QtColorEditWidget::updateColor()
{
    const auto pm =
        QtPropertyBrowserUtils::brushValuePixmap(QBrush(m_color),
                                                 QtPropertyBrowserUtils::itemViewIconSize,
                                                 devicePixelRatioF());
    m_pixmapLabel->setPixmap(pm);
    m_label->setText(QtPropertyBrowserUtils::colorValueText(m_color));
}

void QtColorEditWidget::buttonClicked()
{
    const QColor newColor = QColorDialog::getColor(m_color, this, QString(), QColorDialog::ShowAlphaChannel);
    if (newColor.isValid() && newColor != m_color) {
        setValue(newColor);
        emit valueChanged(m_color);
    }
}

bool QtColorEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_button) {
        switch (ev->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent*>(ev)->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->ignore();
                return true;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// QtColorEditorFactoryPrivate

class QtColorEditorFactoryPrivate : public EditorFactoryPrivate<QtColorEditWidget>
{
    QtColorEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtColorEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, QColor value);
    void slotSetValue(QtProperty *property, QColor value);
};

void QtColorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
                                                      QColor value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QtColorEditWidget *e : it.value())
        e->setValue(value);
}

void QtColorEditorFactoryPrivate::slotSetValue(QtProperty *property, QColor value)
{
    if (QtColorPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtColorEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtColorEditorFactory class provides color editing  for
    properties created by QtColorPropertyManager objects.

    \sa QtAbstractEditorFactory, QtColorPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtColorEditorFactory::QtColorEditorFactory(QObject *parent) :
    QtAbstractEditorFactory<QtColorPropertyManager>(parent),
    d_ptr(new QtColorEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtColorEditorFactory::~QtColorEditorFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::connectPropertyManager(QtColorPropertyManager *manager)
{
    connect(manager, &QtColorPropertyManager::valueChanged,
            this, [this](QtProperty *property, QColor value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtColorEditorFactory::createEditor(QtColorPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtColorEditWidget *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));
    connect(editor, &QtColorEditWidget::valueChanged,
            this, [this, property](QColor value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::disconnectPropertyManager(QtColorPropertyManager *manager)
{
    disconnect(manager, &QtColorPropertyManager::valueChanged, this, nullptr);
}

// QtFontEditWidget

class QtFontEditWidget : public QWidget {
    Q_OBJECT

public:
    QtFontEditWidget(QWidget *parent);

    bool eventFilter(QObject *obj, QEvent *ev) override;

public Q_SLOTS:
    void setValue(const QFont &value);

private Q_SLOTS:
    void buttonClicked();

Q_SIGNALS:
    void valueChanged(const QFont &value);

private:
    void updateFont();

    QFont m_font;
    QLabel *m_pixmapLabel;
    QLabel *m_label;
    QToolButton *m_button;
};

QtFontEditWidget::QtFontEditWidget(QWidget *parent) :
    QWidget(parent),
    m_pixmapLabel(new QLabel),
    m_label(new QLabel),
    m_button(new QToolButton)
{
    auto *lt = new QHBoxLayout(this);
    setupTreeViewEditorMargin(lt);
    lt->setSpacing(0);
    lt->addWidget(m_pixmapLabel);
    lt->addWidget(m_label);
    lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    setFocusProxy(m_button);
    setFocusPolicy(m_button->focusPolicy());
    m_button->setText(tr("..."));
    m_button->installEventFilter(this);
    connect(m_button, &QAbstractButton::clicked, this, &QtFontEditWidget::buttonClicked);
    lt->addWidget(m_button);
    updateFont();
}

void QtFontEditWidget::updateFont()
{
    const auto pm = QtPropertyBrowserUtils::fontValuePixmap(
            m_font, QtPropertyBrowserUtils::itemViewIconSize, devicePixelRatioF());
    m_pixmapLabel->setPixmap(pm);
    m_label->setText(QtPropertyBrowserUtils::fontValueText(m_font));
}

void QtFontEditWidget::setValue(const QFont &f)
{
    if (m_font != f) {
        m_font = f;
        updateFont();
    }
}

void QtFontEditWidget::buttonClicked()
{
    bool ok = false;
    QFont newFont = QFontDialog::getFont(&ok, m_font, this, tr("Select Font"));
    if (ok && newFont != m_font) {
        QFont f = m_font;
        // prevent mask for unchanged attributes, don't change other attributes (like kerning, etc...)
        if (m_font.family() != newFont.family())
            f.setFamily(newFont.family());
        if (m_font.pointSize() != newFont.pointSize())
            f.setPointSize(newFont.pointSize());
        if (m_font.bold() != newFont.bold())
            f.setBold(newFont.bold());
        if (m_font.italic() != newFont.italic())
            f.setItalic(newFont.italic());
        if (m_font.underline() != newFont.underline())
            f.setUnderline(newFont.underline());
        if (m_font.strikeOut() != newFont.strikeOut())
            f.setStrikeOut(newFont.strikeOut());
        setValue(f);
        emit valueChanged(m_font);
    }
}

bool QtFontEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_button) {
        switch (ev->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent*>(ev)->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->ignore();
                return true;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// QtFontEditorFactoryPrivate

class QtFontEditorFactoryPrivate : public EditorFactoryPrivate<QtFontEditWidget>
{
    QtFontEditorFactory *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtFontEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QFont &value);
    void slotSetValue(QtProperty *property, const QFont &value);
};

void QtFontEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
                const QFont &value)
{
    const auto it = m_createdEditors.constFind(property);
    if (it == m_createdEditors.constEnd())
        return;

    for (QtFontEditWidget *e : it.value())
        e->setValue(value);
}

void QtFontEditorFactoryPrivate::slotSetValue(QtProperty *property, const QFont &value)
{
    if (QtFontPropertyManager *manager = q_ptr->propertyManager(property))
        manager->setValue(property, value);
}

/*!
    \class QtFontEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtFontEditorFactory class provides font editing for
    properties created by QtFontPropertyManager objects.

    \sa QtAbstractEditorFactory, QtFontPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtFontEditorFactory::QtFontEditorFactory(QObject *parent) :
    QtAbstractEditorFactory<QtFontPropertyManager>(parent),
    d_ptr(new QtFontEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtFontEditorFactory::~QtFontEditorFactory()
{
    d_ptr->deleteEditors();
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtFontEditorFactory::connectPropertyManager(QtFontPropertyManager *manager)
{
    connect(manager, &QtFontPropertyManager::valueChanged,
            this, [this](QtProperty *property, const QFont &value)
            { d_ptr->slotPropertyChanged(property, value); });
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtFontEditorFactory::createEditor(QtFontPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtFontEditWidget *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));
    connect(editor, &QtFontEditWidget::valueChanged,
            this, [this, property](const QFont &value) { d_ptr->slotSetValue(property, value); });
    connect(editor, &QObject::destroyed,
            this, [this](QObject *object) { d_ptr->slotEditorDestroyed(object); });
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtFontEditorFactory::disconnectPropertyManager(QtFontPropertyManager *manager)
{
    disconnect(manager, &QtFontPropertyManager::valueChanged, this, nullptr);
}

QT_END_NAMESPACE

#include "moc_qteditorfactory_p.cpp"
#include "qteditorfactory.moc"
