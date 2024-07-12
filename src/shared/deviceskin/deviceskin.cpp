// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "deviceskin_p.h"

#include <QtCore/qnamespace.h>
#include <QtWidgets/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>

#ifdef TEST_SKIN
#  include <QtWidgets/QMainWindow>
#  include <QtWidgets/QDialog>
#  include <QtWidgets/QDialogButtonBox>
#  include <QtWidgets/QHBoxLayout>
#endif

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    enum { joydistance = 10, key_repeat_period = 50, key_repeat_delay = 500 };
    enum { debugDeviceSkin = 0 };
}

static void parseRect(const QString &value, QRect *rect) {
    const auto l = QStringView{value}.split(QLatin1Char(' '));
    rect->setRect(l[0].toInt(), l[1].toInt(), l[2].toInt(), l[3].toInt());
}

static QString msgImageNotLoaded(const QString &f)        {
    return DeviceSkin::tr("The image file '%1' could not be loaded.").arg(f);
}

// ------------ DeviceSkinButtonArea
QDebug &operator<<(QDebug &str, const DeviceSkinButtonArea &a)
{

    str << "Area: " <<  a.name << " keyCode=" << a.keyCode << " area=" <<  a.area
        << " text=" << a.text << " activeWhenClosed=" << a.activeWhenClosed;
    return str;
}

// ------------  DeviceSkinParameters

QDebug operator<<(QDebug str, const DeviceSkinParameters &p)
{
    str << "Images " << p.skinImageUpFileName << ','
        << p.skinImageDownFileName<< ',' << p.skinImageClosedFileName
        <<  ',' <<  p.skinCursorFileName <<"\nScreen: " << p.screenRect
        << " back: " << p.backScreenRect << " closed: " << p.closedScreenRect
        << " cursor: " << p.cursorHot << " Prefix: " <<  p.prefix
        << " Joystick: " << p.joystick << " MouseHover" << p.hasMouseHover;
    const int numAreas = p.buttonAreas.size();
    for (int i = 0; i < numAreas; i++)
        str <<  p.buttonAreas[i];
    return str;
}

QSize DeviceSkinParameters::secondaryScreenSize() const
{
    return backScreenRect.isNull() ?  closedScreenRect .size(): backScreenRect.size();
}

bool DeviceSkinParameters::hasSecondaryScreen() const
{
    return secondaryScreenSize() != QSize(0, 0);
}

