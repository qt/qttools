// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "formwindowbase_p.h"
#include "previewmanager_p.h"
#include "qdesigner_formbuilder_p.h"
#include "shared_settings_p.h"
#include "widgetfactory_p.h"
#include "zoomwidget_p.h"

#include <deviceskin_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractsettings.h>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qmenu.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qscreen.h>
#include <QtGui/qtransform.h>

#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static inline int compare(const qdesigner_internal::PreviewConfiguration &pc1, const qdesigner_internal::PreviewConfiguration &pc2)
{
    int rc = pc1.style().compare(pc2.style());
    if (rc)
        return rc;
    rc = pc1.applicationStyleSheet().compare(pc2.applicationStyleSheet());
    if (rc)
        return rc;
    return pc1.deviceSkin().compare(pc2.deviceSkin());
}

namespace qdesigner_internal {
    // ------ PreviewData (data associated with a preview window)
    struct PreviewData {
        PreviewData(const QPointer<QWidget> &widget, const  QDesignerFormWindowInterface *formWindow, const qdesigner_internal::PreviewConfiguration &pc);
        QPointer<QWidget> m_widget;
        const QDesignerFormWindowInterface *m_formWindow;
        qdesigner_internal::PreviewConfiguration m_configuration;
    };

    PreviewData::PreviewData(const QPointer<QWidget>& widget,
                             const QDesignerFormWindowInterface *formWindow,
                             const qdesigner_internal::PreviewConfiguration &pc) :
        m_widget(widget),
        m_formWindow(formWindow),
        m_configuration(pc)
    {
    }

/* In designer, we have the situation that laid-out maincontainers have
 * a geometry set (which might differ from their sizeHint()). The QGraphicsItem
 * should return that in its size hint, else such cases won't work */

class DesignerZoomProxyWidget : public ZoomProxyWidget  {
    Q_DISABLE_COPY_MOVE(DesignerZoomProxyWidget)
public:
    DesignerZoomProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = {});
protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF() ) const override;
};

DesignerZoomProxyWidget::DesignerZoomProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags) :
    ZoomProxyWidget(parent, wFlags)
{
}

QSizeF DesignerZoomProxyWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (const QWidget *w = widget())
            return QSizeF(w->size());
    return ZoomProxyWidget::sizeHint(which, constraint);
}

// DesignerZoomWidget which returns DesignerZoomProxyWidget in its factory function
class DesignerZoomWidget : public ZoomWidget {
    Q_DISABLE_COPY_MOVE(DesignerZoomWidget)
public:
    DesignerZoomWidget(QWidget *parent = nullptr);
private:
    QGraphicsProxyWidget *createProxyWidget(QGraphicsItem *parent = nullptr,
                                            Qt::WindowFlags wFlags = {}) const override;
};

DesignerZoomWidget::DesignerZoomWidget(QWidget *parent) :
    ZoomWidget(parent)
{
}

QGraphicsProxyWidget *DesignerZoomWidget::createProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags) const
{
    return new DesignerZoomProxyWidget(parent, wFlags);
}

// PreviewDeviceSkin: Forwards the key events to the window and
// provides context menu with rotation options. Derived class
// can apply additional transformations to the skin.

class PreviewDeviceSkin : public  DeviceSkin
{
    Q_OBJECT
public:
    enum Direction { DirectionUp, DirectionLeft,  DirectionRight };

    explicit PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent);
    virtual void setPreview(QWidget *w);
    QSize screenSize() const { return  m_screenSize; }

private slots:
    void slotSkinKeyPressEvent(int code, const QString& text, bool autorep);
    void slotSkinKeyReleaseEvent(int code, const QString& text, bool autorep);
    void slotPopupMenu();

protected:
    virtual void populateContextMenu(QMenu *) {}

private slots:
    void slotDirection(QAction *);

protected:
    // Fit the widget in case the orientation changes (transposing screensize)
    virtual void fitWidget(const QSize &size);
    //  Calculate the complete transformation for the skin
    // (base class implementation provides rotation).
    virtual QTransform skinTransform() const;

private:
    const QSize m_screenSize;
    Direction m_direction;

    QAction *m_directionUpAction;
    QAction *m_directionLeftAction;
    QAction *m_directionRightAction;
    QAction *m_closeAction;
};

