// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtgradientstopscontroller.h"
#include "ui_qtgradienteditor.h"
#include "qtgradientstopsmodel.h"

#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QtGradientStopsControllerPrivate : public QObject
{
    Q_OBJECT
    QtGradientStopsController *q_ptr;
    Q_DECLARE_PUBLIC(QtGradientStopsController)
public:
    using PositionColorMap = QMap<qreal, QColor>;
    using PositionStopMap = QMap<qreal, QtGradientStop *>;

    void setUi(Ui::QtGradientEditor *ui);

    void slotHsvClicked();
    void slotRgbClicked();

    void slotCurrentStopChanged(QtGradientStop *stop);
    void slotStopMoved(QtGradientStop *stop, qreal newPos);
    void slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2);
    void slotStopChanged(QtGradientStop *stop, const QColor &newColor);
    void slotStopSelected(QtGradientStop *stop, bool selected);
    void slotStopAdded(QtGradientStop *stop);
    void slotStopRemoved(QtGradientStop *stop);
    void slotUpdatePositionSpinBox();

    void slotChangeColor(const QColor &color);
    void slotChangeHueColor(const QColor &color);
    void slotChangeSaturationColor(const QColor &color);
    void slotChangeValueColor(const QColor &color);
    void slotChangeAlphaColor(const QColor &color);
    void slotChangeHue(int color);
    void slotChangeSaturation(int color);
    void slotChangeValue(int color);
    void slotChangeAlpha(int color);
    void slotChangePosition(double value);

    void slotChangeZoom(int value);
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomAll();
    void slotZoomChanged(double zoom);

    void enableCurrent(bool enable);
    void setColorSpinBoxes(const QColor &color);
    PositionColorMap stopsData(const PositionStopMap &stops) const;
    QGradientStops makeGradientStops(const PositionColorMap &data) const;
    void updateZoom(double zoom);

    QtGradientStopsModel *m_model = nullptr;
    QColor::Spec m_spec = QColor::Hsv;

    Ui::QtGradientEditor *m_ui;
};

void QtGradientStopsControllerPrivate::setUi(Ui::QtGradientEditor *ui)
{
    m_ui = ui;

    m_ui->hueColorLine->setColorComponent(QtColorLine::Hue);
    m_ui->saturationColorLine->setColorComponent(QtColorLine::Saturation);
    m_ui->valueColorLine->setColorComponent(QtColorLine::Value);
    m_ui->alphaColorLine->setColorComponent(QtColorLine::Alpha);

    m_model = new QtGradientStopsModel(this);
    m_ui->gradientStopsWidget->setGradientStopsModel(m_model);
    connect(m_model, &QtGradientStopsModel::currentStopChanged,
            this, &QtGradientStopsControllerPrivate::slotCurrentStopChanged);
    connect(m_model, &QtGradientStopsModel::stopMoved,
            this, &QtGradientStopsControllerPrivate::slotStopMoved);
    connect(m_model, &QtGradientStopsModel::stopsSwapped,
            this, &QtGradientStopsControllerPrivate::slotStopsSwapped);
    connect(m_model, &QtGradientStopsModel::stopChanged,
            this, &QtGradientStopsControllerPrivate::slotStopChanged);
    connect(m_model, &QtGradientStopsModel::stopSelected,
            this, &QtGradientStopsControllerPrivate::slotStopSelected);
    connect(m_model, &QtGradientStopsModel::stopAdded,
            this, &QtGradientStopsControllerPrivate::slotStopAdded);
    connect(m_model, &QtGradientStopsModel::stopRemoved,
            this, &QtGradientStopsControllerPrivate::slotStopRemoved);

    connect(m_ui->hueColorLine, &QtColorLine::colorChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeHueColor);
    connect(m_ui->saturationColorLine, &QtColorLine::colorChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeSaturationColor);
    connect(m_ui->valueColorLine, &QtColorLine::colorChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeValueColor);
    connect(m_ui->alphaColorLine, &QtColorLine::colorChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeAlphaColor);
    connect(m_ui->colorButton, &QtColorButton::colorChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeColor);

    connect(m_ui->hueSpinBox, &QSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeHue);
    connect(m_ui->saturationSpinBox, &QSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeSaturation);
    connect(m_ui->valueSpinBox, &QSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeValue);
    connect(m_ui->alphaSpinBox, &QSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeAlpha);

    connect(m_ui->positionSpinBox, &QDoubleSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangePosition);

    connect(m_ui->zoomSpinBox, &QSpinBox::valueChanged,
            this, &QtGradientStopsControllerPrivate::slotChangeZoom);
    connect(m_ui->zoomInButton, &QToolButton::clicked,
            this, &QtGradientStopsControllerPrivate::slotZoomIn);
    connect(m_ui->zoomOutButton, &QToolButton::clicked,
            this, &QtGradientStopsControllerPrivate::slotZoomOut);
    connect(m_ui->zoomAllButton, &QToolButton::clicked,
            this, &QtGradientStopsControllerPrivate::slotZoomAll);
    connect(m_ui->gradientStopsWidget, &QtGradientStopsWidget::zoomChanged,
            this, &QtGradientStopsControllerPrivate::slotZoomChanged);

    connect(m_ui->hsvRadioButton, &QRadioButton::clicked,
            this, &QtGradientStopsControllerPrivate::slotHsvClicked);
    connect(m_ui->rgbRadioButton, &QRadioButton::clicked,
            this, &QtGradientStopsControllerPrivate::slotRgbClicked);

    enableCurrent(false);
    m_ui->zoomInButton->setIcon(QIcon(":/qt-project.org/qtgradienteditor/images/zoomin.png"_L1));
    m_ui->zoomOutButton->setIcon(QIcon(":/qt-project.org/qtgradienteditor/images/zoomout.png"_L1));
    updateZoom(1);
}