bool DeviceSkinParameters::read(const QString &skinDirectory,  ReadMode rm,  QString *errorMessage)
{
    // Figure out the name. remove ending '/' if present
    QString skinFile = skinDirectory;
    if (skinFile.endsWith(QLatin1Char('/')))
        skinFile.truncate(skinFile.size() - 1);

    QFileInfo fi(skinFile);
    QString fn;
    if ( fi.isDir() ) {
        prefix = skinFile;
        prefix += QLatin1Char('/');
        fn = prefix;
        fn += fi.baseName();
        fn += ".skin"_L1;
    } else if (fi.isFile()){
        fn = skinFile;
        prefix = fi.path();
        prefix += QLatin1Char('/');
    } else {
        *errorMessage =  DeviceSkin::tr("The skin directory '%1' does not contain a configuration file.").arg(skinDirectory);
        return false;
    }
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly )) {
        *errorMessage =  DeviceSkin::tr("The skin configuration file '%1' could not be opened.").arg(fn);
        return false;
    }
    QTextStream ts(&f);
    const bool rc = read(ts, rm, errorMessage);
    if (!rc)
        *errorMessage =  DeviceSkin::tr("The skin configuration file '%1' could not be read: %2")
                         .arg(fn, *errorMessage);
    return rc;
}
bool DeviceSkinParameters::read(QTextStream &ts, ReadMode rm, QString *errorMessage)
{
    QStringList closedAreas;
    QStringList toggleAreas;
    QStringList toggleActiveAreas;
    int nareas = 0;
    screenDepth = 0;
    QString mark;
    ts >> mark;
    hasMouseHover = true; // historical default
    if (mark == "[SkinFile]"_L1) {
        const QString UpKey = "Up"_L1;
        const QString DownKey = "Down"_L1;
        const QString ClosedKey = "Closed"_L1;
        const QString ClosedAreasKey = "ClosedAreas"_L1;
        const QString ScreenKey = "Screen"_L1;
        const QString ScreenDepthKey = "ScreenDepth"_L1;
        const QString BackScreenKey = "BackScreen"_L1;
        const QString ClosedScreenKey = "ClosedScreen"_L1;
        const QString CursorKey = "Cursor"_L1;
        const QString AreasKey = "Areas"_L1;
        const QString ToggleAreasKey = "ToggleAreas"_L1;
        const QString ToggleActiveAreasKey = "ToggleActiveAreas"_L1;
        const QString HasMouseHoverKey = "HasMouseHover"_L1;
        // New
        while (!nareas) {
            QString line = ts.readLine();
            if ( line.isNull() )
                break;
            if (!line.isEmpty() && line.at(0) != u'#') {
                int eq = line.indexOf(QLatin1Char('='));
                if ( eq >= 0 ) {
                    const QString key = line.left(eq);
                    eq++;
                    while (eq<line.size()-1 && line[eq].isSpace())
                        eq++;
                    const QString value = line.mid(eq);
                    if ( key == UpKey ) {
                        skinImageUpFileName = value;
                    } else if ( key == DownKey ) {
                        skinImageDownFileName = value;
                    } else if ( key ==  ClosedKey ) {
                        skinImageClosedFileName = value;
                    } else if ( key == ClosedAreasKey ) {
                        closedAreas = value.split(QLatin1Char(' '));
                    } else if ( key == ScreenKey ) {
                        parseRect( value, &screenRect);
                    } else if ( key == ScreenDepthKey ) {
                        screenDepth = value.toInt();
                    } else if ( key == BackScreenKey ) {
                        parseRect(value, &backScreenRect);
                    } else if ( key == ClosedScreenKey ) {
                        parseRect( value, &closedScreenRect );
                    } else if ( key == CursorKey ) {
                        QStringList l = value.split(QLatin1Char(' '));
                        skinCursorFileName = l[0];
                        cursorHot = QPoint(l[1].toInt(),l[2].toInt());
                    } else if ( key == AreasKey ) {
                        nareas = value.toInt();
                    } else if ( key == ToggleAreasKey ) {
                        toggleAreas = value.split(QLatin1Char(' '));
                    } else if ( key == ToggleActiveAreasKey ) {
                        toggleActiveAreas = value.split(QLatin1Char(' '));
                    } else if ( key == HasMouseHoverKey ) {
                        hasMouseHover = value == "true"_L1 || value == "1"_L1;
                    }
                } else {
                    *errorMessage =  DeviceSkin::tr("Syntax error: %1").arg(line);
                    return false;
                }
            }
        }
    } else {
        // Old
        skinImageUpFileName = mark;
        QString s;
        int x,y,w,h,na;
        ts >> s >> x >> y >> w >> h >> na;
        skinImageDownFileName = s;
        screenRect.setRect(x, y, w, h);
        nareas = na;
    }
    // Done for short mode
    if (rm ==  ReadSizeOnly)
        return true;
    //  verify skin files exist
    skinImageUpFileName.insert(0, prefix);
    if (!QFile(skinImageUpFileName).exists()) {
        *errorMessage =  DeviceSkin::tr("The skin \"up\" image file '%1' does not exist.").arg(skinImageUpFileName);
        return false;
    }
    if (!skinImageUp.load(skinImageUpFileName)) {
        *errorMessage = msgImageNotLoaded(skinImageUpFileName);
        return false;
    }

    skinImageDownFileName.insert(0, prefix);
    if (!QFile(skinImageDownFileName).exists()) {
        *errorMessage =  DeviceSkin::tr("The skin \"down\" image file '%1' does not exist.").arg(skinImageDownFileName);
        return false;
    }
    if (!skinImageDown.load(skinImageDownFileName)) {
        *errorMessage = msgImageNotLoaded(skinImageDownFileName);
        return false;
    }

    if (!skinImageClosedFileName.isEmpty()) {
        skinImageClosedFileName.insert(0, prefix);
        if (!QFile(skinImageClosedFileName).exists()) {
            *errorMessage =  DeviceSkin::tr("The skin \"closed\" image file '%1' does not exist.").arg(skinImageClosedFileName);
            return false;
        }
        if (!skinImageClosed.load(skinImageClosedFileName)) {
            *errorMessage = msgImageNotLoaded(skinImageClosedFileName);
            return false;
        }
    }

    if (!skinCursorFileName.isEmpty()) {
        skinCursorFileName.insert(0, prefix);
        if (!QFile(skinCursorFileName).exists()) {
            *errorMessage =  DeviceSkin::tr("The skin cursor image file '%1' does not exist.").arg(skinCursorFileName);
            return false;
        }
        if (!skinCursor.load(skinCursorFileName)) {
            *errorMessage = msgImageNotLoaded(skinCursorFileName);
            return false;
        }
    }

    // read areas
    if (!nareas)
        return true;
    buttonAreas.reserve(nareas);

    int i = 0;
    ts.readLine(); // eol
    joystick = -1;
    const QString Joystick = "Joystick"_L1;
    const QRegularExpression splitRe("[ \t][ \t]*"_L1);
    Q_ASSERT(splitRe.isValid());
    while (i < nareas && !ts.atEnd() ) {
        buttonAreas.push_back(DeviceSkinButtonArea());
        DeviceSkinButtonArea &area = buttonAreas.back();
        const QString line = ts.readLine();
        if ( !line.isEmpty() && line[0] != QLatin1Char('#') ) {
            const QStringList tok = line.split(splitRe);
            if ( tok.size()<6 ) {
                *errorMessage =  DeviceSkin::tr("Syntax error in area definition: %1").arg(line);
                return false;
            } else {
                area.name = tok[0];
                QString k = tok[1];
                if ( k.left(2).toLower() == "0x"_L1) {
                    area.keyCode = k.mid(2).toInt(0,16);
                } else {
                    area.keyCode = k.toInt();
                }

                int p=0;
                for (int j=2; j < tok.size() - 1; ) {
                    const int x = tok[j++].toInt();
                    const int y = tok[j++].toInt();
                    area.area.putPoints(p++,1,x,y);
                }

                const QChar doubleQuote = QLatin1Char('"');
                if ( area.name[0] == doubleQuote && area.name.endsWith(doubleQuote)) {
                    area.name.truncate(area.name.size() - 1);
                    area.name.remove(0, 1);
                }
                if ( area.name.size() == 1 )
                    area.text = area.name;
                if ( area.name == Joystick)
                    joystick = i;
                area.activeWhenClosed = closedAreas.contains(area.name)
                    || area.keyCode == Qt::Key_Flip; // must be to work
                area.toggleArea = toggleAreas.contains(area.name);
                area.toggleActiveArea = toggleActiveAreas.contains(area.name);
                if (area.toggleArea)
                    toggleAreaList += i;
                i++;
            }
        }
    }
    if (i != nareas) {
        qWarning() << DeviceSkin::tr("Mismatch in number of areas, expected %1, got %2.")
                      .arg(nareas).arg(i);
    }
    if (debugDeviceSkin)
        qDebug() << *this;
    return true;
}

