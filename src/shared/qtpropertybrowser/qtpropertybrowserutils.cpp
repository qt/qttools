// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtpropertybrowserutils_p.h"
#include <QtWidgets/QApplication>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

// Make sure icons are removed as soon as QApplication is destroyed, otherwise,
// handles are leaked on X11.
static void clearCursorDatabase()
{
    QtCursorDatabase::instance()->clear();
}

QtCursorDatabase::QtCursorDatabase()
{
    qAddPostRoutine(clearCursorDatabase);

    appendCursor(Qt::ArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Arrow"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-arrow.png"_L1));
    appendCursor(Qt::UpArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Up Arrow"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-uparrow.png"_L1));
    appendCursor(Qt::CrossCursor, QCoreApplication::translate("QtCursorDatabase", "Cross"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-cross.png"_L1));
    appendCursor(Qt::WaitCursor, QCoreApplication::translate("QtCursorDatabase", "Wait"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-wait.png"_L1));
    appendCursor(Qt::IBeamCursor, QCoreApplication::translate("QtCursorDatabase", "IBeam"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-ibeam.png"_L1));
    appendCursor(Qt::SizeVerCursor, QCoreApplication::translate("QtCursorDatabase", "Size Vertical"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-sizev.png"_L1));
    appendCursor(Qt::SizeHorCursor, QCoreApplication::translate("QtCursorDatabase", "Size Horizontal"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-sizeh.png"_L1));
    appendCursor(Qt::SizeFDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Backslash"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-sizef.png"_L1));
    appendCursor(Qt::SizeBDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Slash"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-sizeb.png"_L1));
    appendCursor(Qt::SizeAllCursor, QCoreApplication::translate("QtCursorDatabase", "Size All"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-sizeall.png"_L1));
    appendCursor(Qt::BlankCursor, QCoreApplication::translate("QtCursorDatabase", "Blank"),
                 QIcon());
    appendCursor(Qt::SplitVCursor, QCoreApplication::translate("QtCursorDatabase", "Split Vertical"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-vsplit.png"_L1));
    appendCursor(Qt::SplitHCursor, QCoreApplication::translate("QtCursorDatabase", "Split Horizontal"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-hsplit.png"_L1));
    appendCursor(Qt::PointingHandCursor, QCoreApplication::translate("QtCursorDatabase", "Pointing Hand"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-hand.png"_L1));
    appendCursor(Qt::ForbiddenCursor, QCoreApplication::translate("QtCursorDatabase", "Forbidden"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-forbidden.png"_L1));
    appendCursor(Qt::OpenHandCursor, QCoreApplication::translate("QtCursorDatabase", "Open Hand"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-openhand.png"_L1));
    appendCursor(Qt::ClosedHandCursor, QCoreApplication::translate("QtCursorDatabase", "Closed Hand"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-closedhand.png"_L1));
    appendCursor(Qt::WhatsThisCursor, QCoreApplication::translate("QtCursorDatabase", "What's This"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-whatsthis.png"_L1));
    appendCursor(Qt::BusyCursor, QCoreApplication::translate("QtCursorDatabase", "Busy"),
                 QIcon(":/qt-project.org/qtpropertybrowser/images/cursor-busy.png"_L1));
}

void QtCursorDatabase::clear()
{
    m_cursorNames.clear();
    m_cursorIcons.clear();
    m_valueToCursorShape.clear();
    m_cursorShapeToValue.clear();
}

void QtCursorDatabase::appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon)
{
    if (m_cursorShapeToValue.contains(shape))
        return;
    const int value = m_cursorNames.size();
    m_cursorNames.append(name);
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
    Qt::CursorShape shape = cursor.shape();
    if (m_cursorShapeToValue.contains(shape))
        return m_cursorShapeToValue[shape];
#endif
    return -1;
}

#ifndef QT_NO_CURSOR
QCursor QtCursorDatabase::valueToCursor(int value) const
{
    if (m_valueToCursorShape.contains(value))
        return QCursor(m_valueToCursorShape[value]);
    return {};
}
#endif

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

QtCursorDatabase *QtCursorDatabase::instance()
{
    return cursorDatabase();
}

QPixmap QtPropertyBrowserUtils::brushValuePixmap(const QBrush &b)
{
    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), b);
    QColor color = b.color();
    if (color.alpha() != 255) { // indicate alpha by an inset
        QBrush  opaqueBrush = b;
        color.setAlpha(255);
        opaqueBrush.setColor(color);
        painter.fillRect(img.width() / 4, img.height() / 4,
                         img.width() / 2, img.height() / 2, opaqueBrush);
    }
    painter.end();
    return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::brushValueIcon(const QBrush &b)
{
    return QIcon(brushValuePixmap(b));
}

QString QtPropertyBrowserUtils::colorValueText(const QColor &c)
{
    return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2, %3] (%4)")
           .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

QPixmap QtPropertyBrowserUtils::fontValuePixmap(const QFont &font)
{
    QFont f = font;
    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter p(&img);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::Antialiasing, true);
    f.setPointSize(13);
    p.setFont(f);
    QTextOption t;
    t.setAlignment(Qt::AlignCenter);
    p.drawText(QRect(0, 0, 16, 16), QString(QLatin1Char('A')), t);
    return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::fontValueIcon(const QFont &f)
{
    return QIcon(fontValuePixmap(f));
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

QT_END_NAMESPACE
