// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "newformwidget_p.h"
#include "ui_newformwidget.h"
#include "qdesigner_formbuilder_p.h"
#include "sheet_delegate_p.h"
#include "widgetdatabase_p.h"
#include "shared_settings_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/abstractwidgetdatabase.h>

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qtreewidget.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

enum { profileComboIndexOffset = 1 };
enum { debugNewFormWidget = 0 };

enum NewForm_CustomRole {
    // File name (templates from resources, paths)
    TemplateNameRole = Qt::UserRole + 100,
    // Class name (widgets from Widget data base)
    ClassNameRole = Qt::UserRole + 101
};

static constexpr auto newFormObjectNameC = "Form"_L1;

// Create a form name for an arbitrary class. If it is Qt, qtify it,
//  else return "Form".
static QString formName(const QString &className)
{
    if (!className.startsWith(u'Q'))
        return newFormObjectNameC;
    QString rc = className;
    rc.remove(0, 1);
    return rc;
}

namespace qdesigner_internal {

struct TemplateSize {
    const char *name;
    int width;
    int height;
};

static const struct TemplateSize templateSizes[] =
{
    { QT_TRANSLATE_NOOP("qdesigner_internal::NewFormWidget", "Default size"), 0, 0 },
    { QT_TRANSLATE_NOOP("qdesigner_internal::NewFormWidget", "QVGA portrait (240x320)"), 240, 320  },
    { QT_TRANSLATE_NOOP("qdesigner_internal::NewFormWidget", "QVGA landscape (320x240)"), 320, 240 },
    { QT_TRANSLATE_NOOP("qdesigner_internal::NewFormWidget", "VGA portrait (480x640)"), 480, 640 },
    { QT_TRANSLATE_NOOP("qdesigner_internal::NewFormWidget", "VGA landscape (640x480)"), 640, 480 }
};

/* -------------- NewForm dialog.
 * Designer takes new form templates from:
 * 1) Files located in directories specified in resources
 * 2) Files located in directories specified as user templates
 * 3) XML from container widgets deemed usable for form templates by the widget
 *    database
 * 4) XML from custom container widgets deemed usable for form templates by the
 *    widget database
 *
 * The widget database provides helper functions to obtain lists of names
 * and xml for 3,4.
 *
 * Fixed-size forms for embedded platforms are obtained as follows:
 * 1) If the origin is a file:
 *    - Check if the file exists in the subdirectory "/<width>x<height>/" of
 *      the path (currently the case for the dialog box because the button box
 *      needs to be positioned)
 *    - Scale the form using the QWidgetDatabase::scaleFormTemplate routine.
 * 2) If the origin is XML:
 *     - Scale the form using the QWidgetDatabase::scaleFormTemplate routine.
 *
 * The tree widget item roles indicate which type of entry it is
 * (TemplateNameRole = file name 1,2, ClassNameRole = class name 3,4)
 */

NewFormWidget::NewFormWidget(QDesignerFormEditorInterface *core, QWidget *parentWidget) :
    QDesignerNewFormWidgetInterface(parentWidget),
    m_core(core),
      m_ui(new QT_PREPEND_NAMESPACE(qdesigner_internal)::Ui::NewFormWidget),
    m_currentItem(nullptr),
    m_acceptedItem(nullptr)
{
     // ### FIXME Qt 8: Remove (QTBUG-96005)
#if QT_VERSION >= QT_VERSION_CHECK(7, 0, 0)
    QDesignerSharedSettings::migrateTemplates();
#endif

    m_ui->setupUi(this);
    m_ui->treeWidget->setItemDelegate(new qdesigner_internal::SheetDelegate(m_ui->treeWidget, this));
    m_ui->treeWidget->header()->hide();
    m_ui->treeWidget->header()->setStretchLastSection(true);
    m_ui->lblPreview->setBackgroundRole(QPalette::Base);

    connect(m_ui->treeWidget, &QTreeWidget::itemActivated,
            this, &NewFormWidget::treeWidgetItemActivated);
    connect(m_ui->treeWidget, &QTreeWidget::currentItemChanged,
            this, &NewFormWidget::treeWidgetCurrentItemChanged);
    connect(m_ui->treeWidget, &QTreeWidget::itemPressed,
            this, &NewFormWidget::treeWidgetItemPressed);

    QDesignerSharedSettings settings(m_core);

    QString uiExtension = u"ui"_s;
    QString templatePath = u":/qt-project.org/designer/templates/forms"_s;

    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core);
    if (lang) {
        templatePath = u":/templates/forms"_s;
        uiExtension = lang->uiExtension();
    }