// --------- CursorWindow declaration

namespace qvfb_internal {

class CursorWindow : public QWidget
{
public:
    explicit CursorWindow(const QImage &cursor, QPoint hot, QWidget *sk);

    void setView(QWidget*);
    void setPos(QPoint);
    bool handleMouseEvent(QEvent *ev);

protected:
    bool event(QEvent *) override;
    bool eventFilter(QObject*, QEvent *) override;

private:
    QWidget *mouseRecipient;
    QWidget *m_view;
    QWidget *skin;
    QPoint hotspot;
};
}

// --------- Skin

DeviceSkin::DeviceSkin(const DeviceSkinParameters &parameters,  QWidget *p ) :
    QWidget(p),
    m_parameters(parameters),
    buttonRegions(parameters.buttonAreas.size(), QRegion()),
    parent(p),
    t_skinkey(new QTimer(this)),
    t_parentmove(new QTimer(this))
{
    Q_ASSERT(p);
    setMouseTracking(true);
    setAttribute(Qt::WA_NoSystemBackground);

    setZoom(1.0);
    connect(t_skinkey, &QTimer::timeout, this, &DeviceSkin::skinKeyRepeat );
    t_parentmove->setSingleShot( true );
    connect(t_parentmove, &QTimer::timeout, this, &DeviceSkin::moveParent );
}

