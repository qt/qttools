// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qpixeltool.h"

#include <qapplication.h>
#include <qdir.h>
#include <qscreen.h>
#if QT_CONFIG(clipboard)
#include <qclipboard.h>
#endif
#include <qpainter.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qmenu.h>
#include <qactiongroup.h>
#include <qimagewriter.h>
#include <qstandardpaths.h>
#include <qtextstream.h>
#include <qwindow.h>
#include <qmetaobject.h>
#include <private/qhighdpiscaling_p.h>

#include <qdebug.h>

#include <cmath>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto settingsGroup = "QPixelTool"_L1;
static constexpr auto organization = "QtProject"_L1;
static constexpr auto autoUpdateKey = "autoUpdate"_L1;
static constexpr auto gridSizeKey = "gridSize"_L1;
static constexpr auto gridActiveKey = "gridActive"_L1;
static constexpr auto zoomKey = "zoom"_L1;
static constexpr auto initialSizeKey = "initialSize"_L1;
static constexpr auto positionKey = "position"_L1;
static constexpr auto lcdModeKey = "lcdMode"_L1;

static QPoint initialPos(const QSettings &settings, QSize initialSize)
{
    const QPoint defaultPos = QGuiApplication::primaryScreen()->availableGeometry().topLeft();
    const QPoint savedPos =
        settings.value(positionKey, QVariant(defaultPos)).toPoint();
    auto *savedScreen = QGuiApplication::screenAt(savedPos);
    return savedScreen != nullptr
        && savedScreen->availableGeometry().intersects(QRect(savedPos, initialSize))
        ? savedPos : defaultPos;
}

QPixelTool::QPixelTool(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QCoreApplication::applicationName());
    QSettings settings(organization, settingsGroup);
    m_autoUpdate = settings.value(autoUpdateKey, 0).toBool();
    m_gridSize = settings.value(gridSizeKey, 1).toInt();
    m_gridActive = settings.value(gridActiveKey, 1).toInt();
    m_zoom = settings.value(zoomKey, 4).toInt();
    m_initialSize = settings.value(initialSizeKey, QSize(250, 200)).toSize();
    m_lcdMode = settings.value(lcdModeKey, 0).toInt();

    move(initialPos(settings, m_initialSize));

    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
    m_updateId = startTimer(30);
}

QPixelTool::~QPixelTool()
{
    QSettings settings(organization, settingsGroup);
    settings.setValue(autoUpdateKey, int(m_autoUpdate));
    settings.setValue(gridSizeKey, m_gridSize);
    settings.setValue(gridActiveKey, m_gridActive);
    settings.setValue(zoomKey, m_zoom);
    settings.setValue(initialSizeKey, size());
    settings.setValue(positionKey, pos());
    settings.setValue(lcdModeKey, m_lcdMode);
}

void QPixelTool::setPreviewImage(const QImage &image)
{
    m_preview_mode = true;
    m_preview_image = image;
    m_freeze = true;
}

void QPixelTool::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateId && !m_freeze) {
        grabScreen();
    } else if (event->timerId() == m_displayZoomId) {
        killTimer(m_displayZoomId);
        m_displayZoomId = 0;
        setZoomVisible(false);
    } else if (event->timerId() == m_displayGridSizeId) {
        killTimer(m_displayGridSizeId);
        m_displayGridSizeId = 0;
        m_displayGridSize = false;
    }
}

void render_string(QPainter *p, int w, int h, const QString &text, int flags)
{
    p->setBrush(QColor(255, 255, 255, 191));
    p->setPen(Qt::black);
    QRect bounds;
    p->drawText(0, 0, w, h, Qt::TextDontPrint | flags, text, &bounds);

    if (bounds.x() == 0) bounds.adjust(0, 0, 10, 0);
    else bounds.adjust(-10, 0, 0, 0);

    if (bounds.y() == 0) bounds.adjust(0, 0, 0, 10);
    else bounds.adjust(0, -10, 0, 0);

    p->drawRect(bounds);
    p->drawText(bounds, flags, text);
}

