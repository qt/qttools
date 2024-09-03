// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_utils_p.h"
#include "qdesigner_propertycommand_p.h"
#include "abstractformbuilder.h"
#include "formwindowbase_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractresourcebrowser.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/taskmenu.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qoperatingsystemversion.h>
#include <QtCore/qprocess.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qqueue.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstandardpaths.h>

#include <QtWidgets/qapplication.h>
#include <QtGui/qicon.h>
#include <QtGui/qpalette.h>
#include <QtGui/qpixmap.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qtablewidget.h>
#include <QtWidgets/qcombobox.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal
{
    // ### FIXME Qt 8: Remove (QTBUG-96005)
    QString legacyDataDirectory()
    {
        return QDir::homePath() + u"/.designer"_s;
    }

    QString dataDirectory()
    {
#if QT_VERSION >= QT_VERSION_CHECK(7, 0, 0)
        return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
               + u'/' + QCoreApplication::organizationName() + u"/Designer"_s;
#else
        return legacyDataDirectory();
#endif
    }


    QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message)
    {
        qWarning("Designer: %s", qPrintable(message));
    }

    void reloadTreeItem(DesignerIconCache *iconCache, QTreeWidgetItem *item)
    {
        if (!item)
            return;

        for (int c = 0; c < item->columnCount(); c++) {
            const QVariant v = item->data(c, Qt::DecorationPropertyRole);
            if (v.canConvert<PropertySheetIconValue>())
                item->setIcon(c, iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
        }
    }

    void reloadListItem(DesignerIconCache *iconCache, QListWidgetItem *item)
    {
        if (!item)
            return;

        const QVariant v = item->data(Qt::DecorationPropertyRole);
        if (v.canConvert<PropertySheetIconValue>())
            item->setIcon(iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
    }

    void reloadTableItem(DesignerIconCache *iconCache, QTableWidgetItem *item)
    {
        if (!item)
            return;

        const QVariant v = item->data(Qt::DecorationPropertyRole);
        if (v.canConvert<PropertySheetIconValue>())
            item->setIcon(iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
    }

    void reloadIconResources(DesignerIconCache *iconCache, QObject *object)
    {
        if (QListWidget *listWidget = qobject_cast<QListWidget *>(object)) {
            for (int i = 0; i < listWidget->count(); i++)
                reloadListItem(iconCache, listWidget->item(i));
        } else if (QComboBox *comboBox = qobject_cast<QComboBox *>(object)) {
            for (int i = 0; i < comboBox->count(); i++) {
                const QVariant v = comboBox->itemData(i, Qt::DecorationPropertyRole);
                if (v.canConvert<PropertySheetIconValue>()) {
                    QIcon icon = iconCache->icon(qvariant_cast<PropertySheetIconValue>(v));
                    comboBox->setItemIcon(i, icon);
                    comboBox->setItemData(i, icon);
                }
            }
        } else if (QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>(object)) {
            reloadTreeItem(iconCache, treeWidget->headerItem());
            QQueue<QTreeWidgetItem *> itemsQueue;
            for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
                itemsQueue.enqueue(treeWidget->topLevelItem(i));
            while (!itemsQueue.isEmpty()) {
                QTreeWidgetItem *item = itemsQueue.dequeue();
                for (int i = 0; i < item->childCount(); i++)
                    itemsQueue.enqueue(item->child(i));
                reloadTreeItem(iconCache, item);
            }
        } else if (QTableWidget *tableWidget = qobject_cast<QTableWidget *>(object)) {
            const int columnCount = tableWidget->columnCount();
            const int rowCount = tableWidget->rowCount();
            for (int c = 0; c < columnCount; c++)
                reloadTableItem(iconCache, tableWidget->horizontalHeaderItem(c));
            for (int r = 0; r < rowCount; r++)
                reloadTableItem(iconCache, tableWidget->verticalHeaderItem(r));
            for (int c = 0; c < columnCount; c++)
                for (int r = 0; r < rowCount; r++)
                    reloadTableItem(iconCache, tableWidget->item(r, c));
        }
    }

    // ------------- DesignerMetaEnum
    DesignerMetaEnum::DesignerMetaEnum(const QString &name, const QString &scope, const QString &separator) :
        MetaEnum<int>(name, scope, separator)
    {
    }


    QString DesignerMetaEnum::toString(int value, SerializationMode sm, bool *ok) const
    {
        // find value
        bool valueOk;
        const QString item = valueToKey(value, &valueOk);
        if (ok)
            *ok = valueOk;

        if (!valueOk)
            return item;

        QString qualifiedItem;
        appendQualifiedName(item, sm, qualifiedItem);
        return qualifiedItem;
    }

    QString DesignerMetaEnum::messageToStringFailed(int value) const
    {
        return QCoreApplication::translate("DesignerMetaEnum",
                                           "%1 is not a valid enumeration value of '%2'.")
                                           .arg(value).arg(enumName());
    }

    QString DesignerMetaEnum::messageParseFailed(const QString &s) const
    {
        return QCoreApplication::translate("DesignerMetaEnum",
                                           "'%1' could not be converted to an enumeration value of type '%2'.")
                                           .arg(s, enumName());
    }
    // -------------- DesignerMetaFlags
    DesignerMetaFlags::DesignerMetaFlags(const QString &enumName, const QString &scope,
                                         const QString &separator) :
        MetaEnum<uint>(enumName, scope, separator)
    {
    }

    QStringList DesignerMetaFlags::flags(int ivalue) const
    {
        QStringList rc;
        const uint v = static_cast<uint>(ivalue);
        for (auto it = keyToValueMap().begin(), end = keyToValueMap().end(); it != end; ++it)  {
            const uint itemValue = it->second;
            // Check for equality first as flag values can be 0 or -1, too. Takes preference over a bitwise flag
            if (v == itemValue) {
                rc.clear();
                rc.push_back(it->first);
                return rc;
            }
            // Do not add 0-flags (None-flags)
            if (itemValue)
                if ((v & itemValue) == itemValue)
                    rc.push_back(it->first);
        }
        return rc;
    }


    QString DesignerMetaFlags::toString(int value, SerializationMode sm) const
    {
        const QStringList flagIds = flags(value);
        if (flagIds.isEmpty())
            return QString();

        QString rc;
        for (const auto &id : flagIds) {
            if (!rc.isEmpty())
                rc += u'|';
            appendQualifiedName(id, sm, rc);
        }
        return rc;
    }


    int DesignerMetaFlags::parseFlags(const QString &s, bool *ok) const
    {
        if (s.isEmpty()) {
            if (ok)
                *ok = true;
            return 0;
        }
        uint flags = 0;
        bool valueOk = true;
        const auto keys = QStringView{s}.split(u'|');
        for (const auto &key : keys) {
            const uint flagValue = keyToValue(key, &valueOk);
            if (!valueOk) {
                flags = 0;
                break;
            }
            flags |= flagValue;
        }
        if (ok)
            *ok = valueOk;
        return static_cast<int>(flags);
    }

    QString DesignerMetaFlags::messageParseFailed(const QString &s) const
    {
        return QCoreApplication::translate("DesignerMetaFlags",
                                           "'%1' could not be converted to a flag value of type '%2'.")
                                           .arg(s, enumName());
    }

    // ---------- PropertySheetEnumValue

    PropertySheetEnumValue::PropertySheetEnumValue(int v, const DesignerMetaEnum &me) :
       value(v),
       metaEnum(me)
    {
    }

    PropertySheetEnumValue::PropertySheetEnumValue() = default;

    // ---------------- PropertySheetFlagValue
    PropertySheetFlagValue::PropertySheetFlagValue(int v, const DesignerMetaFlags &mf) :
        value(v),
        metaFlags(mf)
    {
    }

    PropertySheetFlagValue::PropertySheetFlagValue() = default;

    // ---------------- PropertySheetPixmapValue
    PropertySheetPixmapValue::PropertySheetPixmapValue(const QString &path) : m_path(path)
    {
    }

    PropertySheetPixmapValue::PropertySheetPixmapValue() = default;

    PropertySheetPixmapValue::PixmapSource PropertySheetPixmapValue::getPixmapSource(QDesignerFormEditorInterface *core, const QString & path)
    {
        if (const QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core))
            return lang->isLanguageResource(path) ?  LanguageResourcePixmap : FilePixmap;
        return path.startsWith(u':') ? ResourcePixmap : FilePixmap;
    }

    QString PropertySheetPixmapValue::path() const
    {
        return m_path;
    }

    void PropertySheetPixmapValue::setPath(const QString &path)
    {
        if (m_path == path)
            return;
        m_path = path;
    }

    // ---------- PropertySheetIconValue

    class PropertySheetIconValueData : public QSharedData {
    public:
        PropertySheetIconValue::ModeStateToPixmapMap m_paths;
        QString m_theme;
        int m_themeEnum = -1;
    };

    PropertySheetIconValue::PropertySheetIconValue(const PropertySheetPixmapValue &pixmap) :
        m_data(new PropertySheetIconValueData)
    {
        setPixmap(QIcon::Normal, QIcon::Off, pixmap);
    }

    PropertySheetIconValue::PropertySheetIconValue() :
        m_data(new PropertySheetIconValueData)
    {
    }

    PropertySheetIconValue::~PropertySheetIconValue() = default;

    PropertySheetIconValue::PropertySheetIconValue(const PropertySheetIconValue &rhs) noexcept = default;
    PropertySheetIconValue &PropertySheetIconValue::operator=(const PropertySheetIconValue &rhs) = default;

    PropertySheetIconValue::PropertySheetIconValue(PropertySheetIconValue &&) noexcept = default;
    PropertySheetIconValue &PropertySheetIconValue::operator=(PropertySheetIconValue &&) noexcept = default;

} // namespace qdesigner_internal

namespace qdesigner_internal {

    size_t qHash(const PropertySheetIconValue &p, size_t seed) noexcept
    {
        // qHash for paths making use of the existing QPair hash functions.
        const auto *d = p.m_data.constData();
        return qHashMulti(seed, d->m_paths, d->m_themeEnum, d->m_theme);
    }

    bool comparesEqual(const PropertySheetIconValue &lhs,
                       const PropertySheetIconValue &rhs) noexcept
    {
        const auto *lhsd = lhs.m_data.constData();
        const auto *rhsd = rhs.m_data.constData();
        return lhsd == rhsd
            || (lhsd->m_themeEnum == rhsd->m_themeEnum
                && lhsd->m_theme == rhsd->m_theme && lhsd->m_paths == rhsd->m_paths);
    }

    bool PropertySheetIconValue::isEmpty() const
    {
        return m_data->m_themeEnum == -1 && m_data->m_theme.isEmpty()
               && m_data->m_paths.isEmpty();
    }

    QString PropertySheetIconValue::theme() const
    {
        return m_data->m_theme;
    }

    void PropertySheetIconValue::setTheme(const QString &t)
    {
        m_data->m_theme = t;
    }

    int PropertySheetIconValue::themeEnum() const
    {
        return m_data->m_themeEnum;
    }

    void PropertySheetIconValue::setThemeEnum(int e)
    {
        m_data->m_themeEnum = e;
    }

    PropertySheetPixmapValue PropertySheetIconValue::pixmap(QIcon::Mode mode, QIcon::State state) const
    {
        const ModeStateKey pair{mode, state};
        return m_data->m_paths.value(pair);
    }

    void PropertySheetIconValue::setPixmap(QIcon::Mode mode, QIcon::State state, const PropertySheetPixmapValue &pixmap)
    {
        const ModeStateKey pair{mode, state};
        if (pixmap.path().isEmpty())
            m_data->m_paths.remove(pair);
        else
            m_data->m_paths.insert(pair, pixmap);
    }

    QPixmap DesignerPixmapCache::pixmap(const PropertySheetPixmapValue &value) const
    {
        const auto it = m_cache.constFind(value);
        if (it != m_cache.constEnd())
            return it.value();

        QPixmap pix = QPixmap(value.path());
        m_cache.insert(value, pix);
        return pix;
    }

    void DesignerPixmapCache::clear()
    {
        m_cache.clear();
    }

    DesignerPixmapCache::DesignerPixmapCache(QObject *parent)
        : QObject(parent)
    {
    }

    QIcon DesignerIconCache::icon(const PropertySheetIconValue &value) const
    {
        const auto it = m_cache.constFind(value);
        if (it != m_cache.constEnd())
            return it.value();

        // Match on the theme first if it is available.
        if (value.themeEnum() != -1) {
            const QIcon themeIcon = QIcon::fromTheme(static_cast<QIcon::ThemeIcon>(value.themeEnum()));
            m_cache.insert(value, themeIcon);
            return themeIcon;
        }
        if (!value.theme().isEmpty()) {
            const QString theme = value.theme();
            if (QIcon::hasThemeIcon(theme)) {
                const QIcon themeIcon = QIcon::fromTheme(theme);
                m_cache.insert(value, themeIcon);
                return themeIcon;
            }
        }

        QIcon icon;
        const PropertySheetIconValue::ModeStateToPixmapMap &paths = value.paths();
        for (auto it = paths.constBegin(), cend = paths.constEnd(); it != cend; ++it) {
            const auto pair = it.key();
            icon.addFile(it.value().path(), QSize(), pair.first, pair.second);
        }
        m_cache.insert(value, icon);
        return icon;
    }

    void DesignerIconCache::clear()
    {
        m_cache.clear();
    }

    DesignerIconCache::DesignerIconCache(DesignerPixmapCache *pixmapCache, QObject *parent)
        : QObject(parent),
        m_pixmapCache(pixmapCache)
    {

    }

    PropertySheetTranslatableData::PropertySheetTranslatableData(bool translatable, const QString &disambiguation, const QString &comment) :
        m_translatable(translatable), m_disambiguation(disambiguation), m_comment(comment) { }

    PropertySheetStringValue::PropertySheetStringValue(const QString &value,
                    bool translatable, const QString &disambiguation, const QString &comment) :
        PropertySheetTranslatableData(translatable, disambiguation, comment), m_value(value) {}

    QString PropertySheetStringValue::value() const
    {
        return m_value;
    }

    void PropertySheetStringValue::setValue(const QString &value)
    {
        m_value = value;
    }

    PropertySheetStringListValue::PropertySheetStringListValue(const QStringList &value,
                                 bool translatable,
                                 const QString &disambiguation,
                                 const QString &comment) :
        PropertySheetTranslatableData(translatable, disambiguation, comment), m_value(value)
    {
    }

    QStringList PropertySheetStringListValue::value() const
    {
        return m_value;
    }

    void PropertySheetStringListValue::setValue(const QStringList &value)
    {
        m_value = value;
    }

    QStringList m_value;


    PropertySheetKeySequenceValue::PropertySheetKeySequenceValue(const QKeySequence &value,
                    bool translatable, const QString &disambiguation, const QString &comment)
        : PropertySheetTranslatableData(translatable, disambiguation, comment),
          m_value(value), m_standardKey(QKeySequence::UnknownKey) {}

    PropertySheetKeySequenceValue::PropertySheetKeySequenceValue(const QKeySequence::StandardKey &standardKey,
                    bool translatable, const QString &disambiguation, const QString &comment)
        : PropertySheetTranslatableData(translatable, disambiguation, comment),
          m_value(QKeySequence(standardKey)), m_standardKey(standardKey) {}

    QKeySequence PropertySheetKeySequenceValue::value() const
    {
        return m_value;
    }

    void PropertySheetKeySequenceValue::setValue(const QKeySequence &value)
    {
        m_value = value;
        m_standardKey = QKeySequence::UnknownKey;
    }

    QKeySequence::StandardKey PropertySheetKeySequenceValue::standardKey() const
    {
        return m_standardKey;
    }

    void PropertySheetKeySequenceValue::setStandardKey(const QKeySequence::StandardKey &standardKey)
    {
        m_value = QKeySequence(standardKey);
        m_standardKey = standardKey;
    }

    bool PropertySheetKeySequenceValue::isStandardKey() const
    {
        return m_standardKey != QKeySequence::UnknownKey;
    }

    /* IconSubPropertyMask: Assign each icon sub-property (pixmaps for the
     * various states/modes and the theme) a flag bit (see QFont) so that they
     * can be handled individually when assigning property values to
     * multiselections in the set-property-commands (that is, do not clobber
     * other subproperties when assigning just one).
     * Provide back-and-forth mapping functions for the icon states. */

    enum IconSubPropertyMask {
        NormalOffIconMask   = 0x01,
        NormalOnIconMask    = 0x02,
        DisabledOffIconMask = 0x04,
        DisabledOnIconMask  = 0x08,
        ActiveOffIconMask   = 0x10,
        ActiveOnIconMask    = 0x20,
        SelectedOffIconMask = 0x40,
        SelectedOnIconMask  = 0x80,
        ThemeIconMask       = 0x10000,
        ThemeEnumIconMask   = 0x20000
    };

    static inline uint iconStateToSubPropertyFlag(QIcon::Mode mode, QIcon::State state)
    {
        switch (mode) {
        case QIcon::Disabled:
            return state == QIcon::On ? DisabledOnIconMask : DisabledOffIconMask;
        case QIcon::Active:
            return state == QIcon::On ?   ActiveOnIconMask :   ActiveOffIconMask;
        case QIcon::Selected:
            return state == QIcon::On ? SelectedOnIconMask : SelectedOffIconMask;
        case QIcon::Normal:
            break;
        }
        return     state == QIcon::On ?   NormalOnIconMask :   NormalOffIconMask;
    }

    static inline std::pair<QIcon::Mode, QIcon::State> subPropertyFlagToIconModeState(unsigned flag)
    {
        switch (flag) {
        case NormalOnIconMask:
            return {QIcon::Normal,   QIcon::On};
        case DisabledOffIconMask:
            return {QIcon::Disabled, QIcon::Off};
        case DisabledOnIconMask:
            return {QIcon::Disabled, QIcon::On};
        case ActiveOffIconMask:
            return {QIcon::Active,   QIcon::Off};
        case ActiveOnIconMask:
            return {QIcon::Active,   QIcon::On};
        case SelectedOffIconMask:
            return {QIcon::Selected, QIcon::Off};
        case SelectedOnIconMask:
            return {QIcon::Selected, QIcon::On};
        case NormalOffIconMask:
        default:
            break;
        }
        return     {QIcon::Normal,   QIcon::Off};
    }

    uint PropertySheetIconValue::mask() const
    {
        uint flags = 0;
        for (auto it = m_data->m_paths.constBegin(), cend = m_data->m_paths.constEnd(); it != cend; ++it)
            flags |= iconStateToSubPropertyFlag(it.key().first, it.key().second);
        if (!m_data->m_theme.isEmpty())
            flags |= ThemeIconMask;
        if (m_data->m_themeEnum != -1)
            flags |= ThemeEnumIconMask;
        return flags;
    }

    uint PropertySheetIconValue::compare(const PropertySheetIconValue &other) const
    {
        uint diffMask = mask() | other.mask();
        for (int i = 0; i < 8; i++) {
            const uint flag = 1 << i;
            if (diffMask & flag) { // if state is set in both icons, compare the values
                const auto state = subPropertyFlagToIconModeState(flag);
                if (pixmap(state.first, state.second) == other.pixmap(state.first, state.second))
                    diffMask &= ~flag;
            }
        }
        if ((diffMask & ThemeIconMask) && theme() == other.theme())
            diffMask &= ~ThemeIconMask;
        if ((diffMask & ThemeEnumIconMask) && themeEnum() == other.themeEnum())
            diffMask &= ~ThemeEnumIconMask;

        return diffMask;
    }

    PropertySheetIconValue PropertySheetIconValue::themed() const
    {
        PropertySheetIconValue rc(*this);
        rc.m_data->m_paths.clear();
        return rc;
    }

    PropertySheetIconValue PropertySheetIconValue::unthemed() const
    {
        PropertySheetIconValue rc(*this);
        rc.m_data->m_theme.clear();
        rc.m_data->m_themeEnum = -1;
        return rc;
    }

    void PropertySheetIconValue::assign(const PropertySheetIconValue &other, uint mask)
    {
        for (int i = 0; i < 8; i++) {
            uint flag = 1 << i;
            if (mask & flag) {
                const ModeStateKey state = subPropertyFlagToIconModeState(flag);
                setPixmap(state.first, state.second, other.pixmap(state.first, state.second));
            }
        }
        if (mask & ThemeIconMask)
            setTheme(other.theme());
        if (mask & ThemeEnumIconMask)
            setThemeEnum(other.themeEnum());
    }

    const PropertySheetIconValue::ModeStateToPixmapMap &PropertySheetIconValue::paths() const
    {
        return m_data->m_paths;
    }

    QDESIGNER_SHARED_EXPORT QDebug operator<<(QDebug debug, const PropertySheetIconValue &p)
    {
        QDebugStateSaver saver(debug);
        debug.nospace();
        debug.noquote();
        debug << "PropertySheetIconValue(mask=0x" << Qt::hex << p.mask() << Qt::dec << ", ";
        if (p.themeEnum() != -1)
            debug << "theme=" << p.themeEnum() << ", ";
        if (!p.theme().isEmpty())
            debug << "XDG theme=\"" << p.theme() << "\", ";

        const PropertySheetIconValue::ModeStateToPixmapMap &paths = p.paths();
        for (auto it = paths.constBegin(), cend = paths.constEnd(); it != cend; ++it) {
            debug << " mode=" << it.key().first << ",state=" << it.key().second
                << ", \"" << it.value().path() << '"';
        }
        debug << ')';
        return debug;
    }

    QDESIGNER_SHARED_EXPORT QDesignerFormWindowCommand *createTextPropertyCommand(const QString &propertyName, const QString &text, QObject *object, QDesignerFormWindowInterface *fw)
    {
        if (text.isEmpty()) {
            ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
            cmd->init(object, propertyName);
            return cmd;
        }
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(object, propertyName, text);
        return cmd;
    }

    QDESIGNER_SHARED_EXPORT QAction *preferredEditAction(QDesignerFormEditorInterface *core, QWidget *managedWidget)
    {
        QAction *action = nullptr;
        if (const QDesignerTaskMenuExtension *taskMenu = qt_extension<QDesignerTaskMenuExtension*>(core->extensionManager(), managedWidget)) {
            action = taskMenu->preferredEditAction();
            if (!action) {
                const auto actions = taskMenu->taskActions();
                if (!actions.isEmpty())
                    action = actions.first();
            }
        }
        if (!action) {
            if (const auto *taskMenu = qobject_cast<QDesignerTaskMenuExtension *>(
                        core->extensionManager()->extension(managedWidget, u"QDesignerInternalTaskMenuExtension"_s))) {
                action = taskMenu->preferredEditAction();
                if (!action) {
                    const auto actions = taskMenu->taskActions();
                    if (!actions.isEmpty())
                        action = actions.first();
                }
            }
        }
        return action;
    }

    QDESIGNER_SHARED_EXPORT bool runUIC(const QString &fileName, UicLanguage language,
                                        QByteArray& ba, QString &errorMessage)
    {
        QProcess uic;
        QStringList arguments;
        static constexpr auto uicBinary =
            QOperatingSystemVersion::currentType() != QOperatingSystemVersion::Windows
            ? "/uic"_L1 : "/uic.exe"_L1;
        QString binary = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath) + uicBinary;
        // In a PySide6 installation, there is no libexec directory; uic.exe is
        // in the main wheel directory next to designer.exe.
        if (!QFileInfo::exists(binary))
            binary = QCoreApplication::applicationDirPath() + uicBinary;
        if (!QFileInfo::exists(binary)) {
            errorMessage = QApplication::translate("Designer", "%1 does not exist.").
                           arg(QDir::toNativeSeparators(binary));
            return false;
        }

        switch (language) {
        case UicLanguage::Cpp:
            break;
        case UicLanguage::Python:
            arguments << u"-g"_s << u"python"_s;
            break;
        }
        arguments << fileName;

        uic.start(binary, arguments);
        if (!uic.waitForStarted()) {
            errorMessage = QApplication::translate("Designer", "Unable to launch %1: %2").
                           arg(QDir::toNativeSeparators(binary), uic.errorString());
            return false;
        }
        if (!uic.waitForFinished()) {
            errorMessage = QApplication::translate("Designer", "%1 timed out.").arg(binary);
            return false;
        }
        if (uic.exitCode()) {
            errorMessage =  QString::fromLatin1(uic.readAllStandardError());
            return false;
        }
        ba = uic.readAllStandardOutput();
        return true;
    }

    QDESIGNER_SHARED_EXPORT QString qtify(const QString &name)
    {
        QString qname = name;

        Q_ASSERT(qname.isEmpty() == false);


        if (qname.size() > 1 && qname.at(1).isUpper()) {
            const QChar first = qname.at(0);
            if (first == u'Q' || first == u'K')
                qname.remove(0, 1);
        }

        const qsizetype len = qname.size();
        for (qsizetype i = 0; i < len && qname.at(i).isUpper(); ++i)
            qname[i] = qname.at(i).toLower();

        return qname;
    }

    // --------------- UpdateBlocker
    UpdateBlocker::UpdateBlocker(QWidget *w) :
        m_widget(w),
        m_enabled(w->updatesEnabled() && w->isVisible())
    {
        if (m_enabled)
            m_widget->setUpdatesEnabled(false);
    }

    UpdateBlocker::~UpdateBlocker()
    {
        if (m_enabled)
            m_widget->setUpdatesEnabled(true);
    }

// from qpalette.cpp
quint64 paletteResolveMask(QPalette::ColorGroup colorGroup,
                           QPalette::ColorRole colorRole)
{
    if (colorRole == QPalette::Accent)
        colorRole = QPalette::NoRole; // See qtbase/17c589df94a2245ee92d45839c2cba73566d7310
    const auto offset = quint64(QPalette::NColorRoles - 1) * quint64(colorGroup);
    const auto bitPos = quint64(colorRole) + offset;
    return 1ull << bitPos;
}

quint64 paletteResolveMask(QPalette::ColorRole colorRole)
{
    return paletteResolveMask(QPalette::Active, colorRole)
        | paletteResolveMask(QPalette::Inactive, colorRole)
        | paletteResolveMask(QPalette::Disabled, colorRole);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