void DeviceSkin::skinKeyRepeat()
{
    if ( m_view ) {
        const DeviceSkinButtonArea &area = m_parameters.buttonAreas[buttonIndex];
        emit skinKeyReleaseEvent(area.keyCode,area.text, true);
        emit skinKeyPressEvent(area.keyCode, area.text, true);
        t_skinkey->start(key_repeat_period);
    }
}

void DeviceSkin::calcRegions()
{
    const int numAreas = m_parameters.buttonAreas.size();
    for (int i=0; i<numAreas; i++) {
        QPolygon xa(m_parameters.buttonAreas[i].area.size());
        int n = m_parameters.buttonAreas[i].area.size();
        for (int p = 0; p < n; p++) {
            xa.setPoint(p,transform.map(m_parameters.buttonAreas[i].area[p]));
        }
        if (n == 2) {
            buttonRegions[i] = QRegion(xa.boundingRect());
        } else {
            buttonRegions[i] = QRegion(xa);
        }
    }
}

void DeviceSkin::loadImages()
{
    QImage iup = m_parameters.skinImageUp;
    QImage idown = m_parameters.skinImageDown;

    QImage iclosed;
    const bool hasClosedImage = !m_parameters.skinImageClosed.isNull();

    if (hasClosedImage)
        iclosed =  m_parameters.skinImageClosed;
    QImage icurs;
    const bool hasCursorImage = !m_parameters.skinCursor.isNull();
    if (hasCursorImage)
        icurs =  m_parameters.skinCursor;

    if (!transform.isIdentity()) {
        iup = iup.transformed(transform, Qt::SmoothTransformation);
        idown = idown.transformed(transform, Qt::SmoothTransformation);
        if (hasClosedImage)
            iclosed = iclosed.transformed(transform, Qt::SmoothTransformation);
        if (hasCursorImage)
            icurs = icurs.transformed(transform, Qt::SmoothTransformation);
    }
    const Qt::ImageConversionFlags conv = Qt::ThresholdAlphaDither|Qt::AvoidDither;
    skinImageUp = QPixmap::fromImage(iup);
    skinImageDown = QPixmap::fromImage(idown, conv);
    if (hasClosedImage)
        skinImageClosed = QPixmap::fromImage(iclosed, conv);
    if (hasCursorImage)
        skinCursor = QPixmap::fromImage(icurs, conv);

    setFixedSize( skinImageUp.size() );
    if (!skinImageUp.mask())
        skinImageUp.setMask(skinImageUp.createHeuristicMask());
    if (!skinImageClosed.mask())
        skinImageClosed.setMask(skinImageClosed.createHeuristicMask());

    QWidget* parent = parentWidget();
    parent->setMask( skinImageUp.mask() );
    parent->setFixedSize( skinImageUp.size() );

    delete cursorw;
    cursorw = 0;
    if (hasCursorImage) {
        cursorw = new qvfb_internal::CursorWindow(m_parameters.skinCursor, m_parameters.cursorHot, this);
        if (m_view)
            cursorw->setView(m_view);
    }
}