static QImage imageLCDFilter(const QImage &image, int lcdMode)
{
    Q_ASSERT(lcdMode > 0 && lcdMode < 5);
    const bool vertical = (lcdMode > 2);
    QImage scaled(image.width()  * (vertical ? 1 : 3),
                  image.height() * (vertical ? 3 : 1),
                  image.format());

    const int w = image.width();
    const int h = image.height();
    if (!vertical) {
        for (int y = 0; y < h; ++y) {
            const QRgb *in = reinterpret_cast<const QRgb *>(image.scanLine(y));
            QRgb *out = reinterpret_cast<QRgb *>(scaled.scanLine(y));
            if (lcdMode == 1) {
                for (int x = 0; x < w; ++x) {
                    *out++ = in[x] & 0xffff0000;
                    *out++ = in[x] & 0xff00ff00;
                    *out++ = in[x] & 0xff0000ff;
                }
            } else {
                for (int x = 0; x < w; ++x) {
                    *out++ = in[x] & 0xff0000ff;
                    *out++ = in[x] & 0xff00ff00;
                    *out++ = in[x] & 0xffff0000;
                }
            }
        }
    } else {
        for (int y = 0; y < h; ++y) {
            const QRgb *in = reinterpret_cast<const QRgb *>(image.scanLine(y));
            QRgb *out1 = reinterpret_cast<QRgb *>(scaled.scanLine(y * 3 + 0));
            QRgb *out2 = reinterpret_cast<QRgb *>(scaled.scanLine(y * 3 + 1));
            QRgb *out3 = reinterpret_cast<QRgb *>(scaled.scanLine(y * 3 + 2));
            if (lcdMode == 2) {
                for (int x = 0; x < w; ++x) {
                    out1[x] = in[x] & 0xffff0000;
                    out2[x] = in[x] & 0xff00ff00;
                    out3[x] = in[x] & 0xff0000ff;
                }
            } else {
                for (int x = 0; x < w; ++x) {
                    out1[x] = in[x] & 0xff0000ff;
                    out2[x] = in[x] & 0xff00ff00;
                    out3[x] = in[x] & 0xffff0000;
                }
            }
        }
    }
    return scaled;
}

void QPixelTool::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (m_preview_mode) {
        QPixmap pixmap(40, 40);
        QPainter pt(&pixmap);
        pt.fillRect(0, 0, 20, 20, Qt::white);
        pt.fillRect(20, 20, 20, 20, Qt::white);
        pt.fillRect(20, 0, 20, 20, Qt::lightGray);
        pt.fillRect(0, 20, 20, 20, Qt::lightGray);
        pt.end();
        p.fillRect(0, 0, width(), height(), pixmap);
    }

    int w = width();
    int h = height();

    p.save();
    if (m_lcdMode == 0)  {
        p.scale(m_zoom, m_zoom);
        p.drawPixmap(0, 0, m_buffer);
    } else {
        if (m_lcdMode <= 2)
            p.scale(m_zoom / 3.0, m_zoom);
        else
            p.scale(m_zoom, m_zoom / 3.0);
        p.drawImage(0, 0, imageLCDFilter(m_buffer.toImage(), m_lcdMode));
    }
    p.restore();

    // Draw the grid on top.
    if (m_gridActive) {
        p.setPen(m_gridActive == 1 ? Qt::black : Qt::white);
        int incr = m_gridSize * m_zoom;
        if (m_lcdMode == 0 || m_lcdMode > 2) {
            for (int x=0; x<w; x+=incr)
                p.drawLine(x, 0, x, h);
        }
        if (m_lcdMode <= 2) {
            for (int y=0; y<h; y+=incr)
                p.drawLine(0, y, w, y);
        }
    }

    QFont f(QStringList{u"courier"_s}, -1, QFont::Bold);
    p.setFont(f);

    if (m_displayZoom) {
        render_string(&p, w, h,
                      "Zoom: x"_L1 + QString::number(m_zoom),
                      Qt::AlignTop | Qt::AlignRight);
    }

    if (m_displayGridSize) {
        render_string(&p, w, h,
                      "Grid size: "_L1 + QString::number(m_gridSize),
                      Qt::AlignBottom | Qt::AlignLeft);
    }

    if (m_freeze) {
        QString str = QString::asprintf("%8X (%3d,%3d,%3d,%3d)",
                                        m_currentColor,
                                        (0xff000000 & m_currentColor) >> 24,
                                        (0x00ff0000 & m_currentColor) >> 16,
                                        (0x0000ff00 & m_currentColor) >> 8,
                                        (0x000000ff & m_currentColor));
        render_string(&p, w, h,
                      str,
                      Qt::AlignBottom | Qt::AlignRight);
    }

    if (m_mouseDown && m_dragStart != m_dragCurrent) {
        int x1 = (m_dragStart.x() / m_zoom) * m_zoom;
        int y1 = (m_dragStart.y() / m_zoom) * m_zoom;
        int x2 = (m_dragCurrent.x() / m_zoom) * m_zoom;
        int y2 = (m_dragCurrent.y() / m_zoom) * m_zoom;
        QRect r = QRect(x1, y1, x2 - x1, y2 - y1).normalized();
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
        p.drawRect(r);
        p.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        p.drawRect(r);

        QString str = QString::asprintf("Rect: x=%d, y=%d, w=%d, h=%d",
                                        r.x() / m_zoom,
                                        r.y() / m_zoom,
                                        r.width() / m_zoom,
                                        r.height() / m_zoom);
        render_string(&p, w, h, str, Qt::AlignBottom | Qt::AlignLeft);
    }


}