PreviewDeviceSkin::PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent) :
    DeviceSkin(parameters, parent),
    m_screenSize(parameters.screenSize()),
    m_direction(DirectionUp),
    m_directionUpAction(nullptr),
    m_directionLeftAction(nullptr),
    m_directionRightAction(nullptr),
    m_closeAction(nullptr)
{
    connect(this, &PreviewDeviceSkin::skinKeyPressEvent,
            this, &PreviewDeviceSkin::slotSkinKeyPressEvent);
    connect(this, &PreviewDeviceSkin::skinKeyReleaseEvent,
            this, &PreviewDeviceSkin::slotSkinKeyReleaseEvent);
    connect(this, &PreviewDeviceSkin::popupMenu, this, &PreviewDeviceSkin::slotPopupMenu);
}

void PreviewDeviceSkin::setPreview(QWidget *formWidget)
{
    formWidget->setFixedSize(m_screenSize);
    formWidget->setParent(this, Qt::SubWindow);
    formWidget->setAutoFillBackground(true);
    setView(formWidget);
}

void PreviewDeviceSkin::slotSkinKeyPressEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyPress, code, {}, text, autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}

void PreviewDeviceSkin::slotSkinKeyReleaseEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyRelease, code, {}, text, autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}

// Create a checkable action with integer data and
// set it checked if it matches the currentState.
static inline QAction
        *createCheckableActionIntData(const QString &label,
                                      int actionValue, int currentState,
                                      QActionGroup *ag, QObject *parent)
{
    QAction *a = new QAction(label, parent);
    a->setData(actionValue);
    a->setCheckable(true);
    if (actionValue == currentState)
        a->setChecked(true);
    ag->addAction(a);
    return a;
}

void PreviewDeviceSkin::slotPopupMenu()
{
    QMenu menu(this);
    // Create actions
    if (!m_directionUpAction) {
        QActionGroup *directionGroup = new QActionGroup(this);
        connect(directionGroup, &QActionGroup::triggered, this, &PreviewDeviceSkin::slotDirection);
        directionGroup->setExclusive(true);
        m_directionUpAction = createCheckableActionIntData(tr("&Portrait"), DirectionUp, m_direction, directionGroup, this);
        //: Rotate form preview counter-clockwise
        m_directionLeftAction = createCheckableActionIntData(tr("Landscape (&CCW)"), DirectionLeft, m_direction, directionGroup, this);
        //: Rotate form preview clockwise
        m_directionRightAction = createCheckableActionIntData(tr("&Landscape (CW)"), DirectionRight, m_direction, directionGroup, this);
        m_closeAction = new QAction(tr("&Close"), this);
        connect(m_closeAction, &QAction::triggered, parentWidget(), &QWidget::close);
    }
    menu.addAction(m_directionUpAction);
    menu.addAction(m_directionLeftAction);
    menu.addAction(m_directionRightAction);
    menu.addSeparator();
    populateContextMenu(&menu);
    menu.addAction(m_closeAction);
    menu.exec(QCursor::pos());
}

void PreviewDeviceSkin::slotDirection(QAction *a)
{
    const Direction newDirection = static_cast<Direction>(a->data().toInt());
    if (m_direction == newDirection)
        return;
    const Qt::Orientation newOrientation = newDirection == DirectionUp ? Qt::Vertical : Qt::Horizontal;
    const Qt::Orientation oldOrientation = m_direction  == DirectionUp ? Qt::Vertical : Qt::Horizontal;
    m_direction = newDirection;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (oldOrientation != newOrientation) {
        QSize size = screenSize();
        if (newOrientation == Qt::Horizontal)
            size.transpose();
        fitWidget(size);
    }
    setTransform(skinTransform());
    QApplication::restoreOverrideCursor();
}

void PreviewDeviceSkin::fitWidget(const QSize &size)
{
    view()->setFixedSize(size);
}

QTransform PreviewDeviceSkin::skinTransform() const
{
    QTransform newTransform;
    switch (m_direction)  {
        case DirectionUp:
            break;
        case DirectionLeft:
            newTransform.rotate(270.0);
            break;
        case DirectionRight:
            newTransform.rotate(90.0);
            break;
    }
    return newTransform;
}