DeviceSkin::~DeviceSkin( )
{
    delete cursorw;
}

void DeviceSkin::setTransform(const QTransform &wm)
{
    transform = QImage::trueMatrix(wm,m_parameters.skinImageUp.width(),m_parameters.skinImageUp.height());
    calcRegions();
    loadImages();
    if ( m_view ) {
        QPoint p = transform.map(QPolygon(m_parameters.screenRect)).boundingRect().topLeft();
        m_view->move(p);
    }
    updateSecondaryScreen();
}

void DeviceSkin::setZoom( double z )
{
    setTransform(QTransform().scale(z,z));
}

void DeviceSkin::updateSecondaryScreen()
{
    if (!m_secondaryView)
        return;
    if (flipped_open) {
        if (m_parameters.backScreenRect.isNull()) {
            m_secondaryView->hide();
        } else {
            m_secondaryView->move(transform.map(QPolygon(m_parameters.backScreenRect)).boundingRect().topLeft());
            m_secondaryView->show();
        }
    } else {
        if (m_parameters.closedScreenRect.isNull()) {
            m_secondaryView->hide();
        } else {
            m_secondaryView->move(transform.map(QPolygon(m_parameters.closedScreenRect)).boundingRect().topLeft());
            m_secondaryView->show();
        }
    }
}

void DeviceSkin::setView( QWidget *v )
{
    m_view = v;
    m_view->setFocus();
    m_view->move(transform.map(QPolygon(m_parameters.screenRect)).boundingRect().topLeft());
    if ( cursorw )
        cursorw->setView(v);
}

void DeviceSkin::setSecondaryView( QWidget *v )
{
    m_secondaryView = v;
    updateSecondaryScreen();
}

void DeviceSkin::paintEvent( QPaintEvent *)
{
    QPainter p( this );
    if ( flipped_open ) {
        p.drawPixmap(0, 0, skinImageUp);
    } else {
        p.drawPixmap(0, 0, skinImageClosed);
    }
    QList<int> toDraw;
    if ( buttonPressed == true ) {
        toDraw += buttonIndex;
    }
    for (int toggle : std::as_const(m_parameters.toggleAreaList)) {
        const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[toggle];
        if (flipped_open || ba.activeWhenClosed) {
            if (ba.toggleArea && ba.toggleActiveArea)
                toDraw += toggle;
        }
    }
    for (int button : std::as_const(toDraw)) {
        const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[button];
        const QRect r = buttonRegions[button].boundingRect();
        if ( ba.area.size() > 2 )
            p.setClipRegion(buttonRegions[button]);
        p.drawPixmap( r.topLeft(), skinImageDown, r);
    }
}

void DeviceSkin::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == Qt::RightButton) {
        emit popupMenu();
    } else {
        buttonPressed = false;

        onjoyrelease = -1;
        const int numAreas = m_parameters.buttonAreas.size();
        for (int i = 0; i < numAreas ; i++) {
            const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[i];
            if (  buttonRegions[i].contains( e->position().toPoint() ) ) {
                if ( flipped_open || ba.activeWhenClosed ) {
                    if ( m_parameters.joystick == i ) {
                        joydown = true;
                    } else {
                        if ( joydown )
                            onjoyrelease = i;
                        else
                            startPress(i);
                        break;
                        if (debugDeviceSkin)// Debug message to be sure we are clicking the right areas
                            qDebug()<< m_parameters.buttonAreas[i].name << " clicked";
                    }
                }
            }
        }
        clickPos = e->position().toPoint();
//      This is handy for finding the areas to define rectangles for new skins
        if (debugDeviceSkin)
            qDebug()<< "Clicked in " <<  e->position().toPoint().x() << ',' <<  e->position().toPoint().y();
        clickPos = e->position().toPoint();
    }
}