void QPixelTool::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Space:
        toggleFreeze();
        break;
    case Qt::Key_Plus:
        increaseZoom();
        break;
    case Qt::Key_Minus:
        decreaseZoom();
        break;
    case Qt::Key_PageUp:
        setGridSize(m_gridSize + 1);
        break;
    case Qt::Key_PageDown:
        setGridSize(m_gridSize - 1);
        break;
    case Qt::Key_G:
        toggleGrid();
        break;
    case Qt::Key_A:
        m_autoUpdate = !m_autoUpdate;
        break;
#if QT_CONFIG(clipboard)
    case Qt::Key_C:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            copyToClipboard();
        else
            copyColorToClipboard();
        break;
#endif // QT_CONFIG(clipboard)
    case Qt::Key_S:
        if (e->modifiers().testFlag(Qt::ControlModifier)) {
            releaseKeyboard();
            saveToFile();
        }
        break;
    case Qt::Key_Control:
        grabKeyboard();
        break;
    case Qt::Key_F1:
        aboutPixelTool();
        break;
    }
}

void QPixelTool::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Control:
        releaseKeyboard();
        break;
    default:
        break;
    }
}

void QPixelTool::resizeEvent(QResizeEvent *)
{
    grabScreen();
}

void QPixelTool::mouseMoveEvent(QMouseEvent *e)
{
    if (m_mouseDown)
        m_dragCurrent = e->pos();

    const auto pos = e->position().toPoint();
    const int x = pos.x() / m_zoom;
    const int y = pos.y() / m_zoom;

    QImage im = m_buffer.toImage().convertToFormat(QImage::Format_ARGB32);
    if (x < im.width() && y < im.height() && x >= 0 && y >= 0) {
        m_currentColor = im.pixel(x, y);
        update();
    }
}

void QPixelTool::mousePressEvent(QMouseEvent *e)
{
    if (!m_freeze)
        return;
    m_mouseDown = true;
    m_dragStart = e->pos();
}

void QPixelTool::mouseReleaseEvent(QMouseEvent *)
{
    m_mouseDown = false;
}

static QAction *addCheckableAction(QMenu &menu, const QString &title,
                                   bool value, const QKeySequence &key)
{
    QAction *result = menu.addAction(title);
    result->setCheckable(true);
    result->setChecked(value);
    result->setShortcut(key);
    return result;
}

static QAction *addCheckableAction(QMenu &menu, const QString &title,
                                   bool value, const QKeySequence &key,
                                   QActionGroup *group)
{
    QAction *result = addCheckableAction(menu, title, value, key);
    result->setActionGroup(group);
    return result;
}

