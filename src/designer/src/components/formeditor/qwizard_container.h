// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QWIZARD_CONTAINER_H
#define QWIZARD_CONTAINER_H

#include <QtDesigner/container.h>

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>

#include <QtWidgets/qwizard.h>

QT_BEGIN_NAMESPACE

class QWizardPage;

namespace qdesigner_internal {

// Container for QWizard. Care must be taken to position
// the  QWizard at some valid page after removal/insertion
// as it is not used to having its pages ripped out.
class QWizardContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QWizardContainer(QWizard *widget, QObject *parent = nullptr);

    int count() const override;
    QWidget *widget(int index) const override;
    int currentIndex() const override;
    void setCurrentIndex(int index) override;
    bool canAddWidget() const override { return true; }
    void addWidget(QWidget *widget) override;
    void insertWidget(int index, QWidget *widget) override;
    bool canRemove(int) const override { return true; }
    void remove(int index) override;

private:
    QWizard *m_wizard;
};

// QWizardPagePropertySheet: Introduces a attribute string fake property
// "pageId" that allows for specifying enumeration values (uic only).
// This breaks the pattern of having a "currentSth" property for the
// container, but was deemed to make sense here since the Page has
// its own "title" properties.
class QWizardPagePropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
public:
    explicit QWizardPagePropertySheet(QWizardPage *object, QObject *parent = nullptr);

    bool reset(int index) override;

    static const char *pageIdProperty;

private:
    const int m_pageIdIndex;
};

// QWizardPropertySheet: Hides the "startId" property. It cannot be used
// as QWizard cannot handle setting it as a property before the actual
// page is added.

class QWizardPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
public:
    explicit QWizardPropertySheet(QWizard *object, QObject *parent = nullptr);
    bool isVisible(int index) const override;

private:
    const QString m_startId;
};

// Factories
using QWizardPropertySheetFactory = QDesignerPropertySheetFactory<QWizard, QWizardPropertySheet>;
using QWizardPagePropertySheetFactory = QDesignerPropertySheetFactory<QWizardPage, QWizardPagePropertySheet>;
using QWizardContainerFactory = ExtensionFactory<QDesignerContainerExtension,  QWizard,  QWizardContainer>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QWIZARD_CONTAINER_H
