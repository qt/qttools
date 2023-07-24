// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "fontpanel.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QFontComboBox>
#include <QtCore/QTimer>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

FontPanel::FontPanel(QWidget *parentWidget) :
    QGroupBox(parentWidget),
    m_previewLineEdit(new QLineEdit),
    m_writingSystemComboBox(new QComboBox),
    m_familyComboBox(new QFontComboBox),
    m_styleComboBox(new QComboBox),
    m_pointSizeComboBox(new QComboBox),
    m_previewFontUpdateTimer(0)
{
    setTitle(tr("Font"));

    QFormLayout *formLayout = new QFormLayout(this);
    // writing systems
    m_writingSystemComboBox->setEditable(false);

    auto writingSystems = QFontDatabase::writingSystems();
    writingSystems.push_front(QFontDatabase::Any);
    for (QFontDatabase::WritingSystem ws : std::as_const(writingSystems))
        m_writingSystemComboBox->addItem(QFontDatabase::writingSystemName(ws), QVariant(ws));
    connect(m_writingSystemComboBox, &QComboBox::currentIndexChanged,
            this, &FontPanel::slotWritingSystemChanged);
    formLayout->addRow(tr("&Writing system"), m_writingSystemComboBox);

    connect(m_familyComboBox, &QFontComboBox::currentFontChanged,
            this, &FontPanel::slotFamilyChanged);
    formLayout->addRow(tr("&Family"), m_familyComboBox);

    m_styleComboBox->setEditable(false);
    connect(m_styleComboBox, &QComboBox::currentIndexChanged,
            this, &FontPanel::slotStyleChanged);
    formLayout->addRow(tr("&Style"), m_styleComboBox);

    m_pointSizeComboBox->setEditable(false);
    connect(m_pointSizeComboBox, &QComboBox::currentIndexChanged,
            this, &FontPanel::slotPointSizeChanged);
    formLayout->addRow(tr("&Point size"), m_pointSizeComboBox);

    m_previewLineEdit->setReadOnly(true);
    formLayout->addRow(m_previewLineEdit);

    setWritingSystem(QFontDatabase::Any);
}

QFont FontPanel::selectedFont() const
{
    QFont rc = m_familyComboBox->currentFont();
    const QString family = rc.family();
    rc.setPointSize(pointSize());
    const QString styleDescription = styleString();
    if (styleDescription.contains("Italic"_L1))
        rc.setStyle(QFont::StyleItalic);
    else if (styleDescription.contains("Oblique"_L1))
        rc.setStyle(QFont::StyleOblique);
    else
        rc.setStyle(QFont::StyleNormal);
    rc.setBold(QFontDatabase::bold(family, styleDescription));
    rc.setWeight(QFont::Weight(QFontDatabase::weight(family, styleDescription)));
    return rc;
}

void FontPanel::setSelectedFont(const QFont &f)
{
    m_familyComboBox->setCurrentFont(f);
    if (m_familyComboBox->currentIndex() < 0) {
        // family not in writing system - find the corresponding one?
        QList<QFontDatabase::WritingSystem> familyWritingSystems = QFontDatabase::writingSystems(f.family());
        if (familyWritingSystems.isEmpty())
            return;

        setWritingSystem(familyWritingSystems.constFirst());
        m_familyComboBox->setCurrentFont(f);
    }

    updateFamily(family());

    const int pointSizeIndex = closestPointSizeIndex(f.pointSize());
    m_pointSizeComboBox->setCurrentIndex( pointSizeIndex);

    const QString styleString = QFontDatabase::styleString(f);
    const int styleIndex = m_styleComboBox->findText(styleString);
    m_styleComboBox->setCurrentIndex(styleIndex);
    slotUpdatePreviewFont();
}


QFontDatabase::WritingSystem FontPanel::writingSystem() const
{
    const int currentIndex = m_writingSystemComboBox->currentIndex();
    if ( currentIndex == -1)
        return QFontDatabase::Latin;
    return static_cast<QFontDatabase::WritingSystem>(m_writingSystemComboBox->itemData(currentIndex).toInt());
}

QString FontPanel::family() const
{
    const int currentIndex = m_familyComboBox->currentIndex();
    return currentIndex != -1 ?  m_familyComboBox->currentFont().family() : QString();
}

int FontPanel::pointSize() const
{
    const int currentIndex = m_pointSizeComboBox->currentIndex();
    return currentIndex != -1 ? m_pointSizeComboBox->itemData(currentIndex).toInt() : 9;
}

QString FontPanel::styleString() const
{
    const int currentIndex = m_styleComboBox->currentIndex();
    return currentIndex != -1 ? m_styleComboBox->itemText(currentIndex) : QString();
}