void DeviceSkin::flip(bool open)
{
    if ( flipped_open == open )
        return;
    if ( open ) {
        parent->setMask(skinImageUp.mask());
        emit skinKeyReleaseEvent(Qt::Key(Qt::Key_Flip), QString(), false);
    } else {
        parent->setMask(skinImageClosed.mask());
        emit skinKeyPressEvent(Qt::Key(Qt::Key_Flip), QString(), false);
    }
    flipped_open = open;
    updateSecondaryScreen();
    repaint();
}

void DeviceSkin::startPress(int i)
{
    buttonPressed = true;
    buttonIndex = i;
    if (m_view) {
        const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[buttonIndex];
        if (ba.keyCode == Qt::Key_Flip) {
            flip(!flipped_open);
        } else if (ba.toggleArea) {
            bool active = !ba.toggleActiveArea;
            const_cast<DeviceSkinButtonArea &>(ba).toggleActiveArea = active;
            if (active)
                emit skinKeyPressEvent(ba.keyCode, ba.text, false);
            else
                emit skinKeyReleaseEvent(ba.keyCode, ba.text, false);
        } else {
            emit skinKeyPressEvent(ba.keyCode, ba.text, false);
            t_skinkey->start(key_repeat_delay);
        }
        repaint(buttonRegions[buttonIndex].boundingRect());
    }
}

void DeviceSkin::endPress()
{
    const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[buttonIndex];
    if (m_view && ba.keyCode != Qt::Key_Flip && !ba.toggleArea )
        emit skinKeyReleaseEvent(ba.keyCode, ba.text, false);
    t_skinkey->stop();
    buttonPressed = false;
    repaint( buttonRegions[buttonIndex].boundingRect() );
}

void DeviceSkin::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->buttons() & Qt::LeftButton ) {
        const int joystick = m_parameters.joystick;
        QPoint newpos =  e->globalPosition().toPoint() - clickPos;
        if (joydown) {
            int k1=0, k2=0;
            if (newpos.x() < -joydistance) {
                k1 = joystick+1;
            } else if (newpos.x() > +joydistance) {
                k1 = joystick+3;
            }
            if (newpos.y() < -joydistance) {
                k2 = joystick+2;
            } else if (newpos.y() > +joydistance) {
                k2 = joystick+4;
            }
            if (k1 || k2) {
                if (!buttonPressed) {
                    onjoyrelease = -1;
                    if (k1 && k2) {
                        startPress(k2);
                        endPress();
                    }
                    startPress(k1 ? k1 : k2);
                }
            } else if (buttonPressed) {
                endPress();
            }
        } else if (buttonPressed == false) {
            parentpos = newpos;
            if (!t_parentmove->isActive())
                t_parentmove->start(50);
        }
    }
    if ( cursorw )
        cursorw->setPos(e->globalPosition().toPoint());
}

void DeviceSkin::moveParent()
{
    parent->move( parentpos );
}

void DeviceSkin::mouseReleaseEvent( QMouseEvent * )
{
    if ( buttonPressed )
        endPress();
    if ( joydown ) {
        joydown = false;
        if (onjoyrelease >= 0) {
            startPress(onjoyrelease);
            endPress();
        }
    }
}

bool DeviceSkin::hasCursor() const
{
    return !skinCursor.isNull();
}

// ------------------ CursorWindow implementation