    // Resource templates
    const QString formTemplate = settings.formTemplate();
    QTreeWidgetItem *selectedItem = nullptr;
    loadFrom(templatePath, true, uiExtension, formTemplate, selectedItem);
    // Additional template paths
    const QStringList formTemplatePaths = settings.formTemplatePaths();
    for (const auto &ftp : formTemplatePaths)
        loadFrom(ftp, false, uiExtension, formTemplate, selectedItem);

    // Widgets/custom widgets
    if (!lang) {
        //: New Form Dialog Categories
        loadFrom(tr("Widgets"), qdesigner_internal::WidgetDataBase::formWidgetClasses(core), formTemplate, selectedItem);
        loadFrom(tr("Custom Widgets"), qdesigner_internal::WidgetDataBase::customFormWidgetClasses(core), formTemplate, selectedItem);
    }

    // Still no selection - default to first item
    if (selectedItem == nullptr && m_ui->treeWidget->topLevelItemCount() != 0) {
        QTreeWidgetItem *firstTopLevel = m_ui->treeWidget->topLevelItem(0);
        if (firstTopLevel->childCount() > 0)
            selectedItem = firstTopLevel->child(0);
    }

    // Open parent, select and make visible
    if (selectedItem) {
        m_ui->treeWidget->setCurrentItem(selectedItem);
        selectedItem->setSelected(true);
        m_ui->treeWidget->scrollToItem(selectedItem->parent());
    }
    // Fill profile combo
    m_deviceProfiles = settings.deviceProfiles();
    m_ui->profileComboBox->addItem(tr("None"));
    connect(m_ui->profileComboBox,
            &QComboBox::currentIndexChanged,
            this, &NewFormWidget::slotDeviceProfileIndexChanged);
    if (m_deviceProfiles.isEmpty()) {
        m_ui->profileComboBox->setEnabled(false);
    } else {
        for (const auto &deviceProfile : std::as_const(m_deviceProfiles))
            m_ui->profileComboBox->addItem(deviceProfile.name());
        const int ci = settings.currentDeviceProfileIndex();
        if (ci >= 0)
            m_ui->profileComboBox->setCurrentIndex(ci + profileComboIndexOffset);
    }
    // Fill size combo
    for (const TemplateSize &t : templateSizes)
        m_ui->sizeComboBox->addItem(tr(t.name), QSize(t.width, t.height));

    setTemplateSize(settings.newFormSize());

    if (debugNewFormWidget)
        qDebug() << Q_FUNC_INFO << "Leaving";
}

NewFormWidget::~NewFormWidget()
{
    QDesignerSharedSettings settings (m_core);
    settings.setNewFormSize(templateSize());
    // Do not change previously stored item if dialog was rejected
    if (m_acceptedItem)
        settings.setFormTemplate(m_acceptedItem->text(0));
    delete m_ui;
}

void NewFormWidget::treeWidgetCurrentItemChanged(QTreeWidgetItem *current)
{
    if (debugNewFormWidget)
        qDebug() << Q_FUNC_INFO << current;
    if (!current)
        return;

    if (!current->parent()) { // Top level item: Ensure expanded when browsing down
        return;
    }

    m_currentItem = current;

    emit currentTemplateChanged(showCurrentItemPixmap());
}