// ------------ PreviewConfigurationPrivate
class PreviewConfigurationData : public QSharedData {
public:
    PreviewConfigurationData() = default;
    explicit PreviewConfigurationData(const QString &style, const QString &applicationStyleSheet, const QString &deviceSkin);

    QString m_style;
    // Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
    QString m_applicationStyleSheet;
    QString m_deviceSkin;
};

PreviewConfigurationData::PreviewConfigurationData(const QString &style, const QString &applicationStyleSheet, const QString &deviceSkin) :
    m_style(style),
    m_applicationStyleSheet(applicationStyleSheet),
    m_deviceSkin(deviceSkin)
{
}

/* ZoomablePreviewDeviceSkin: A Zoomable Widget Preview skin. Embeds preview
 *  into a ZoomWidget and this in turn into the DeviceSkin view and keeps
 * Device skin zoom + ZoomWidget zoom in sync. */

class ZoomablePreviewDeviceSkin : public PreviewDeviceSkin
{
    Q_OBJECT
public:
    explicit ZoomablePreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent);
    void setPreview(QWidget *w) override;

    int zoomPercent() const; // Device Skins have a double 'zoom' property

public slots:
    void setZoomPercent(int);

signals:
    void zoomPercentChanged(int);

protected:
    void populateContextMenu(QMenu *m) override;
    QTransform skinTransform() const override;
    void fitWidget(const QSize &size) override;

private:
    ZoomMenu *m_zoomMenu;
    QAction *m_zoomSubMenuAction;
    ZoomWidget *m_zoomWidget;
};

ZoomablePreviewDeviceSkin::ZoomablePreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent) :
    PreviewDeviceSkin(parameters, parent),
    m_zoomMenu(new ZoomMenu(this)),
    m_zoomSubMenuAction(nullptr),
    m_zoomWidget(new DesignerZoomWidget)
{
    connect(m_zoomMenu, &ZoomMenu::zoomChanged, this, &ZoomablePreviewDeviceSkin::setZoomPercent);
    connect(m_zoomMenu, &ZoomMenu::zoomChanged, this, &ZoomablePreviewDeviceSkin::zoomPercentChanged);
    m_zoomWidget->setZoomContextMenuEnabled(false);
    m_zoomWidget->setWidgetZoomContextMenuEnabled(false);
    m_zoomWidget->resize(screenSize());
    m_zoomWidget->setParent(this, Qt::SubWindow);
    m_zoomWidget->setAutoFillBackground(true);
    setView(m_zoomWidget);
}

static inline qreal zoomFactor(int percent)
{
    return qreal(percent) / 100.0;
}

static inline QSize scaleSize(int zoomPercent, const QSize &size)
{
    return zoomPercent == 100 ? size : (QSizeF(size) * zoomFactor(zoomPercent)).toSize();
}

void ZoomablePreviewDeviceSkin::setPreview(QWidget *formWidget)
{
    m_zoomWidget->setWidget(formWidget);
    m_zoomWidget->resize(scaleSize(zoomPercent(), screenSize()));
}

int ZoomablePreviewDeviceSkin::zoomPercent() const
{
    return m_zoomWidget->zoom();
}

void ZoomablePreviewDeviceSkin::setZoomPercent(int zp)
{
    if (zp == zoomPercent())
        return;

    // If not triggered by the menu itself: Update it
    if (m_zoomMenu->zoom() != zp)
        m_zoomMenu->setZoom(zp);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_zoomWidget->setZoom(zp);
    setTransform(skinTransform());
    QApplication::restoreOverrideCursor();
}

void ZoomablePreviewDeviceSkin::populateContextMenu(QMenu *menu)
{
    if (!m_zoomSubMenuAction) {
        m_zoomSubMenuAction = new QAction(tr("&Zoom"), this);
        QMenu *zoomSubMenu = new QMenu;
        m_zoomSubMenuAction->setMenu(zoomSubMenu);
        m_zoomMenu->addActions(zoomSubMenu);
    }
    menu->addAction(m_zoomSubMenuAction);
    menu->addSeparator();
}

