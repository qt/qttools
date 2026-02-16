// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtpropertybrowserutils_p.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>

#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterstateguard.h>

#include <QtCore/qhash.h>
#include <QtCore/qlocale.h>
#include <QtCore/qoperatingsystemversion.h>

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

// Make sure icons are removed as soon as QApplication is destroyed, otherwise,
// handles are leaked on X11.
static void clearCursorDatabase()
{
    QtCursorDatabase::instance()->clear();
}

using ShapeNames = QHash<Qt::CursorShape, const char *>;

static const ShapeNames &shapeNames()
{
    static const ShapeNames result =
            {{Qt::ArrowCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Arrow")},
              {Qt::UpArrowCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Up Arrow")},
              {Qt::CrossCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Cross")},
              {Qt::WaitCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Wait")},
              {Qt::IBeamCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "IBeam")},
              {Qt::SizeVerCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Size Vertical")},
              {Qt::SizeHorCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Size Horizontal")},
              {Qt::SizeFDiagCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Size Backslash")},
              {Qt::SizeBDiagCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Size Slash")},
              {Qt::SizeAllCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Size All")},
              {Qt::BlankCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Blank")},
              {Qt::SplitVCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Split Vertical")},
              {Qt::SplitHCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Split Horizontal")},
              {Qt::PointingHandCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Pointing Hand")},
              {Qt::ForbiddenCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Forbidden")},
              {Qt::OpenHandCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Open Hand")},
              {Qt::ClosedHandCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Closed Hand")},
              {Qt::WhatsThisCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "What's This")},
              {Qt::BusyCursor, QT_TRANSLATE_NOOP("QtCursorDatabase", "Busy")}};
    return result;
}

using CursorResource = std::pair<Qt::CursorShape, QLatin1StringView>;

static constexpr CursorResource cursorResources[] = {
    {Qt::ArrowCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-arrow.png"_L1},
    {Qt::UpArrowCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-uparrow.png"_L1},
    {Qt::CrossCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-cross.png"_L1},
    {Qt::WaitCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-wait.png"_L1},
    {Qt::IBeamCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-ibeam.png"_L1},
    {Qt::SizeVerCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-sizev.png"_L1},
    {Qt::SizeHorCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-sizeh.png"_L1},
    {Qt::SizeFDiagCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-sizef.png"_L1},
    {Qt::SizeBDiagCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-sizeb.png"_L1},
    {Qt::SizeAllCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-sizeall.png"_L1},
    {Qt::SplitVCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-vsplit.png"_L1},
    {Qt::SplitHCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-hsplit.png"_L1},
    {Qt::PointingHandCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-hand.png"_L1},
    {Qt::ForbiddenCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-forbidden.png"_L1},
    {Qt::OpenHandCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-openhand.png"_L1},
    {Qt::ClosedHandCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-closedhand.png"_L1},
    {Qt::WhatsThisCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-whatsthis.png"_L1},
    {Qt::BusyCursor, ":/qt-project.org/qtpropertybrowser/images/cursor-busy.png"_L1}
};

// A Pixmap icon engine for resource pixmaps that do not have a DPR set.
// It sets the DPR on the pixmaps and either scales it down or centers it.
class QtPixmapIconEngine : public QtPropertyIconEngine
{
public:
    Q_DISABLE_COPY_MOVE(QtPixmapIconEngine)

    explicit QtPixmapIconEngine(QPixmap p) : m_pixmap(std::move(p)) { }

    QIconEngine *clone() const override
    {
        return new QtPixmapIconEngine(m_pixmap);
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State) override
    {
        QPainterStateGuard psg(painter);
        painter->setRenderHint(QPainter::Antialiasing, true);

        const auto dpr = painter->device()->devicePixelRatioF();
        m_pixmap.setDevicePixelRatio(dpr);
        QSize dipSize =
                qFuzzyCompare(dpr, 1) ? m_pixmap.size() : (QSizeF(m_pixmap.size()) / dpr).toSize();
        const int dipWidth = dipSize.width();
        const int dipHeight = dipSize.height();
        if (dipWidth > rect.width() || dipHeight > rect.height()) {
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            painter->drawPixmap(rect, m_pixmap); // Too big: Let Qt scale down
        } else {
            QPoint point = rect.topLeft(); // center if needed
            if (dipWidth < rect.width())
                point.rx() += (rect.width() - dipWidth) / 2;
            if (dipHeight < rect.height())
                point.ry() += (rect.height() - dipHeight) / 2;
            painter->drawPixmap(point, m_pixmap);
        }
    }

private:
    QPixmap m_pixmap;
};

static inline QIcon getResourceCursorIcon(const QString &fn)
{
    return QIcon{new QtPixmapIconEngine(QPixmap(fn))};
}

// Get an icon from the resources of the Windows QPA plugin. No scaling required.
[[maybe_unused]] static QIcon getWindowsCursorIcon(QLatin1StringView name)
{
    static constexpr auto root = ":/qt-project.org/windows/cursors/images/"_L1;
    static constexpr QLatin1StringView suffixes[] = {"_32.png"_L1, "_48.png"_L1, "_64.png"_L1};
    QIcon result;
    for (auto suffix : suffixes) {
        const QString path = root + name + suffix;
        QPixmap pixmap(path);
        if (pixmap.isNull())
            qWarning("%s: Failed to load \"%s\"", __FUNCTION__, qPrintable(path));
        else
            result.addPixmap(pixmap);
    }
    return result;
}

// Get a (small) icon from the resources of the Cocoa QPA plugin.
[[maybe_unused]] static QIcon getCocoaCursorIcon(QLatin1StringView name)
{
    static constexpr auto root = ":/qt-project.org/mac/cursors/images/"_L1;
    const QString path = root + name;
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        qWarning("%s: Failed to load \"%s\"", __FUNCTION__, qPrintable(path));
        return {};
    }
    return QIcon{new QtPixmapIconEngine(pixmap)};
}

QtCursorDatabase::QtCursorDatabase()
{
    qAddPostRoutine(clearCursorDatabase);

    if constexpr (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
        appendCursor(Qt::ClosedHandCursor, getWindowsCursorIcon("closedhandcursor"_L1));
        appendCursor(Qt::OpenHandCursor, getWindowsCursorIcon("openhandcursor"_L1));
        appendCursor(Qt::SplitHCursor, getWindowsCursorIcon("splithcursor"_L1));
        appendCursor(Qt::SplitVCursor, getWindowsCursorIcon("splitvcursor"_L1));
    }

    if constexpr (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::MacOS) {
        appendCursor(Qt::BusyCursor, getCocoaCursorIcon("spincursor.png"_L1));
        appendCursor(Qt::WaitCursor, getCocoaCursorIcon("waitcursor.png"_L1));
    }

    appendCursor(Qt::BlankCursor, QIcon{});

    for (const auto &cursorResource : cursorResources) {
        if (!hasCursor(cursorResource.first))
            appendCursor(cursorResource.first, getResourceCursorIcon(cursorResource.second));
    }
}

void QtCursorDatabase::clear()
{
    m_cursorNames.clear();
    m_cursorIcons.clear();
    m_valueToCursorShape.clear();
    m_cursorShapeToValue.clear();
}

void QtCursorDatabase::appendCursor(Qt::CursorShape shape, const QIcon &icon)
{
    if (hasCursor(shape) || (shape != Qt::BlankCursor && icon.isNull()))
        return;
    const qsizetype value = m_cursorNames.size();
    const char *name = shapeNames().value(shape, nullptr);
    m_cursorNames.append(name ? QCoreApplication::translate("QtCursorDatabase", name) : QString{});
    m_cursorIcons.insert(value, icon);
    m_valueToCursorShape.insert(value, shape);
    m_cursorShapeToValue.insert(shape, value);
}

QStringList QtCursorDatabase::cursorShapeNames() const
{
    return m_cursorNames;
}

QMap<int, QIcon> QtCursorDatabase::cursorShapeIcons() const
{
    return m_cursorIcons;
}

QString QtCursorDatabase::cursorToShapeName(const QCursor &cursor) const
{
    int val = cursorToValue(cursor);
    if (val >= 0)
        return m_cursorNames.at(val);
    return {};
}

QIcon QtCursorDatabase::cursorToShapeIcon(const QCursor &cursor) const
{
    int val = cursorToValue(cursor);
    return m_cursorIcons.value(val);
}

int QtCursorDatabase::cursorToValue(const QCursor &cursor) const
{
#ifndef QT_NO_CURSOR
    return m_cursorShapeToValue.value(cursor.shape(), -1);
#endif
    return -1;
}

#ifndef QT_NO_CURSOR
QCursor QtCursorDatabase::valueToCursor(int value) const
{
    auto it = m_valueToCursorShape.constFind(value);
    if (it != m_valueToCursorShape.cend())
        return QCursor(it.value());
    return {};
}
#endif

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

QtCursorDatabase *QtCursorDatabase::instance()
{
    return cursorDatabase();
}

class QtPropertyBrushValueIconEngine : public QtPropertyIconEngine
{
public:
    Q_DISABLE_COPY_MOVE(QtPropertyBrushValueIconEngine)

    explicit QtPropertyBrushValueIconEngine(const QBrush &b) : m_brush(b) { }

    QIconEngine *clone() const override
    {
        return new QtPropertyBrushValueIconEngine(m_brush);
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State) override
    {
        QPainterStateGuard psg(painter);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, m_brush);
        QColor color = m_brush.color();
        if (color.alpha() != 255) { // indicate alpha by an inset
            QBrush opaqueBrush = m_brush;
            color.setAlpha(255);
            opaqueBrush.setColor(color);
            const QPoint offset{rect.width() / 4, rect.height() / 4};
            const QRect inset{rect.topLeft() + offset, rect.size() / 2};
            painter->fillRect(inset, opaqueBrush);
        }
    }

private:
    QBrush m_brush;
};

QPixmap QtPropertyBrowserUtils::brushValuePixmap(const QBrush &b, const QSize &size,
                                                 qreal devicePixelRatio)
{
    QtPropertyBrushValueIconEngine engine(b);
    return engine.drawPixmap(size, devicePixelRatio);
}

QIcon QtPropertyBrowserUtils::brushValueIcon(const QBrush &b)
{
    return QIcon(new QtPropertyBrushValueIconEngine(b));
}

QString QtPropertyBrowserUtils::colorValueText(QColor c)
{
    return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2, %3] (%4)")
           .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

class QtPropertyFontValueIconEngine : public QtPropertyIconEngine
{
public:
    Q_DISABLE_COPY_MOVE(QtPropertyFontValueIconEngine)

    explicit QtPropertyFontValueIconEngine(const QFont &f) : m_font(f) { }

    QIconEngine *clone() const override
    {
        return new QtPropertyFontValueIconEngine(m_font);
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State) override
    {
        QPainterStateGuard psg(painter);
        const auto maxDimension = qMax(rect.width(), rect.height());
        m_font.setPointSize(maxDimension - 3);
        painter->setPen(QPen(Qt::black));
        painter->setFont(m_font);
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawText(rect, QString(u'A'), QTextOption(Qt::AlignCenter));
    }

private:
    QFont m_font;
};

QPixmap QtPropertyBrowserUtils::fontValuePixmap(const QFont &font, const QSize &size,
                                                qreal devicePixelRatio)
{
    QtPropertyFontValueIconEngine engine(font);
    return engine.drawPixmap(size, devicePixelRatio);
}

QIcon QtPropertyBrowserUtils::fontValueIcon(const QFont &f)
{
    return QIcon(new QtPropertyFontValueIconEngine(f));
}

QString QtPropertyBrowserUtils::fontValueText(const QFont &f)
{
    return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2]")
           .arg(f.family()).arg(f.pointSize());
}

QString QtPropertyBrowserUtils::dateFormat()
{
    QLocale loc;
    QString format = loc.dateFormat(QLocale::ShortFormat);
    // Change dd.MM.yy, MM/dd/yy to 4 digit years
    if (format.count(QLatin1Char('y')) == 2)
        format.insert(format.indexOf(QLatin1Char('y')), "yy"_L1);
    return format;
}

QString QtPropertyBrowserUtils::timeFormat()
{
    QLocale loc;
    // ShortFormat is missing seconds on UNIX.
    return loc.timeFormat(QLocale::LongFormat);
}

QString QtPropertyBrowserUtils::dateTimeFormat()
{
    QString format = dateFormat();
    format += QLatin1Char(' ');
    format += timeFormat();
    return format;
}

QtBoolEdit::QtBoolEdit(QWidget *parent) :
    QWidget(parent),
    m_checkBox(new QCheckBox(this)),
    m_textVisible(true)
{
    auto *lt = new QHBoxLayout;
    if (QApplication::layoutDirection() == Qt::LeftToRight)
        lt->setContentsMargins(4, 0, 0, 0);
    else
        lt->setContentsMargins(0, 0, 4, 0);
    lt->addWidget(m_checkBox);
    setLayout(lt);
    connect(m_checkBox, &QAbstractButton::toggled, this, &QtBoolEdit::toggled);
    setFocusProxy(m_checkBox);
    m_checkBox->setText(tr("True"));
}

void QtBoolEdit::setTextVisible(bool textVisible)
{
    if (m_textVisible == textVisible)
        return;

    m_textVisible = textVisible;
    if (m_textVisible)
        m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
    else
        m_checkBox->setText(QString());
}

Qt::CheckState QtBoolEdit::checkState() const
{
    return m_checkBox->checkState();
}

void QtBoolEdit::setCheckState(Qt::CheckState state)
{
    m_checkBox->setCheckState(state);
}

bool QtBoolEdit::isChecked() const
{
    return m_checkBox->isChecked();
}

void QtBoolEdit::setChecked(bool c)
{
    m_checkBox->setChecked(c);
    if (!m_textVisible)
        return;
    m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
}

bool QtBoolEdit::blockCheckBoxSignals(bool block)
{
    return m_checkBox->blockSignals(block);
}

void QtBoolEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        m_checkBox->click();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

QPixmap QtPropertyIconEngine::createEmptyPixmap(const QSize &size, qreal devicePixelRatio)
{
    QPixmap result(size * devicePixelRatio);
    result.fill(Qt::transparent);
    result.setDevicePixelRatio(devicePixelRatio);
    return result;
}

QtPropertyIconEngine::QtPropertyIconEngine() = default;

// ### FIXME Qt 7: Remove? (reimplemented in Qt 6 since the default
// cannot be trusted to handle fractional dpr).
QPixmap QtPropertyIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state,
                                         qreal scale)
{
    QPixmap result = createEmptyPixmap(size, scale);
    QPainter painter(&result);
    paint(&painter, QRect(0, 0, size.width(), size.height()), mode, state);
    painter.end();
    return result;
}

QPixmap QtPropertyIconEngine::drawPixmap(const QSize &size, qreal devicePixelRatio)
{
    return scaledPixmap(size, QIcon::Mode::Normal, QIcon::State::On, devicePixelRatio);
}

QT_END_NAMESPACE