bool NewFormWidget::showCurrentItemPixmap()
{
    bool rc = false;
    if (m_currentItem) {
        const QPixmap pixmap = formPreviewPixmap(m_currentItem);
        if (pixmap.isNull()) {
            m_ui->lblPreview->setText(tr("Error loading form"));
        } else {
            m_ui->lblPreview->setPixmap(pixmap);
            rc = true;
        }
    }
    return rc;
}

void NewFormWidget::treeWidgetItemActivated(QTreeWidgetItem *item)
{
    if (debugNewFormWidget)
        qDebug() << Q_FUNC_INFO << item;

    if (item->data(0, TemplateNameRole).isValid() || item->data(0, ClassNameRole).isValid())
        emit templateActivated();
}

QPixmap  NewFormWidget::formPreviewPixmap(const QTreeWidgetItem *item)
{
    // Cache pixmaps per item/device profile
    const ItemPixmapCacheKey cacheKey(item, profileComboIndex());
    auto it = m_itemPixmapCache.find(cacheKey);
    if (it == m_itemPixmapCache.end()) {
        // file or string?
        const QVariant fileName = item->data(0, TemplateNameRole);
        QPixmap rc;
        if (fileName.metaType().id() == QMetaType::QString) {
            rc = formPreviewPixmap(fileName.toString());
        } else {
            const QVariant classNameV = item->data(0, ClassNameRole);
            Q_ASSERT(classNameV.metaType().id() == QMetaType::QString);
            const QString className = classNameV.toString();
            QByteArray data =  qdesigner_internal::WidgetDataBase::formTemplate(m_core, className, formName(className)).toUtf8();
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            rc = formPreviewPixmap(buffer);
        }
        if (rc.isNull()) // Retry invalid ones
            return rc;
        it = m_itemPixmapCache.insert(cacheKey, rc);
    }
    return it.value();
}

QPixmap NewFormWidget::formPreviewPixmap(const QString &fileName) const
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QFileInfo fi(fileName);
        const QPixmap rc = formPreviewPixmap(f, fi.absolutePath());
        f.close();
        return rc;
    }
    qWarning() << "The file " << fileName << " could not be opened: " << f.errorString();
    return QPixmap();
}

QImage NewFormWidget::grabForm(QDesignerFormEditorInterface *core,
                         QIODevice &file,
                         const QString &workingDir,
                         const qdesigner_internal::DeviceProfile &dp)
{
    qdesigner_internal::NewFormWidgetFormBuilder
        formBuilder(core, dp);
    if (!workingDir.isEmpty())
        formBuilder.setWorkingDirectory(workingDir);

    QWidget *widget = formBuilder.load(&file, nullptr);
    if (!widget)
        return QImage();

    const QPixmap pixmap = widget->grab(QRect(0, 0, -1, -1));
    widget->deleteLater();
    return pixmap.toImage();
}