QTransform ZoomablePreviewDeviceSkin::skinTransform() const
{
    // Complete transformation consisting of base class rotation and zoom.
    QTransform rc = PreviewDeviceSkin::skinTransform();
    const int zp = zoomPercent();
    if (zp != 100) {
        const qreal factor = zoomFactor(zp);
        rc.scale(factor, factor);
    }
    return rc;
}

void ZoomablePreviewDeviceSkin::fitWidget(const QSize &size)
{
    m_zoomWidget->resize(scaleSize(zoomPercent(), size));
}

// ------------- PreviewConfiguration

static constexpr auto styleKey = "Style"_L1;
static constexpr auto appStyleSheetKey = "AppStyleSheet"_L1;
static constexpr auto skinKey = "Skin"_L1;

PreviewConfiguration::PreviewConfiguration() :
    m_d(new PreviewConfigurationData)
{
}

PreviewConfiguration::PreviewConfiguration(const QString &sty, const QString &applicationSheet, const QString &skin) :
    m_d(new PreviewConfigurationData(sty, applicationSheet, skin))
{
}

PreviewConfiguration::PreviewConfiguration(const PreviewConfiguration &o) :
    m_d(o.m_d)
{
}

PreviewConfiguration &PreviewConfiguration::operator=(const PreviewConfiguration &o)
{
    m_d.operator=(o.m_d);
    return *this;
}

PreviewConfiguration::~PreviewConfiguration() = default;

void PreviewConfiguration::clear()
{
    PreviewConfigurationData &d = *m_d;
    d.m_style.clear();
    d.m_applicationStyleSheet.clear();
    d.m_deviceSkin.clear();
}

QString PreviewConfiguration::style() const
{
    return m_d->m_style;
}

void PreviewConfiguration::setStyle(const QString &s)
{
    m_d->m_style = s;
}

// Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
QString PreviewConfiguration::applicationStyleSheet() const
{
    return m_d->m_applicationStyleSheet;
}

void PreviewConfiguration::setApplicationStyleSheet(const QString &as)
{
    m_d->m_applicationStyleSheet = as;
}

QString PreviewConfiguration::deviceSkin() const
{
    return m_d->m_deviceSkin;
}

void PreviewConfiguration::setDeviceSkin(const QString &s)
{
     m_d->m_deviceSkin = s;
}

void PreviewConfiguration::toSettings(const QString &prefix, QDesignerSettingsInterface *settings) const
{
    const PreviewConfigurationData &d = *m_d;
    settings->beginGroup(prefix);
    settings->setValue(styleKey, d.m_style);
    settings->setValue(appStyleSheetKey, d.m_applicationStyleSheet);
    settings->setValue(skinKey, d.m_deviceSkin);
    settings->endGroup();
}

void PreviewConfiguration::fromSettings(const QString &prefix, const QDesignerSettingsInterface *settings)
{
    clear();
    QString key = prefix + u'/';
    const auto prefixSize = key.size();

    PreviewConfigurationData &d = *m_d;

    const QVariant emptyString = QVariant(QString());

    key += styleKey;
    d.m_style = settings->value(key, emptyString).toString();

    key.replace(prefixSize, key.size() - prefixSize, appStyleSheetKey);
    d.m_applicationStyleSheet = settings->value(key, emptyString).toString();

    key.replace(prefixSize, key.size() - prefixSize, skinKey);
    d.m_deviceSkin = settings->value(key, emptyString).toString();
}


QDESIGNER_SHARED_EXPORT bool operator<(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) < 0;
}

QDESIGNER_SHARED_EXPORT bool operator==(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) == 0;
}

QDESIGNER_SHARED_EXPORT bool operator!=(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) != 0;
}

// ------------- PreviewManagerPrivate
class PreviewManagerPrivate {
public:
    PreviewManagerPrivate(PreviewManager::PreviewMode mode);

    const PreviewManager::PreviewMode m_mode;

    QPointer<QWidget> m_activePreview;

    using PreviewDataList = QList<PreviewData>;

    PreviewDataList m_previews;

    QMap<QString, DeviceSkinParameters> m_deviceSkinConfigCache;

    QDesignerFormEditorInterface *m_core;
    bool m_updateBlocked;
};