void FontPanel::setWritingSystem(QFontDatabase::WritingSystem ws)
{
    m_writingSystemComboBox->setCurrentIndex(m_writingSystemComboBox->findData(QVariant(ws)));
    updateWritingSystem(ws);
}


void FontPanel::slotWritingSystemChanged(int)
{
    updateWritingSystem(writingSystem());
    delayedPreviewFontUpdate();
}

void FontPanel::slotFamilyChanged(const QFont &)
{
    updateFamily(family());
    delayedPreviewFontUpdate();
}

void FontPanel::slotStyleChanged(int)
{
    updatePointSizes(family(), styleString());
    delayedPreviewFontUpdate();
}

void FontPanel::slotPointSizeChanged(int)
{
    delayedPreviewFontUpdate();
}

void FontPanel::updateWritingSystem(QFontDatabase::WritingSystem ws)
{

    m_previewLineEdit->setText(QFontDatabase::writingSystemSample(ws));
    m_familyComboBox->setWritingSystem (ws);
    // Current font not in WS ... set index 0.
    if (m_familyComboBox->currentIndex() < 0) {
        m_familyComboBox->setCurrentIndex(0);
        updateFamily(family());
    }
}

void FontPanel::updateFamily(const QString &family)
{
    // Update styles and trigger update of point sizes.
    // Try to maintain selection or select normal
    const QString &oldStyleString = styleString();

    const QStringList &styles = QFontDatabase::styles(family);
    const bool hasStyles = !styles.isEmpty();

    m_styleComboBox->setCurrentIndex(-1);
    m_styleComboBox->clear();
    m_styleComboBox->setEnabled(hasStyles);

    int normalIndex = -1;
    const QString normalStyle = "Normal"_L1;

    if (hasStyles) {
        for (const QString &style : styles) {
            // try to maintain selection or select 'normal' preferably
            const int newIndex = m_styleComboBox->count();
            m_styleComboBox->addItem(style);
            if (oldStyleString == style) {
                m_styleComboBox->setCurrentIndex(newIndex);
            } else {
                if (oldStyleString ==  normalStyle)
                    normalIndex = newIndex;
            }
        }
        if (m_styleComboBox->currentIndex() == -1 && normalIndex != -1)
            m_styleComboBox->setCurrentIndex(normalIndex);
    }
    updatePointSizes(family, styleString());
}

int FontPanel::closestPointSizeIndex(int desiredPointSize) const
{
    //  try to maintain selection or select closest.
    int closestIndex = -1;
    int closestAbsError = 0xFFFF;

    const int pointSizeCount = m_pointSizeComboBox->count();
    for (int i = 0; i < pointSizeCount; i++) {
        const int itemPointSize = m_pointSizeComboBox->itemData(i).toInt();
        const int absError = qAbs(desiredPointSize - itemPointSize);
        if (absError < closestAbsError) {
            closestIndex  = i;
            closestAbsError = absError;
            if (closestAbsError == 0)
                break;
        } else {    // past optimum
            if (absError > closestAbsError) {
                break;
            }
        }
    }
    return closestIndex;
}


void FontPanel::updatePointSizes(const QString &family, const QString &styleString)
{
    const int oldPointSize = pointSize();

    auto pointSizes =  QFontDatabase::pointSizes(family, styleString);
    if (pointSizes.isEmpty())
        pointSizes = QFontDatabase::standardSizes();

    const bool hasSizes = !pointSizes.isEmpty();
    m_pointSizeComboBox->clear();
    m_pointSizeComboBox->setEnabled(hasSizes);
    m_pointSizeComboBox->setCurrentIndex(-1);

    //  try to maintain selection or select closest.
    if (hasSizes) {
        QString n;
        for (int pointSize : std::as_const(pointSizes))
            m_pointSizeComboBox->addItem(n.setNum(pointSize), QVariant(pointSize));
        const int closestIndex = closestPointSizeIndex(oldPointSize);
        if (closestIndex != -1)
            m_pointSizeComboBox->setCurrentIndex(closestIndex);
    }
}

void FontPanel::slotUpdatePreviewFont()
{
    m_previewLineEdit->setFont(selectedFont());
}

void FontPanel::delayedPreviewFontUpdate()
{
    if (!m_previewFontUpdateTimer) {
        m_previewFontUpdateTimer = new QTimer(this);
        connect(m_previewFontUpdateTimer, &QTimer::timeout,
                this, &FontPanel::slotUpdatePreviewFont);
        m_previewFontUpdateTimer->setInterval(0);
        m_previewFontUpdateTimer->setSingleShot(true);
    }
    if (m_previewFontUpdateTimer->isActive())
        return;
    m_previewFontUpdateTimer->start();
}

QT_END_NAMESPACE