void QtGradientStopsControllerPrivate::enableCurrent(bool enable)
{
    m_ui->positionLabel->setEnabled(enable);
    m_ui->colorLabel->setEnabled(enable);
    m_ui->hLabel->setEnabled(enable);
    m_ui->sLabel->setEnabled(enable);
    m_ui->vLabel->setEnabled(enable);
    m_ui->aLabel->setEnabled(enable);
    m_ui->hueLabel->setEnabled(enable);
    m_ui->saturationLabel->setEnabled(enable);
    m_ui->valueLabel->setEnabled(enable);
    m_ui->alphaLabel->setEnabled(enable);

    m_ui->positionSpinBox->setEnabled(enable);
    m_ui->colorButton->setEnabled(enable);

    m_ui->hueColorLine->setEnabled(enable);
    m_ui->saturationColorLine->setEnabled(enable);
    m_ui->valueColorLine->setEnabled(enable);
    m_ui->alphaColorLine->setEnabled(enable);

    m_ui->hueSpinBox->setEnabled(enable);
    m_ui->saturationSpinBox->setEnabled(enable);
    m_ui->valueSpinBox->setEnabled(enable);
    m_ui->alphaSpinBox->setEnabled(enable);
}

QtGradientStopsControllerPrivate::PositionColorMap QtGradientStopsControllerPrivate::stopsData(const PositionStopMap &stops) const
{
    PositionColorMap data;
    for (QtGradientStop *stop : stops)
        data[stop->position()] = stop->color();
    return data;
}

QGradientStops QtGradientStopsControllerPrivate::makeGradientStops(const PositionColorMap &data) const
{
    QGradientStops stops;
    for (auto itData = data.cbegin(), cend = data.cend(); itData != cend; ++itData)
        stops << QPair<qreal, QColor>(itData.key(), itData.value());
    return stops;
}