namespace qvfb_internal {

bool CursorWindow::eventFilter( QObject *, QEvent *ev)
{
    handleMouseEvent(ev);
    return false;
}

bool CursorWindow::event( QEvent *ev )
{
    if (handleMouseEvent(ev))
        return true;
    return QWidget::event(ev);
}

bool CursorWindow::handleMouseEvent(QEvent *ev)
{
    bool handledEvent = false;
    static int inhere=0;
    if ( !inhere ) {
        inhere++;
        if (m_view) {
            if (ev->type() >= QEvent::MouseButtonPress && ev->type() <= QEvent::MouseMove) {
                QMouseEvent *e = (QMouseEvent*)ev;
                QPoint gp = e->globalPosition().toPoint();
                QPoint vp = m_view->mapFromGlobal(gp);
                QPoint sp = skin->mapFromGlobal(gp);
                if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick) {
                    if (m_view->rect().contains(vp))
                        mouseRecipient = m_view;
                    else if (skin->parentWidget()->geometry().contains(gp))
                        mouseRecipient = skin;
                    else
                        mouseRecipient = 0;
                }
                if (mouseRecipient) {
                    setPos(gp);
                    QMouseEvent me(e->type(),mouseRecipient==skin ? sp : vp,gp,e->button(),e->buttons(),e->modifiers());
                    QApplication::sendEvent(mouseRecipient, &me);
                } else if (!skin->parentWidget()->geometry().contains(gp)) {
                    hide();
                } else {
                    setPos(gp);
                }
                if (e->type() == QEvent::MouseButtonRelease)
                    mouseRecipient = 0;
                handledEvent = true;
            }
        }
        inhere--;
    }
    return handledEvent;
}

void CursorWindow::setView(QWidget* v)
{
    if ( m_view ) {
        m_view->removeEventFilter(this);
        m_view->removeEventFilter(this);
    }
    m_view = v;
    m_view->installEventFilter(this);
    m_view->installEventFilter(this);
    mouseRecipient = 0;
}

CursorWindow::CursorWindow(const QImage &img, QPoint hot, QWidget* sk)
    : QWidget(0),
      m_view(0),
      skin(sk),
      hotspot(hot)
{
    setWindowFlags( Qt::FramelessWindowHint );
    mouseRecipient = 0;
    setMouseTracking(true);
#ifndef QT_NO_CURSOR
    setCursor(Qt::BlankCursor);
#endif
    QPixmap p;
    p = QPixmap::fromImage(img);
    if (!p.mask()) {
        QBitmap bm = img.hasAlphaChannel() ? QBitmap::fromImage(img.createAlphaMask())
                                           : QBitmap::fromImage(img.createHeuristicMask());
        p.setMask(bm);
    }
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(p));
    setPalette(palette);
    setFixedSize( p.size() );
    if ( !p.mask().isNull() )
        setMask(p.mask());
}

void CursorWindow::setPos(QPoint p)
{
    move(p-hotspot);
    show();
    raise();
}
}

#ifdef TEST_SKIN

int main(int argc,char *argv[])
{
    if (argc < 1)
        return 1;
    const QString skinFile = QString::fromUtf8(argv[1]);
    QApplication app(argc,argv);
    QMainWindow mw;

    DeviceSkinParameters params;
    QString errorMessage;
    if (!params.read(skinFile, DeviceSkinParameters::ReadAll, &errorMessage)) {
        qWarning() << errorMessage;
        return 1;
    }
    DeviceSkin ds(params, &mw);
    // View Dialog
    QDialog *dialog = new QDialog();
    QHBoxLayout *dialogLayout = new QHBoxLayout();
    dialog->setLayout(dialogLayout);
    QDialogButtonBox *dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QObject::connect(dialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    QObject::connect(dialogButtonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    dialogLayout->addWidget(dialogButtonBox);
    dialog->setFixedSize(params.screenSize());
    dialog->setParent(&ds, Qt::SubWindow);
    dialog->setAutoFillBackground(true);
    ds.setView(dialog);

    QObject::connect(&ds, &DeviceSkin::popupMenu, &mw, &QWidget::close);
    QObject::connect(&ds, &DeviceSkin::skinKeyPressEvent, &mw, &QWidget::close);
    mw.show();
    return app.exec();
}

#endif

QT_END_NAMESPACE