PreviewManagerPrivate::PreviewManagerPrivate(PreviewManager::PreviewMode mode) :
    m_mode(mode),
    m_core(nullptr),
    m_updateBlocked(false)
{
}

// ------------- PreviewManager

PreviewManager::PreviewManager(PreviewMode mode, QObject *parent) :
   QObject(parent),
   d(new PreviewManagerPrivate(mode))
{
}

PreviewManager:: ~PreviewManager()
{
    delete d;
}


Qt::WindowFlags PreviewManager::previewWindowFlags(const QWidget *widget) const
{
#ifdef Q_OS_WIN
    Qt::WindowFlags windowFlags = (widget->windowType() == Qt::Window) ?
                                  Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint :
                                  Qt::WindowFlags(Qt::Dialog);
#else
    Q_UNUSED(widget);
    // Only Dialogs have close buttons on Mac.
    // On Linux, we don't want an additional task bar item and we don't want a minimize button;
    // we want the preview to be on top.
    Qt::WindowFlags windowFlags = Qt::Dialog;
#endif
    return windowFlags;
}

QWidget *PreviewManager::createDeviceSkinContainer(const QDesignerFormWindowInterface *fw) const
{
    return new QDialog(fw->window());
}

// Some widgets might require fake containers

static QWidget *fakeContainer(QWidget *w)
{
    // Prevent a dock widget from trying to dock to Designer's main window
    // (which can be found in the parent hierarchy in MDI mode) by
    // providing a fake mainwindow
    if (QDockWidget *dock = qobject_cast<QDockWidget *>(w)) {
        // Reparent: Clear modality, propagate title and resize outer container
        const QSize size = w->size();
        w->setWindowModality(Qt::NonModal);
        dock->setFeatures(dock->features() & ~(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable));
        dock->setAllowedAreas(Qt::LeftDockWidgetArea);
        QMainWindow *mw = new QMainWindow;
        const QMargins cm = mw->contentsMargins();
        mw->addDockWidget(Qt::LeftDockWidgetArea, dock);
        mw->resize(size + QSize(cm.left() + cm.right(), cm.top() + cm.bottom()));
        return mw;
    }
    return w;
}

static PreviewConfiguration configurationFromSettings(QDesignerFormEditorInterface *core, const QString &style)
{
    qdesigner_internal::PreviewConfiguration pc;
    const QDesignerSharedSettings settings(core);
    if (settings.isCustomPreviewConfigurationEnabled())
        pc = settings.customPreviewConfiguration();
    if (!style.isEmpty())
        pc.setStyle(style);
    return pc;
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw, const QString &style, int deviceProfileIndex, QString *errorMessage)
{
    return showPreview(fw, configurationFromSettings(fw->core(), style), deviceProfileIndex, errorMessage);
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw, const QString &style, QString *errorMessage)
{
    return showPreview(fw, style, -1, errorMessage);
}