void QtGradientStopsControllerPrivate::updateZoom(double zoom)
{
    m_ui->gradientStopsWidget->setZoom(zoom);
    m_ui->zoomSpinBox->blockSignals(true);
    m_ui->zoomSpinBox->setValue(qRound(zoom * 100));
    m_ui->zoomSpinBox->blockSignals(false);
    bool zoomInEnabled = true;
    bool zoomOutEnabled = true;
    bool zoomAllEnabled = true;
    if (zoom <= 1) {
        zoomAllEnabled = false;
        zoomOutEnabled = false;
    } else if (zoom >= 100) {
        zoomInEnabled = false;
    }
    m_ui->zoomInButton->setEnabled(zoomInEnabled);
    m_ui->zoomOutButton->setEnabled(zoomOutEnabled);
    m_ui->zoomAllButton->setEnabled(zoomAllEnabled);
}

void QtGradientStopsControllerPrivate::slotHsvClicked()
{
    QString h = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "H");
    QString s = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "S");
    QString v = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "V");

    m_ui->hLabel->setText(h);
    m_ui->sLabel->setText(s);
    m_ui->vLabel->setText(v);

    h = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Hue");
    s = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Sat");
    v = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Val");

    const QString hue = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Hue");
    const QString saturation = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Saturation");
    const QString value = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Value");

    m_ui->hLabel->setToolTip(hue);
    m_ui->hueLabel->setText(h);
    m_ui->hueColorLine->setToolTip(hue);
    m_ui->hueColorLine->setColorComponent(QtColorLine::Hue);

    m_ui->sLabel->setToolTip(saturation);
    m_ui->saturationLabel->setText(s);
    m_ui->saturationColorLine->setToolTip(saturation);
    m_ui->saturationColorLine->setColorComponent(QtColorLine::Saturation);

    m_ui->vLabel->setToolTip(value);
    m_ui->valueLabel->setText(v);
    m_ui->valueColorLine->setToolTip(value);
    m_ui->valueColorLine->setColorComponent(QtColorLine::Value);

    setColorSpinBoxes(m_ui->colorButton->color());
}

void QtGradientStopsControllerPrivate::slotRgbClicked()
{
    QString r = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "R");
    QString g = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "G");
    QString b = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "B");

    m_ui->hLabel->setText(r);
    m_ui->sLabel->setText(g);
    m_ui->vLabel->setText(b);

    QString red = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Red");
    QString green = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Green");
    QString blue = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Blue");

    m_ui->hLabel->setToolTip(red);
    m_ui->hueLabel->setText(red);
    m_ui->hueColorLine->setToolTip(red);
    m_ui->hueColorLine->setColorComponent(QtColorLine::Red);

    m_ui->sLabel->setToolTip(green);
    m_ui->saturationLabel->setText(green);
    m_ui->saturationColorLine->setToolTip(green);
    m_ui->saturationColorLine->setColorComponent(QtColorLine::Green);

    m_ui->vLabel->setToolTip(blue);
    m_ui->valueLabel->setText(blue);
    m_ui->valueColorLine->setToolTip(blue);
    m_ui->valueColorLine->setColorComponent(QtColorLine::Blue);

    setColorSpinBoxes(m_ui->colorButton->color());
}

void QtGradientStopsControllerPrivate::setColorSpinBoxes(const QColor &color)
{
    m_ui->hueSpinBox->blockSignals(true);
    m_ui->saturationSpinBox->blockSignals(true);
    m_ui->valueSpinBox->blockSignals(true);
    m_ui->alphaSpinBox->blockSignals(true);
    if (m_ui->hsvRadioButton->isChecked()) {
        if (m_ui->hueSpinBox->maximum() != 359)
            m_ui->hueSpinBox->setMaximum(359);
        if (m_ui->hueSpinBox->value() != color.hue())
            m_ui->hueSpinBox->setValue(color.hue());
        if (m_ui->saturationSpinBox->value() != color.saturation())
            m_ui->saturationSpinBox->setValue(color.saturation());
        if (m_ui->valueSpinBox->value() != color.value())
            m_ui->valueSpinBox->setValue(color.value());
    } else {
        if (m_ui->hueSpinBox->maximum() != 255)
            m_ui->hueSpinBox->setMaximum(255);
        if (m_ui->hueSpinBox->value() != color.red())
            m_ui->hueSpinBox->setValue(color.red());
        if (m_ui->saturationSpinBox->value() != color.green())
            m_ui->saturationSpinBox->setValue(color.green());
        if (m_ui->valueSpinBox->value() != color.blue())
            m_ui->valueSpinBox->setValue(color.blue());
    }
    m_ui->alphaSpinBox->setValue(color.alpha());
    m_ui->hueSpinBox->blockSignals(false);
    m_ui->saturationSpinBox->blockSignals(false);
    m_ui->valueSpinBox->blockSignals(false);
    m_ui->alphaSpinBox->blockSignals(false);
}