QPixmap NewFormWidget::formPreviewPixmap(QIODevice &file, const QString &workingDir) const
{
    const QSizeF screenSize(screen()->geometry().size());
    const int previewSize = qRound(screenSize.width() / 7.5); // 256 on 1920px screens.
    const int margin = previewSize / 32 - 1; // 7 on 1920px screens.
    const int shadow = margin;

    const QImage wimage = grabForm(m_core, file, workingDir,  currentDeviceProfile());
    if (wimage.isNull())
        return QPixmap();
    const qreal devicePixelRatio = wimage.devicePixelRatioF();
    const QSize imageSize(previewSize - margin * 2, previewSize - margin * 2);
    QImage image = wimage.scaled((QSizeF(imageSize) * devicePixelRatio).toSize(),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image.setDevicePixelRatio(devicePixelRatio);

    QImage dest((QSizeF(previewSize, previewSize) * devicePixelRatio).toSize(),
                QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(devicePixelRatio);
    dest.fill(0);

    QPainter p(&dest);
    p.drawImage(margin, margin, image);

    p.setPen(QPen(palette().brush(QPalette::WindowText), 0));

    p.drawRect(QRectF(margin - 1, margin - 1, imageSize.width() + 1.5, imageSize.height() + 1.5));

    const QColor dark(Qt::darkGray);
    const QColor light(Qt::transparent);

    // right shadow
    {
        const QRect rect(margin + imageSize.width() + 1, margin + shadow, shadow, imageSize.height() - shadow + 1);
        QLinearGradient lg(rect.topLeft(), rect.topRight());
        lg.setColorAt(0, dark);
        lg.setColorAt(1, light);
        p.fillRect(rect, lg);
    }

    // bottom shadow
    {
        const QRect rect(margin + shadow, margin + imageSize.height() + 1, imageSize.width() - shadow + 1, shadow);
        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0, dark);
        lg.setColorAt(1, light);
        p.fillRect(rect, lg);
    }

    // bottom/right corner shadow
    {
        const QRect rect(margin + imageSize.width() + 1, margin + imageSize.height() + 1, shadow, shadow);
        QRadialGradient g(rect.topLeft(), shadow - 1);
        g.setColorAt(0, dark);
        g.setColorAt(1, light);
        p.fillRect(rect, g);
    }

    // top/right corner
    {
        const QRect rect(margin + imageSize.width() + 1, margin, shadow, shadow);
        QRadialGradient g(rect.bottomLeft(), shadow - 1);
        g.setColorAt(0, dark);
        g.setColorAt(1, light);
        p.fillRect(rect, g);
    }

    // bottom/left corner
    {
        const QRect rect(margin, margin + imageSize.height() + 1, shadow, shadow);
        QRadialGradient g(rect.topRight(), shadow - 1);
        g.setColorAt(0, dark);
        g.setColorAt(1, light);
        p.fillRect(rect, g);
    }

    p.end();

    return QPixmap::fromImage(dest);
}

void NewFormWidget::loadFrom(const QString &path, bool resourceFile, const QString &uiExtension,
                       const QString &selectedItem, QTreeWidgetItem *&selectedItemFound)
{
    const QDir dir(path);

    if (!dir.exists())
        return;

    // Iterate through the directory and add the templates
    const QFileInfoList list = dir.entryInfoList(QStringList{"*."_L1 + uiExtension},
                                                 QDir::Files);

    if (list.isEmpty())
        return;

    const QChar separator = resourceFile ? QChar(u'/')
                                         : QDir::separator();
    QTreeWidgetItem *root = new QTreeWidgetItem(m_ui->treeWidget);
    root->setFlags(root->flags() & ~Qt::ItemIsSelectable);
    // Try to get something that is easy to read.
    QString visiblePath = path;
    int index = visiblePath.lastIndexOf(separator);
    if (index != -1) {
        // try to find a second slash, just to be a bit better.
        const int index2 = visiblePath.lastIndexOf(separator, index - 1);
        if (index2 != -1)
            index = index2;
        visiblePath = visiblePath.mid(index + 1);
        visiblePath = QDir::toNativeSeparators(visiblePath);
    }

    root->setText(0, visiblePath.replace(u'_', u' '));
    root->setToolTip(0, path);

    for (const auto &fi : list) {
        if (!fi.isFile())
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(root);
        const QString text = fi.baseName().replace(u'_', u' ');
        if (selectedItemFound == nullptr && text == selectedItem)
            selectedItemFound = item;
        item->setText(0, text);
        item->setData(0, TemplateNameRole, fi.absoluteFilePath());
    }
}

void NewFormWidget::loadFrom(const QString &title, const QStringList &nameList,
                       const QString &selectedItem, QTreeWidgetItem *&selectedItemFound)
{
    if (nameList.isEmpty())
        return;
    QTreeWidgetItem *root = new QTreeWidgetItem(m_ui->treeWidget);
    root->setFlags(root->flags() & ~Qt::ItemIsSelectable);
    root->setText(0, title);
    for (const auto &text : nameList) {
        QTreeWidgetItem *item = new QTreeWidgetItem(root);
        item->setText(0, text);
        if (selectedItemFound == nullptr && text == selectedItem)
            selectedItemFound = item;
        item->setData(0, ClassNameRole, text);
    }
}

void NewFormWidget::treeWidgetItemPressed(QTreeWidgetItem *item)
{
    if (item && !item->parent())
        item->setExpanded(!item->isExpanded());
}

QSize NewFormWidget::templateSize() const
{
    return m_ui->sizeComboBox->itemData(m_ui->sizeComboBox->currentIndex()).toSize();
}

void NewFormWidget::setTemplateSize(const QSize &s)
{
    const int index = s.isNull() ? 0 : m_ui->sizeComboBox->findData(s);
    if (index != -1)
        m_ui->sizeComboBox->setCurrentIndex(index);
}

static QString readAll(const QString &fileName, QString *errorMessage)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        *errorMessage = NewFormWidget::tr("Unable to open the form template file '%1': %2").arg(fileName, file.errorString());
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

QString NewFormWidget::itemToTemplate(const QTreeWidgetItem *item, QString *errorMessage) const
{
    const QSize size = templateSize();
    // file name or string contents?
    const QVariant templateFileName = item->data(0, TemplateNameRole);
    if (templateFileName.metaType().id() == QMetaType::QString) {
        const QString fileName = templateFileName.toString();
        // No fixed size: just open.
        if (size.isNull())
            return readAll(fileName, errorMessage);
        // try to find a file matching the size, like "../640x480/xx.ui"
        const QFileInfo fiBase(fileName);
        QString sizeFileName;
        QTextStream(&sizeFileName) << fiBase.path() << QDir::separator()
                                   << size.width() << 'x' << size.height() << QDir::separator()
                                   << fiBase.fileName();
        if (QFileInfo(sizeFileName).isFile())
            return readAll(sizeFileName, errorMessage);
        // Nothing found, scale via DOM/temporary file
        QString contents = readAll(fileName, errorMessage);
        if (!contents.isEmpty())
            contents = qdesigner_internal::WidgetDataBase::scaleFormTemplate(contents, size, false);
        return contents;
    }
    // Content.
    const QString className = item->data(0, ClassNameRole).toString();
    QString contents = qdesigner_internal::WidgetDataBase::formTemplate(m_core, className, formName(className));
    if (!size.isNull())
        contents = qdesigner_internal::WidgetDataBase::scaleFormTemplate(contents, size, false);
    return contents;
}

void NewFormWidget::slotDeviceProfileIndexChanged(int idx)
{
    // Store index for form windows to take effect and refresh pixmap
    QDesignerSharedSettings settings(m_core);
    settings.setCurrentDeviceProfileIndex(idx - profileComboIndexOffset);
    showCurrentItemPixmap();
}

int NewFormWidget::profileComboIndex() const
{
    return m_ui->profileComboBox->currentIndex();
}

qdesigner_internal::DeviceProfile NewFormWidget::currentDeviceProfile() const
{
    const int ci = profileComboIndex();
    if (ci > 0)
        return m_deviceProfiles.at(ci - profileComboIndexOffset);
    return qdesigner_internal::DeviceProfile();
}

bool NewFormWidget::hasCurrentTemplate() const
{
    return m_currentItem != nullptr;
}

QString NewFormWidget::currentTemplateI(QString *ptrToErrorMessage)
{
    if (m_currentItem == nullptr) {
        *ptrToErrorMessage = tr("Internal error: No template selected.");
        return QString();
    }
    const QString contents = itemToTemplate(m_currentItem, ptrToErrorMessage);
    if (contents.isEmpty())
        return contents;

    m_acceptedItem = m_currentItem;
    return contents;
}

QString NewFormWidget::currentTemplate(QString *ptrToErrorMessage)
{
    if (ptrToErrorMessage)
        return currentTemplateI(ptrToErrorMessage);
    // Do not loose the error
    QString errorMessage;
    const QString contents = currentTemplateI(&errorMessage);
    if (!errorMessage.isEmpty())
        qWarning("%s", errorMessage.toUtf8().constData());
    return contents;
}

}

QT_END_NAMESPACE