void QPixelTool::contextMenuEvent(QContextMenuEvent *e)
{
    const bool tmpFreeze = m_freeze;
    m_freeze = true;

    QMenu menu;
    menu.addAction("Qt Pixel Zooming Tool"_L1)->setEnabled(false);
    menu.addSeparator();

    // Grid color options...
    auto *gridGroup = new QActionGroup(&menu);
    addCheckableAction(menu, "White grid"_L1, m_gridActive == 2,
                       Qt::Key_W, gridGroup);
    QAction *blackGrid = addCheckableAction(menu, "Black grid"_L1,
                                            m_gridActive == 1, Qt::Key_B, gridGroup);
    QAction *noGrid = addCheckableAction(menu, "No grid"_L1, m_gridActive == 0,
                                         Qt::Key_N, gridGroup);
    menu.addSeparator();

    // Grid size options
    menu.addAction("Increase grid size"_L1, Qt::Key_PageUp,
                   this, &QPixelTool::increaseGridSize);
    menu.addAction("Decrease grid size"_L1, Qt::Key_PageDown,
                   this, &QPixelTool::decreaseGridSize);
    menu.addSeparator();

    auto *lcdGroup = new QActionGroup(&menu);
    addCheckableAction(menu, "No subpixels"_L1, m_lcdMode == 0,
                       QKeySequence(), lcdGroup);
    QAction *rgbPixels = addCheckableAction(menu, "RGB subpixels"_L1,
                                            m_lcdMode == 1, QKeySequence(), lcdGroup);
    QAction *bgrPixels = addCheckableAction(menu, "BGR subpixels"_L1,
                                            m_lcdMode == 2, QKeySequence(), lcdGroup);
    QAction *vrgbPixels = addCheckableAction(menu, "VRGB subpixels"_L1,
                                             m_lcdMode == 3, QKeySequence(), lcdGroup);
    QAction *vbgrPixels = addCheckableAction(menu, "VBGR subpixels"_L1,
                                             m_lcdMode == 4, QKeySequence(), lcdGroup);
    menu.addSeparator();

    // Zoom options
    menu.addAction("Zoom in"_L1, Qt::Key_Plus,
                   this, &QPixelTool::increaseZoom);
    menu.addAction("Zoom out"_L1, Qt::Key_Minus,
                   this, &QPixelTool::decreaseZoom);
    menu.addSeparator();

    // Freeze / Autoupdate
    QAction *freeze = addCheckableAction(menu, "Frozen"_L1,
                                         tmpFreeze, Qt::Key_Space);
    QAction *autoUpdate = addCheckableAction(menu, "Continuous update"_L1,
                                             m_autoUpdate, Qt::Key_A);
    menu.addSeparator();

    // Copy to clipboard / save
    menu.addAction("Save as image..."_L1, QKeySequence::SaveAs,
                   this, &QPixelTool::saveToFile);
#if QT_CONFIG(clipboard)
    menu.addAction("Copy to clipboard"_L1, QKeySequence::Copy,
                   this, &QPixelTool::copyToClipboard);
    menu.addAction("Copy color value to clipboard"_L1, Qt::Key_C,
                   this, &QPixelTool::copyColorToClipboard);
    // Screen
    menu.addSeparator();
    menu.addAction("Copy screen shot to clipboard"_L1, QKeySequence(),
                   this, &QPixelTool::copyScreenShotToClipboard);
    menu.addAction("Copy screen info to clipboard"_L1, QKeySequence(),
                   this, &QPixelTool::copyScreenInfoToClipboard);
#endif // QT_CONFIG(clipboard)

    menu.addSeparator();
    menu.addAction("About Qt"_L1, qApp, &QApplication::aboutQt);
    menu.addAction("About Qt Pixeltool"_L1, this, &QPixelTool::aboutPixelTool);

    menu.exec(mapToGlobal(e->pos()));

    // Read out grid settings
    if (noGrid->isChecked())
        m_gridActive = 0;
    else if (blackGrid->isChecked())
        m_gridActive = 1;
    else
        m_gridActive = 2;

    // Read out lcd settings
    if (rgbPixels->isChecked())
        m_lcdMode = 1;
    else if (bgrPixels->isChecked())
        m_lcdMode = 2;
    else if (vrgbPixels->isChecked())
        m_lcdMode = 3;
    else if (vbgrPixels->isChecked())
        m_lcdMode = 4;
    else
        m_lcdMode = 0;

    m_autoUpdate = autoUpdate->isChecked();
    m_freeze = freeze->isChecked();

    // LCD mode looks off unless zoom is dividable by 3
    if (m_lcdMode && (m_zoom % 3) != 0)
        setZoom(qMax(3, (m_zoom + 1) / 3));
}

QSize QPixelTool::sizeHint() const
{
    return m_initialSize;
}

static inline QString pixelToolTitle(QPoint pos, const QScreen *screen, const QColor &currentColor)
{
    if (screen != nullptr)
        pos = QHighDpi::toNativePixels(pos, screen);
    return QCoreApplication::applicationName() + " ["_L1
        + QString::number(pos.x())
        + ", "_L1 + QString::number(pos.y()) + "] "_L1
        + currentColor.name();
}

