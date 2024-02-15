// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "formwindowsettings.h"
#include "ui_formwindowsettings.h"

#include <formwindowbase_p.h>
#include <grid_p.h>

#include <QtWidgets/qstyle.h>

#include <QtCore/qcompare.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qdebug.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

// Data structure containing form dialog data providing comparison
struct FormWindowData {
    void fromFormWindow(FormWindowBase* fw);
    void applyToFormWindow(FormWindowBase* fw) const;

    bool layoutDefaultEnabled{false};
    int defaultMargin{0};
    int defaultSpacing{0};

    bool layoutFunctionsEnabled{false};
    QString marginFunction;
    QString spacingFunction;

    QString pixFunction;

    QString author;

    QStringList includeHints;

    bool hasFormGrid{false};
    Grid grid;
    bool idBasedTranslations{false};
    bool connectSlotsByName{true};

    friend bool comparesEqual(const FormWindowData &lhs,
                              const FormWindowData &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(FormWindowData)
};

QDebug operator<<(QDebug str, const  FormWindowData &d)
{
    str.nospace() << "LayoutDefault=" << d.layoutDefaultEnabled        << ',' << d.defaultMargin
        <<  ',' << d.defaultSpacing << " LayoutFunctions=" << d.layoutFunctionsEnabled << ','
        << d.marginFunction << ',' << d.spacingFunction << " PixFunction="
        << d.pixFunction << " Author=" << d.author << " Hints=" << d.includeHints
        << " Grid=" << d.hasFormGrid << d.grid.deltaX() << d.grid.deltaY()
        << " ID-based translations" << d.idBasedTranslations
        << " Connect slots by name" << d.connectSlotsByName
        << '\n';
    return str;
}

bool comparesEqual(const FormWindowData &lhs, const FormWindowData &rhs) noexcept
{
    return lhs.layoutDefaultEnabled   == rhs.layoutDefaultEnabled &&
           lhs.defaultMargin          == rhs.defaultMargin &&
           lhs.defaultSpacing         == rhs.defaultSpacing &&
           lhs.layoutFunctionsEnabled == rhs.layoutFunctionsEnabled &&
           lhs.marginFunction         == rhs.marginFunction &&
           lhs.spacingFunction        == rhs.spacingFunction &&
           lhs.pixFunction            == rhs.pixFunction  &&
           lhs.author                 == rhs.author &&
           lhs.includeHints           == rhs.includeHints &&
           lhs.hasFormGrid            == rhs.hasFormGrid &&
           lhs.grid                   == rhs.grid &&
           lhs.idBasedTranslations    == rhs.idBasedTranslations &&
           lhs.connectSlotsByName     == rhs.connectSlotsByName;
}

void FormWindowData::fromFormWindow(FormWindowBase* fw)
{
    defaultMargin =  defaultSpacing = INT_MIN;
    fw->layoutDefault(&defaultMargin, &defaultSpacing);

    auto container = fw->formContainer();
    QStyle *style = container->style();
    layoutDefaultEnabled = defaultMargin != INT_MIN || defaultSpacing != INT_MIN;
    if (defaultMargin == INT_MIN)
        defaultMargin = style->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, container);
    if (defaultSpacing == INT_MIN)
        defaultSpacing = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr);


    marginFunction.clear();
    spacingFunction.clear();
    fw->layoutFunction(&marginFunction, &spacingFunction);
    layoutFunctionsEnabled = !marginFunction.isEmpty() || !spacingFunction.isEmpty();

    pixFunction = fw->pixmapFunction();

    author = fw->author();

    includeHints = fw->includeHints();
    includeHints.removeAll(QString());

    hasFormGrid = fw->hasFormGrid();
    grid = hasFormGrid ? fw->designerGrid() : FormWindowBase::defaultDesignerGrid();
    idBasedTranslations = fw->useIdBasedTranslations();
    connectSlotsByName = fw->connectSlotsByName();
}

void FormWindowData::applyToFormWindow(FormWindowBase* fw) const
{
    fw->setAuthor(author);
    fw->setPixmapFunction(pixFunction);

    if (layoutDefaultEnabled) {
        fw->setLayoutDefault(defaultMargin, defaultSpacing);
    } else {
        fw->setLayoutDefault(INT_MIN, INT_MIN);
    }

    if (layoutFunctionsEnabled) {
        fw->setLayoutFunction(marginFunction, spacingFunction);
    } else {
        fw->setLayoutFunction(QString(), QString());
    }

    fw->setIncludeHints(includeHints);

    const bool hadFormGrid = fw->hasFormGrid();
    fw->setHasFormGrid(hasFormGrid);
    if (hasFormGrid || hadFormGrid != hasFormGrid)
        fw->setDesignerGrid(hasFormGrid ? grid : FormWindowBase::defaultDesignerGrid());
    fw->setUseIdBasedTranslations(idBasedTranslations);
    fw->setConnectSlotsByName(connectSlotsByName);
}