void QtGradientStopsControllerPrivate::slotCurrentStopChanged(QtGradientStop *stop)
{
    if (!stop) {
        enableCurrent(false);
        return;
    }
    enableCurrent(true);

    QTimer::singleShot(0, this, &QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox);

    m_ui->colorButton->setColor(stop->color());
    m_ui->hueColorLine->setColor(stop->color());
    m_ui->saturationColorLine->setColor(stop->color());
    m_ui->valueColorLine->setColor(stop->color());
    m_ui->alphaColorLine->setColor(stop->color());
    setColorSpinBoxes(stop->color());
}

void QtGradientStopsControllerPrivate::slotStopMoved(QtGradientStop *stop, qreal newPos)
{
    QTimer::singleShot(0, this, &QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox);

    PositionColorMap stops = stopsData(m_model->stops());
    stops.remove(stop->position());
    stops[newPos] = stop->color();

    QGradientStops gradStops = makeGradientStops(stops);
    emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2)
{
    QTimer::singleShot(0, this, &QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox);

    PositionColorMap stops = stopsData(m_model->stops());
    const qreal pos1 = stop1->position();
    const qreal pos2 = stop2->position();
    stops[pos1] = stop2->color();
    stops[pos2] = stop1->color();

    QGradientStops gradStops = makeGradientStops(stops);
    emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopAdded(QtGradientStop *stop)
{
    PositionColorMap stops = stopsData(m_model->stops());
    stops[stop->position()] = stop->color();

    QGradientStops gradStops = makeGradientStops(stops);
    emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopRemoved(QtGradientStop *stop)
{
    PositionColorMap stops = stopsData(m_model->stops());
    stops.remove(stop->position());

    QGradientStops gradStops = makeGradientStops(stops);
    emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopChanged(QtGradientStop *stop, const QColor &newColor)
{
    if (m_model->currentStop() == stop) {
        m_ui->colorButton->setColor(newColor);
        m_ui->hueColorLine->setColor(newColor);
        m_ui->saturationColorLine->setColor(newColor);
        m_ui->valueColorLine->setColor(newColor);
        m_ui->alphaColorLine->setColor(newColor);
        setColorSpinBoxes(newColor);
    }

    PositionColorMap stops = stopsData(m_model->stops());
    stops[stop->position()] = newColor;

    QGradientStops gradStops = makeGradientStops(stops);
    emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopSelected(QtGradientStop *stop, bool selected)
{
    Q_UNUSED(stop);
    Q_UNUSED(selected);
    QTimer::singleShot(0, this, &QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox);
}

void QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox()
{
    QtGradientStop *current = m_model->currentStop();
    if (!current)
        return;

    qreal min = 0.0;
    qreal max = 1.0;
    const qreal pos = current->position();

    QtGradientStop *first = m_model->firstSelected();
    QtGradientStop *last = m_model->lastSelected();

    if (first && last) {
        const qreal minPos = pos - first->position() - 0.0004999;
        const qreal maxPos = pos + 1.0 - last->position() + 0.0004999;

        if (max > maxPos)
            max = maxPos;
        if (min < minPos)
            min = minPos;

        if (first->position() == 0.0)
            min = pos;
        if (last->position() == 1.0)
            max = pos;
    }

    const int spinMin = qRound(m_ui->positionSpinBox->minimum() * 1000);
    const int spinMax = qRound(m_ui->positionSpinBox->maximum() * 1000);

    const int newMin = qRound(min * 1000);
    const int newMax = qRound(max * 1000);

    m_ui->positionSpinBox->blockSignals(true);
    if (spinMin != newMin || spinMax != newMax) {
        m_ui->positionSpinBox->setRange(double(newMin) / 1000, double(newMax) / 1000);
    }
    if (m_ui->positionSpinBox->value() != pos)
        m_ui->positionSpinBox->setValue(pos);
    m_ui->positionSpinBox->blockSignals(false);
}

void QtGradientStopsControllerPrivate::slotChangeColor(const QColor &color)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;
    m_model->changeStop(stop, color);
    const auto stops = m_model->selectedStops();
    for (QtGradientStop *s : stops) {
        if (s != stop)
            m_model->changeStop(s, color);
    }
}

void QtGradientStopsControllerPrivate::slotChangeHueColor(const QColor &color)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;
    m_model->changeStop(stop, color);
    const auto stops = m_model->selectedStops();
    for (QtGradientStop *s : stops) {
        if (s != stop) {
            QColor c = s->color();
            if (m_ui->hsvRadioButton->isChecked())
                c.setHsvF(color.hueF(), c.saturationF(), c.valueF(), c.alphaF());
            else
                c.setRgbF(color.redF(), c.greenF(), c.blueF(), c.alphaF());
            m_model->changeStop(s, c);
        }
    }
}

void QtGradientStopsControllerPrivate::slotChangeHue(int color)
{
    QColor c = m_ui->hueColorLine->color();
    if (m_ui->hsvRadioButton->isChecked())
        c.setHsvF(qreal(color) / 360.0, c.saturationF(), c.valueF(), c.alphaF());
    else
        c.setRed(color);
    slotChangeHueColor(c);
}

void QtGradientStopsControllerPrivate::slotChangeSaturationColor(const QColor &color)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;
    m_model->changeStop(stop, color);
    const auto stops = m_model->selectedStops();
    for (QtGradientStop *s : stops) {
        if (s != stop) {
            QColor c = s->color();
            if (m_ui->hsvRadioButton->isChecked()) {
                c.setHsvF(c.hueF(), color.saturationF(), c.valueF(), c.alphaF());
                int hue = c.hue();
                if (hue == 360 || hue == -1)
                    c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            } else {
                c.setRgbF(c.redF(), color.greenF(), c.blueF(), c.alphaF());
            }
            m_model->changeStop(s, c);
        }
    }
}

void QtGradientStopsControllerPrivate::slotChangeSaturation(int color)
{
    QColor c = m_ui->saturationColorLine->color();
    if (m_ui->hsvRadioButton->isChecked())
        c.setHsvF(c.hueF(), qreal(color) / 255, c.valueF(), c.alphaF());
    else
        c.setGreen(color);
    slotChangeSaturationColor(c);
}

void QtGradientStopsControllerPrivate::slotChangeValueColor(const QColor &color)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;
    m_model->changeStop(stop, color);
    const auto stops = m_model->selectedStops();
    for (QtGradientStop *s : stops) {
        if (s != stop) {
            QColor c = s->color();
            if (m_ui->hsvRadioButton->isChecked()) {
                c.setHsvF(c.hueF(), c.saturationF(), color.valueF(), c.alphaF());
                int hue = c.hue();
                if (hue == 360 || hue == -1)
                    c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            } else {
                c.setRgbF(c.redF(), c.greenF(), color.blueF(), c.alphaF());
            }
            m_model->changeStop(s, c);
        }
    }
}