void QPixelTool::grabScreen()
{
    if (m_preview_mode) {
        int w = qMin(width() / m_zoom + 1, m_preview_image.width());
        int h = qMin(height() / m_zoom + 1, m_preview_image.height());
        m_buffer = QPixmap::fromImage(m_preview_image).copy(0, 0, w, h);
        update();
        return;
    }

    QPoint mousePos = QCursor::pos();
    if (mousePos == m_lastMousePos && !m_autoUpdate)
        return;

    QScreen *screen = QGuiApplication::screenAt(mousePos);

    if (m_lastMousePos != mousePos)
        setWindowTitle(pixelToolTitle(mousePos, screen, m_currentColor));

    const auto widgetDpr = devicePixelRatioF();
    const auto screenDpr = screen != nullptr ? screen->devicePixelRatio() : widgetDpr;
    // When grabbing from another screen, we grab an area fitting our size using our DPR.
    const auto factor = widgetDpr / screenDpr / qreal(m_zoom);
    const QSize size{int(std::ceil(width() * factor)), int(std::ceil(height() * factor))};
    const QPoint pos = mousePos - QPoint{size.width(), size.height()} / 2;

    const QBrush darkBrush = palette().color(QPalette::Dark);
    if (screen != nullptr) {
        const QPoint screenPos = pos - screen->geometry().topLeft();
        m_buffer = screen->grabWindow(0, screenPos.x(), screenPos.y(), size.width(), size.height());
    } else {
        m_buffer = QPixmap(size);
        m_buffer.fill(darkBrush.color());
    }
    m_buffer.setDevicePixelRatio(widgetDpr);

    QRegion geom(QRect{pos, size});
    QRect screenRect;
    const auto screens = QGuiApplication::screens();
    for (auto *screen : screens)
        screenRect |= screen->geometry();
    geom -= screenRect;
    const auto rectsInRegion = geom.rectCount();
    if (!geom.isEmpty()) {
        QPainter p(&m_buffer);
        p.translate(-pos);
        p.setPen(Qt::NoPen);
        p.setBrush(darkBrush);
        p.drawRects(geom.begin(), rectsInRegion);
    }

    update();

    m_currentColor = m_buffer.toImage().pixel(m_buffer.rect().center());
    m_lastMousePos = mousePos;
}

void QPixelTool::startZoomVisibleTimer()
{
    if (m_displayZoomId > 0)
        killTimer(m_displayZoomId);
    m_displayZoomId = startTimer(5000);
    setZoomVisible(true);
}

void QPixelTool::startGridSizeVisibleTimer()
{
    if (m_gridActive) {
        if (m_displayGridSizeId > 0)
            killTimer(m_displayGridSizeId);
        m_displayGridSizeId = startTimer(5000);
        m_displayGridSize = true;
        update();
    }
}

void QPixelTool::setZoomVisible(bool visible)
{
    m_displayZoom = visible;
    update();
}

void QPixelTool::toggleFreeze()
{
    m_freeze = !m_freeze;
    if (!m_freeze)
        m_dragStart = m_dragCurrent = QPoint();
}

void QPixelTool::increaseZoom()
{
    if (!m_lcdMode)
        setZoom(m_zoom + 1);
    else
        setZoom(m_zoom + 3);
}

void QPixelTool::decreaseZoom()
{
    if (!m_lcdMode)
        setZoom(m_zoom - 1);
    else
        setZoom(m_zoom - 3);
}

void QPixelTool::setZoom(int zoom)
{
    if (zoom > 0) {
        QPoint pos = m_lastMousePos;
        m_lastMousePos = QPoint();
        m_zoom = zoom;
        grabScreen();
        m_lastMousePos = pos;
        m_dragStart = m_dragCurrent = QPoint();
        startZoomVisibleTimer();
    }
}

void QPixelTool::toggleGrid()
{
    if (++m_gridActive > 2)
        m_gridActive = 0;
    update();
}

void QPixelTool::setGridSize(int gridSize)
{
    if (m_gridActive && gridSize > 0) {
        m_gridSize = gridSize;
        startGridSizeVisibleTimer();
        update();
    }
}

#if QT_CONFIG(clipboard)
void QPixelTool::copyToClipboard()
{
    QGuiApplication::clipboard()->setPixmap(m_buffer);
}

void QPixelTool::copyColorToClipboard()
{
    QGuiApplication::clipboard()->setText(QColor(m_currentColor).name());
}