QWidget *PreviewManager::createPreview(const QDesignerFormWindowInterface *fw,
                                       const PreviewConfiguration &pc,
                                       int deviceProfileIndex,
                                       QString *errorMessage,
                                       int initialZoom)
{
    if (!d->m_core)
        d->m_core = fw->core();

    const bool zoomable = initialZoom > 0;
    // Figure out which profile to apply
    DeviceProfile deviceProfile;
    if (deviceProfileIndex >= 0) {
        deviceProfile = QDesignerSharedSettings(fw->core()).deviceProfileAt(deviceProfileIndex);
    } else {
        if (const FormWindowBase *fwb = qobject_cast<const FormWindowBase *>(fw))
            deviceProfile = fwb->deviceProfile();
    }
    // Create
    QWidget *formWidget = QDesignerFormBuilder::createPreview(fw, pc.style(), pc.applicationStyleSheet(), deviceProfile, errorMessage);
    if (!formWidget)
        return nullptr;

    const QString title = tr("%1 - [Preview]").arg(formWidget->windowTitle());
    formWidget = fakeContainer(formWidget);
    formWidget->setWindowTitle(title);

    // Clear any modality settings, child widget modalities must not be higher than parent's
    formWidget->setWindowModality(Qt::NonModal);
    // No skin
    const QString deviceSkin = pc.deviceSkin();
    if (deviceSkin.isEmpty()) {
        if (zoomable) { // Embed into ZoomWidget
            ZoomWidget *zw = new DesignerZoomWidget;
            connect(zw->zoomMenu(), &ZoomMenu::zoomChanged, this, &PreviewManager::slotZoomChanged);
            zw->setWindowTitle(title);
            zw->setWidget(formWidget);
            // Keep any widgets' context menus working, do not use global menu
            zw->setWidgetZoomContextMenuEnabled(true);
            zw->setParent(fw->window(), previewWindowFlags(formWidget));
            // Make preview close when Widget closes (Dialog/accept, etc)
            formWidget->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(formWidget, &QObject::destroyed, zw, &QWidget::close);
            zw->setZoom(initialZoom);
            zw->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
            return zw;
        }
        formWidget->setParent(fw->window(), previewWindowFlags(formWidget));
        formWidget->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
        return formWidget;
    }
    // Embed into skin. find config in cache
    auto it = d->m_deviceSkinConfigCache.find(deviceSkin);
    if (it == d->m_deviceSkinConfigCache.end()) {
        DeviceSkinParameters parameters;
        if (!parameters.read(deviceSkin, DeviceSkinParameters::ReadAll, errorMessage)) {
            formWidget->deleteLater();
            return nullptr;
          }
        it = d->m_deviceSkinConfigCache.insert(deviceSkin, parameters);
    }

    QWidget *skinContainer = createDeviceSkinContainer(fw);
    PreviewDeviceSkin *skin = nullptr;
    if (zoomable) {
        ZoomablePreviewDeviceSkin *zds = new ZoomablePreviewDeviceSkin(it.value(), skinContainer);
        zds->setZoomPercent(initialZoom);
        connect(zds, &ZoomablePreviewDeviceSkin::zoomPercentChanged,
                this, &PreviewManager::slotZoomChanged);
        skin = zds;
    }  else {
        skin = new PreviewDeviceSkin(it.value(), skinContainer);
    }
    skin->setPreview(formWidget);
    // Make preview close when Widget closes (Dialog/accept, etc)
    formWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(formWidget, &QObject::destroyed, skinContainer, &QWidget::close);
    skinContainer->setWindowTitle(title);
    skinContainer->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
    return skinContainer;
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw,
                                     const PreviewConfiguration &pc,
                                     int deviceProfileIndex,
                                     QString *errorMessage)
{
    enum { Spacing = 10 };
    if (QWidget *existingPreviewWidget = raise(fw, pc))
        return existingPreviewWidget;

    const QDesignerSharedSettings settings(fw->core());
    const int initialZoom = settings.zoomEnabled() ? settings.zoom() : -1;

    QWidget *widget = createPreview(fw, pc, deviceProfileIndex, errorMessage, initialZoom);
    if (!widget)
        return nullptr;
    // Install filter for Escape key
    widget->setAttribute(Qt::WA_DeleteOnClose, true);
    widget->installEventFilter(this);

    switch (d->m_mode) {
    case ApplicationModalPreview:
        // Cannot do this on the Mac as the dialog would have no close button
        widget->setWindowModality(Qt::ApplicationModal);
        break;
    case SingleFormNonModalPreview:
    case MultipleFormNonModalPreview:
        widget->setWindowModality(Qt::NonModal);
        connect(fw, &QDesignerFormWindowInterface::changed, widget, &QWidget::close);
        connect(fw, &QObject::destroyed, widget, &QWidget::close);
        if (d->m_mode == SingleFormNonModalPreview) {
            connect(fw->core()->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
                    widget, &QWidget::close);
        }
        break;
    }
    // Semi-smart algorithm to position previews:
    // If its the first one, position relative to form.
    // 2nd, attempt to tile right (for comparing styles) or cascade
    const QSize size = widget->size();
    const bool firstPreview = d->m_previews.isEmpty();
    if (firstPreview) {
        widget->move(fw->mapToGlobal(QPoint(Spacing, Spacing)));
    } else {
        if (QWidget *lastPreview = d->m_previews.constLast().m_widget) {
            const QRect lastPreviewGeometry = lastPreview->frameGeometry();
            const QRect availGeometry = lastPreview->screen()->availableGeometry();
            const QPoint newPos = lastPreviewGeometry.topRight() + QPoint(Spacing, 0);
            if (newPos.x() +  size.width() < availGeometry.right())
                widget->move(newPos);
            else
                widget->move(lastPreviewGeometry.topLeft() + QPoint(Spacing, Spacing));
        }

    }
    d->m_previews.push_back(PreviewData(widget, fw, pc));
    widget->show();
    if (firstPreview)
        emit firstPreviewOpened();
    return widget;
}