void QtGradientStopsControllerPrivate::slotChangeValue(int color)
{
    QColor c = m_ui->valueColorLine->color();
    if (m_ui->hsvRadioButton->isChecked())
        c.setHsvF(c.hueF(), c.saturationF(), qreal(color) / 255, c.alphaF());
    else
        c.setBlue(color);
    slotChangeValueColor(c);
}

void QtGradientStopsControllerPrivate::slotChangeAlphaColor(const QColor &color)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;
    m_model->changeStop(stop, color);
    const auto stops = m_model->selectedStops();
    for (QtGradientStop *s : stops) {
        if (s != stop) {
            QColor c = s->color();
            if (m_ui->hsvRadioButton->isChecked()) {
                c.setHsvF(c.hueF(), c.saturationF(), c.valueF(), color.alphaF());
                int hue = c.hue();
                if (hue == 360 || hue == -1)
                    c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            } else {
                c.setRgbF(c.redF(), c.greenF(), c.blueF(), color.alphaF());
            }
            m_model->changeStop(s, c);
        }
    }
}

void QtGradientStopsControllerPrivate::slotChangeAlpha(int color)
{
    QColor c = m_ui->alphaColorLine->color();
    if (m_ui->hsvRadioButton->isChecked())
        c.setHsvF(c.hueF(), c.saturationF(), c.valueF(), qreal(color) / 255);
    else
        c.setAlpha(color);
    slotChangeAlphaColor(c);
}