void QPixelTool::copyScreenShotToClipboard()
{
    QPixmap screenShot = screen()->grabWindow();
    QGuiApplication::clipboard()->setImage(screenShot.toImage());
}

void QPixelTool::copyScreenInfoToClipboard()
{
    const auto *screen = this->screen();
    QString text;
    QTextStream str(&text);
    const auto geom = screen->geometry();
    const auto availGeom = screen->availableGeometry();
    auto orientationMt = QMetaEnum::fromType<Qt::ScreenOrientation>();

    str << "Model/name: \"" << screen->model() << "\"/\"" << screen->name() << '"'
        << "\nGeometry: " << geom.width() << 'x' << geom.height() << Qt::forcesign
        << geom.x() << geom.y() << Qt::noforcesign
        << "\nAvailable geometry: " << availGeom.width() << 'x' << availGeom.height() << Qt::forcesign
        << availGeom.x() << availGeom.y() << Qt::noforcesign
        << "\nDevice pixel ratio: " << screen->devicePixelRatio()
        << "\nLogical DPI: " << screen->logicalDotsPerInchX() << ','
        << screen->logicalDotsPerInchY() << "DPI"
        << "\nPhysical DPI: " << screen->physicalDotsPerInchX() << ','
        << screen->physicalDotsPerInchY() << "DPI"
        << "\nPhysical size: " << screen->physicalSize().width()  << 'x'
        << screen->physicalSize().height() << "mm";
    if (const char *orientation = orientationMt.valueToKey(screen->orientation()))
        str << "\nOrientation: " << orientation;
    str << "\nRefresh rate: " << screen->refreshRate() << "Hz";
    QGuiApplication::clipboard()->setText(text);
}

#endif // QT_CONFIG(clipboard)

void QPixelTool::saveToFile()
{
    bool oldFreeze = m_freeze;
    m_freeze = true;

    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle("Save as image"_L1);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    QStringList mimeTypes;
    const QByteArrayList supportedMimeTypes = QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeB : supportedMimeTypes)
        mimeTypes.append(QString::fromLatin1(mimeTypeB));
    fileDialog.setMimeTypeFilters(mimeTypes);
    const QString pngType = "image/png"_L1;
    if (mimeTypes.contains(pngType)) {
        fileDialog.selectMimeTypeFilter(pngType);
        fileDialog.setDefaultSuffix("png"_L1);
    }

    while (fileDialog.exec() == QDialog::Accepted
        && !m_buffer.save(fileDialog.selectedFiles().constFirst())) {
        QMessageBox::warning(this, "Unable to write image"_L1,
                             "Unable to write "_L1
                             + QDir::toNativeSeparators(fileDialog.selectedFiles().constFirst()));
    }
    m_freeze = oldFreeze;
}

QTextStream &operator<<(QTextStream &str, const QScreen *screen)
{
    const QRect geometry = screen->geometry();
    str << '"' << screen->name() << "\" " << geometry.width()
        << 'x' << geometry.height() << Qt::forcesign << geometry.x() << geometry.y()
        << Qt::noforcesign << ", " << qRound(screen->logicalDotsPerInch()) << "DPI"
        << ", Depth: " << screen->depth() << ", " << screen->refreshRate() << "Hz";
    const qreal dpr = screen->devicePixelRatio();
    if (!qFuzzyCompare(dpr, qreal(1)))
        str << ", DPR: " << dpr;
    return str;
}

QString QPixelTool::aboutText() const
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    const QScreen *windowScreen = windowHandle()->screen();

    QString result;
    QTextStream str(&result);
    str << "<html><head></head><body><h2>Qt Pixeltool</h2><p>Qt " << QT_VERSION_STR
        << "</p><p>Copyright (C) 2017 The Qt Company Ltd.</p><h3>Screens</h3><ul>";
    for (const QScreen *screen : screens)
        str << "<li>" << (screen == windowScreen ? "* " : "  ") << screen << "</li>";
    str << "</ul></body></html>";
    return result;
}

void QPixelTool::aboutPixelTool()
{
    QMessageBox aboutBox(QMessageBox::Information, tr("About Qt Pixeltool"), aboutText(),
                         QMessageBox::Close, this);
    aboutBox.setWindowFlags(aboutBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    aboutBox.setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutBox.exec();
}

QT_END_NAMESPACE