QWidget *PreviewManager::raise(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc)
{
    if (d->m_previews.isEmpty())
        return nullptr;

    // find matching window
    for (const auto &pd : std::as_const(d->m_previews)) {
        QWidget *w = pd.m_widget;
        if (w && pd.m_formWindow == fw && pd.m_configuration == pc) {
            w->raise();
            w->activateWindow();
            return w;
        }
    }
    return nullptr;
}

void PreviewManager::closeAllPreviews()
{
    if (!d->m_previews.isEmpty()) {
        d->m_updateBlocked = true;
        d->m_activePreview = nullptr;
        for (const auto &pd : std::as_const(d->m_previews)) {
            if (pd.m_widget)
                pd.m_widget->close();
        }
        d->m_previews.clear();
        d->m_updateBlocked = false;
        emit lastPreviewClosed();
    }
}

void PreviewManager::updatePreviewClosed(QWidget *w)
{
    if (d->m_updateBlocked)
        return;
    // Purge out all 0 or widgets to be deleted
    for (auto it = d->m_previews.begin(); it != d->m_previews.end() ; ) {
        QWidget *iw = it->m_widget; // Might be 0 when catching QEvent::Destroyed
        if (iw == nullptr || iw == w) {
            it = d->m_previews.erase(it);
        } else {
            ++it;
        }
    }
    if (d->m_previews.isEmpty())
        emit lastPreviewClosed();
}

bool PreviewManager::eventFilter(QObject *watched, QEvent *event)
{
    // Courtesy of designer
    do {
        if (!watched->isWidgetType())
            break;
        QWidget *previewWindow = qobject_cast<QWidget *>(watched);
        if (!previewWindow || !previewWindow->isWindow())
            break;

        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::ShortcutOverride:        {
            const  QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
            const int key = keyEvent->key();
            if ((key == Qt::Key_Escape
#ifdef Q_OS_MACOS
                 || (keyEvent->modifiers() == Qt::ControlModifier && key == Qt::Key_Period)
#endif
                 )) {
                previewWindow->close();
                return true;
            }
        }
            break;
        case QEvent::WindowActivate:
            d->m_activePreview = previewWindow;
            break;
        case  QEvent::Destroy: // We don't get QEvent::Close if someone accepts a QDialog.
            updatePreviewClosed(previewWindow);
            break;
        case  QEvent::Close:
            updatePreviewClosed(previewWindow);
            previewWindow->removeEventFilter (this);
            break;
        default:
            break;
        }
    } while(false);
    return QObject::eventFilter(watched, event);
}

int PreviewManager::previewCount() const
{
    return  d->m_previews.size();
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style, int deviceProfileIndex, QString *errorMessage)
{
    return createPreviewPixmap(fw, configurationFromSettings(fw->core(), style), deviceProfileIndex, errorMessage);
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style, QString *errorMessage)
{
    return createPreviewPixmap(fw, style, -1, errorMessage);
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw,
                                            const PreviewConfiguration &pc,
                                            int deviceProfileIndex,
                                            QString *errorMessage)
{
    QWidget *widget = createPreview(fw, pc, deviceProfileIndex, errorMessage);
    if (!widget)
        return QPixmap();
    const QPixmap rc = widget->grab(QRect(0, 0, -1, -1));
    widget->deleteLater();
    return rc;
}

void PreviewManager::slotZoomChanged(int z)
{
    if (d->m_core) { // Save the last zoom chosen by the user.
        QDesignerSharedSettings settings(d->m_core);
        settings.setZoom(z);
    }
}
}

QT_END_NAMESPACE

#include "previewmanager.moc"