// -------------------------- FormWindowSettings

FormWindowSettings::FormWindowSettings(QDesignerFormWindowInterface *parent) :
    QDialog(parent),
    m_ui(new QT_PREPEND_NAMESPACE(Ui)::FormWindowSettings),
    m_formWindow(qobject_cast<FormWindowBase*>(parent)),
    m_oldData(new FormWindowData)
{
    Q_ASSERT(m_formWindow);

    m_ui->setupUi(this);
    m_ui->gridPanel->setCheckable(true);
    m_ui->gridPanel->setResetButtonVisible(false);

    QString deviceProfileName = m_formWindow->deviceProfileName();
    if (deviceProfileName.isEmpty())
        deviceProfileName = tr("None");
    m_ui->deviceProfileLabel->setText(tr("Device Profile: %1").arg(deviceProfileName));

    m_oldData->fromFormWindow(m_formWindow);
    setData(*m_oldData);
}

FormWindowSettings::~FormWindowSettings()
{
    delete m_oldData;
    delete m_ui;
}

FormWindowData FormWindowSettings::data() const
{
    FormWindowData rc;
    rc.author = m_ui->authorLineEdit->text();

    if (m_ui->pixmapFunctionGroupBox->isChecked()) {
        rc.pixFunction = m_ui->pixmapFunctionLineEdit->text();
    } else {
        rc.pixFunction.clear();
    }

    rc.layoutDefaultEnabled = m_ui->layoutDefaultGroupBox->isChecked();
    rc.defaultMargin = m_ui->defaultMarginSpinBox->value();
    rc.defaultSpacing = m_ui->defaultSpacingSpinBox->value();

    rc.layoutFunctionsEnabled = m_ui->layoutFunctionGroupBox->isChecked();
    rc.marginFunction = m_ui->marginFunctionLineEdit->text();
    rc.spacingFunction = m_ui->spacingFunctionLineEdit->text();

    const QString hints = m_ui->includeHintsTextEdit->toPlainText();
    if (!hints.isEmpty()) {
        rc.includeHints = hints.split(u'\n');
        // Purge out any lines consisting of blanks only
        const QRegularExpression blankLine(u"^\\s*$"_s);
        Q_ASSERT(blankLine.isValid());
        rc.includeHints.erase(std::remove_if(rc.includeHints.begin(), rc.includeHints.end(),
                                             [blankLine](const QString &hint){ return blankLine.match(hint).hasMatch(); }),
                              rc.includeHints.end());
    }

    rc.hasFormGrid = m_ui->gridPanel->isChecked();
    rc.grid = m_ui->gridPanel->grid();
    rc.idBasedTranslations = m_ui->idBasedTranslationsCheckBox->isChecked();
    rc.connectSlotsByName = m_ui->connectSlotsByNameCheckBox->isChecked();
    return rc;
}

void FormWindowSettings::setData(const FormWindowData &data)
{
    m_ui->layoutDefaultGroupBox->setChecked(data.layoutDefaultEnabled);
    m_ui->defaultMarginSpinBox->setValue(data.defaultMargin);
    m_ui->defaultSpacingSpinBox->setValue(data.defaultSpacing);

    m_ui->layoutFunctionGroupBox->setChecked(data.layoutFunctionsEnabled);
    m_ui->marginFunctionLineEdit->setText(data.marginFunction);
    m_ui->spacingFunctionLineEdit->setText(data.spacingFunction);

    m_ui->pixmapFunctionLineEdit->setText(data.pixFunction);
    m_ui->pixmapFunctionGroupBox->setChecked(!data.pixFunction.isEmpty());

    m_ui->authorLineEdit->setText(data.author);

    if (data.includeHints.isEmpty()) {
        m_ui->includeHintsTextEdit->clear();
    } else {
        m_ui->includeHintsTextEdit->setText(data.includeHints.join(u'\n'));
    }

    m_ui->gridPanel->setChecked(data.hasFormGrid);
    m_ui->gridPanel->setGrid(data.grid);
    m_ui->idBasedTranslationsCheckBox->setChecked(data.idBasedTranslations);
    m_ui->connectSlotsByNameCheckBox->setChecked(data.connectSlotsByName);
}

void FormWindowSettings::accept()
{
    // Anything changed? -> Apply and set dirty
    const FormWindowData newData = data();
    if (newData != *m_oldData) {
        newData.applyToFormWindow(m_formWindow);
        m_formWindow->setDirty(true);
    }

    QDialog::accept();
}
}
QT_END_NAMESPACE