void QtGradientStopsControllerPrivate::slotChangePosition(double value)
{
    QtGradientStop *stop = m_model->currentStop();
    if (!stop)
        return;

    m_model->moveStops(value);
}

void QtGradientStopsControllerPrivate::slotChangeZoom(int value)
{
    updateZoom(value / 100.0);
}

void QtGradientStopsControllerPrivate::slotZoomIn()
{
    double newZoom = m_ui->gradientStopsWidget->zoom() * 2;
    if (newZoom > 100)
        newZoom = 100;
    updateZoom(newZoom);
}

void QtGradientStopsControllerPrivate::slotZoomOut()
{
    double newZoom = m_ui->gradientStopsWidget->zoom() / 2;
    if (newZoom < 1)
        newZoom = 1;
    updateZoom(newZoom);
}

void QtGradientStopsControllerPrivate::slotZoomAll()
{
    updateZoom(1);
}

void QtGradientStopsControllerPrivate::slotZoomChanged(double zoom)
{
    updateZoom(zoom);
}

QtGradientStopsController::QtGradientStopsController(QObject *parent)
    : QObject(parent), d_ptr(new QtGradientStopsControllerPrivate())
{
    d_ptr->q_ptr = this;
}

void QtGradientStopsController::setUi(Ui::QtGradientEditor *ui)
{
    d_ptr->setUi(ui);
}

QtGradientStopsController::~QtGradientStopsController()
{
}

void QtGradientStopsController::setGradientStops(const QGradientStops &stops)
{
    d_ptr->m_model->clear();
    QtGradientStop *first = nullptr;
    for (const QPair<qreal, QColor> &pair : stops) {
        QtGradientStop *stop = d_ptr->m_model->addStop(pair.first, pair.second);
        if (!first)
            first = stop;
    }
    if (first)
        d_ptr->m_model->setCurrentStop(first);
}

QGradientStops QtGradientStopsController::gradientStops() const
{
    QGradientStops stops;
    const auto stopsList = d_ptr->m_model->stops().values();
    for (const QtGradientStop *stop : stopsList)
        stops << QPair<qreal, QColor>(stop->position(), stop->color());
    return stops;
}

QColor::Spec QtGradientStopsController::spec() const
{
    return d_ptr->m_spec;
}

void QtGradientStopsController::setSpec(QColor::Spec spec)
{
    if (d_ptr->m_spec == spec)
        return;

    d_ptr->m_spec = spec;
    if (d_ptr->m_spec == QColor::Rgb) {
        d_ptr->m_ui->rgbRadioButton->setChecked(true);
        d_ptr->slotRgbClicked();
    } else {
        d_ptr->m_ui->hsvRadioButton->setChecked(true);
        d_ptr->slotHsvClicked();
    }
}

QT_END_NAMESPACE

#include "qtgradientstopscontroller.moc"
