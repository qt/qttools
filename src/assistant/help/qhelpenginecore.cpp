// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpenginecore.h"
#include "qhelpcollectionhandler_p.h"
#include "qhelpdbreader_p.h"
#include "qhelpfilterengine.h"
#include "qhelplink.h"

#if QT_CONFIG(future)
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qpromise.h>
#endif

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QHelpEngineCorePrivate
{
public:
    QHelpEngineCorePrivate(const QString &collectionFile, QHelpEngineCore *helpEngineCore);

    void init(const QString &collectionFile);
    bool setup();

    std::unique_ptr<QHelpCollectionHandler> collectionHandler;
    QHelpFilterEngine *filterEngine = nullptr;
    QString currentFilter;
    QString error;
    bool needsSetup = true;
    bool autoSaveFilter = true;
    bool usesFilterEngine = false;
    bool readOnly = true;

    QHelpEngineCore *q;
};

QHelpEngineCorePrivate::QHelpEngineCorePrivate(const QString &collectionFile,
                                               QHelpEngineCore *helpEngineCore)
{
    q = helpEngineCore;
    filterEngine = new QHelpFilterEngine(q);
    init(collectionFile);
}

void QHelpEngineCorePrivate::init(const QString &collectionFile)
{
    collectionHandler.reset(new QHelpCollectionHandler(collectionFile, q));
    QObject::connect(collectionHandler.get(), &QHelpCollectionHandler::error, q,
                     [this](const QString &msg) { error = msg; });
    filterEngine->setCollectionHandler(collectionHandler.get());
    needsSetup = true;
}

bool QHelpEngineCorePrivate::setup()
{
    error.clear();
    if (!needsSetup)
        return true;

    needsSetup = false;
    emit q->setupStarted();

    collectionHandler->setReadOnly(q->isReadOnly());
    const bool opened = collectionHandler->openCollectionFile();
    if (opened)
        q->currentFilter();

    emit q->setupFinished();
    return opened;
}

/*!
    \class QHelpEngineCore
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpEngineCore class provides the core functionality
    of the help system.

    Before the help engine can be used, it must be initialized by
    calling setupData(). At the beginning of the setup process the
    signal setupStarted() is emitted. From this point on until
    the signal setupFinished() is emitted, is the help data in an
    undefined meaning unusable state.

    The core help engine can be used to perform different tasks.
    By calling documentsForIdentifier() the engine returns
    URLs specifying the file locations inside the help system. The
    actual file data can then be retrieved by calling fileData().

    The help engine can contain any number of custom filters.
    The management of the filters, including adding new filters,
    changing filter definitions, or removing existing filters,
    is done through the QHelpFilterEngine class, which can be accessed
    by the filterEngine() method.

    \note QHelpFilterEngine replaces the older filter API that is
    deprecated since Qt 5.13. Call setUsesFilterEngine() with \c true to
    enable the new functionality.

    The core help engine has two modes:
    \list
        \li Read-only mode, where the help collection file is not changed
            unless explicitly requested. This also works if the
            collection file is in a read-only location,
            and is the default.
        \li Fully writable mode, which requires the help collection
            file to be writable.
    \endlist
    The mode can be changed by calling setReadOnly() method, prior to
    calling setupData().

    The help engine also offers the possibility to set and read values
    in a persistent way comparable to ini files or Windows registry
    entries. For more information see \l setCustomValue() or \l customValue().

    This class does not offer any GUI components or functionality for
    indices or contents. If you need one of those use QHelpEngine
    instead.
*/

/*!
    \fn void QHelpEngineCore::setupStarted()

    This signal is emitted when setup is started.
*/

/*!
    \fn void QHelpEngineCore::setupFinished()

    This signal is emitted when the setup is complete.
*/

/*!
    \fn void QHelpEngineCore::readersAboutToBeInvalidated()
    \deprecated
*/

/*!
    \fn void QHelpEngineCore::currentFilterChanged(const QString &newFilter)
    \deprecated

    QHelpFilterEngine::filterActivated() should be used instead.

    This signal is emitted when the current filter is changed to
    \a newFilter.
*/

/*!
    \fn void QHelpEngineCore::warning(const QString &msg)

    This signal is emitted when a non critical error occurs.
    The warning message is stored in \a msg.
*/

/*!
    Constructs a new core help engine with a \a parent. The help engine
    uses the information stored in the \a collectionFile to provide help.
    If the collection file does not exist yet, it'll be created.
*/
QHelpEngineCore::QHelpEngineCore(const QString &collectionFile, QObject *parent)
    : QObject(parent)
    , d(new QHelpEngineCorePrivate(collectionFile, this))
{}

/*!
    \internal
*/
#if QT_DEPRECATED_SINCE(6, 8)
QHelpEngineCore::QHelpEngineCore(QHelpEngineCorePrivate *helpEngineCorePrivate, QObject *parent)
    : QObject(parent)
    , d(helpEngineCorePrivate)
{}
#endif

/*!
    Destructs the help engine.
*/
QHelpEngineCore::~QHelpEngineCore()
{
    delete d;
}

/*!
    \property QHelpEngineCore::collectionFile
    \brief the absolute file name of the collection file currently used.
    \since 4.5

    Setting this property leaves the help engine in an invalid state. It is
    important to invoke setupData() or any getter function in order to setup
    the help engine again.
*/
QString QHelpEngineCore::collectionFile() const
{
    return d->collectionHandler->collectionFile();
}

void QHelpEngineCore::setCollectionFile(const QString &fileName)
{
    if (fileName != collectionFile())
        d->init(fileName);
}

/*!
    \property QHelpEngineCore::readOnly
    \brief whether the help engine is read-only.
    \since 6.0

    In read-only mode, the user can use the help engine
    with a collection file installed in a read-only location.
    In this case, some functionality won't be accessible,
    like registering additional documentation, filter editing,
    or any action that would require changes to the
    collection file. Setting it to \c false enables the full
    functionality of the help engine.

    By default, this property is \c true.
*/
bool QHelpEngineCore::isReadOnly() const
{
    return d->readOnly;
}

void QHelpEngineCore::setReadOnly(bool enable)
{
    if (d->readOnly == enable)
        return;

    d->readOnly = enable;
    d->init(collectionFile());
}

/*!
    \since 5.13

    Returns the filter engine associated with this help engine.
    The filter engine allows for adding, changing, and removing existing
    filters for this help engine. To use the engine you also have to call
    \l setUsesFilterEngine() set to \c true.
*/
QHelpFilterEngine *QHelpEngineCore::filterEngine() const
{
    return d->filterEngine;
}

/*!
    Sets up the help engine by processing the information found
    in the collection file and returns true if successful; otherwise
    returns false.

    By calling the function, the help
    engine is forced to initialize itself immediately. Most of
    the times, this function does not have to be called
    explicitly because getter functions which depend on a correctly
    set up help engine do that themselves.

    \note \c{qsqlite4.dll} needs to be deployed with the application as the
    help system uses the sqlite driver when loading help collections.
*/
bool QHelpEngineCore::setupData()
{
    d->needsSetup = true;
    return d->setup();
}

/*!
    Creates the file \a fileName and copies all contents from
    the current collection file into the newly created file,
    and returns true if successful; otherwise returns false.

    The copying process makes sure that file references to Qt
    Collection files (\c{.qch}) files are updated accordingly.
*/
bool QHelpEngineCore::copyCollectionFile(const QString &fileName)
{
    if (!d->setup())
        return false;
    return d->collectionHandler->copyCollectionFile(fileName);
}

/*!
    Returns the namespace name defined for the Qt compressed help file (.qch)
    specified by its \a documentationFileName. If the file is not valid, an
    empty string is returned.

    \sa documentationFileName()
*/
QString QHelpEngineCore::namespaceName(const QString &documentationFileName)
{
    void *pointer = const_cast<QString *>(&documentationFileName);
    QHelpDBReader reader(documentationFileName, QHelpGlobal::uniquifyConnectionName(
                         "GetNamespaceName"_L1, pointer), nullptr);
    if (reader.init())
        return reader.namespaceName();
    return {};
}

/*!
    Registers the Qt compressed help file (.qch) contained in the file
    \a documentationFileName. One compressed help file, uniquely
    identified by its namespace can only be registered once.
    True is returned if the registration was successful, otherwise
    false.

    \sa unregisterDocumentation(), error()
*/
bool QHelpEngineCore::registerDocumentation(const QString &documentationFileName)
{
    d->error.clear();
    d->needsSetup = true;
    return d->collectionHandler->registerDocumentation(documentationFileName);
}

/*!
    Unregisters the Qt compressed help file (.qch) identified by its
    \a namespaceName from the help collection. Returns true
    on success, otherwise false.

    \sa registerDocumentation(), error()
*/
bool QHelpEngineCore::unregisterDocumentation(const QString &namespaceName)
{
    d->error.clear();
    d->needsSetup = true;
    return d->collectionHandler->unregisterDocumentation(namespaceName);
}

/*!
    Returns the absolute file name of the Qt compressed help file (.qch)
    identified by the \a namespaceName. If there is no Qt compressed help file
    with the specified namespace registered, an empty string is returned.

    \sa namespaceName()
*/
QString QHelpEngineCore::documentationFileName(const QString &namespaceName)
{
    if (!d->setup())
        return {};

    const QHelpCollectionHandler::FileInfo fileInfo =
            d->collectionHandler->registeredDocumentation(namespaceName);

    if (fileInfo.namespaceName.isEmpty())
        return {};

    if (QDir::isAbsolutePath(fileInfo.fileName))
        return fileInfo.fileName;

    return QFileInfo(QFileInfo(d->collectionHandler->collectionFile()).absolutePath()
                     + u'/' + fileInfo.fileName).absoluteFilePath();
}

/*!
    Returns a list of all registered Qt compressed help files of the current collection file.
    The returned names are the namespaces of the registered Qt compressed help files (.qch).
*/
QStringList QHelpEngineCore::registeredDocumentations() const
{
    if (!d->setup())
        return {};
    const auto &docList = d->collectionHandler->registeredDocumentations();
    QStringList list;
    for (const QHelpCollectionHandler::FileInfo &info : docList)
        list.append(info.namespaceName);
    return list;
}

/*!
    \deprecated

    QHelpFilterEngine::filters() should be used instead.

    Returns a list of custom filters.

    \sa addCustomFilter(), removeCustomFilter()
*/
QStringList QHelpEngineCore::customFilters() const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->customFilters();
}

/*!
    \deprecated

    QHelpFilterEngine::setFilterData() should be used instead.

    Adds the new custom filter \a filterName. The filter attributes
    are specified by \a attributes. If the filter already exists,
    its attribute set is replaced. The function returns true if
    the operation succeeded, otherwise it returns false.

    \sa customFilters(), removeCustomFilter()
*/
bool QHelpEngineCore::addCustomFilter(const QString &filterName,
                                      const QStringList &attributes)
{
    d->error.clear();
    d->needsSetup = true;
    return d->collectionHandler->addCustomFilter(filterName, attributes);
}

/*!
    \deprecated

    QHelpFilterEngine::removeFilter() should be used instead.

    Returns true if the filter \a filterName was removed successfully,
    otherwise false.

    \sa addCustomFilter(), customFilters()
*/
bool QHelpEngineCore::removeCustomFilter(const QString &filterName)
{
    d->error.clear();
    d->needsSetup = true;
    return d->collectionHandler->removeCustomFilter(filterName);
}

/*!
    \deprecated

    QHelpFilterEngine::availableComponents() should be used instead.

    Returns a list of all defined filter attributes.
*/
QStringList QHelpEngineCore::filterAttributes() const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->filterAttributes();
}

/*!
    \deprecated

    QHelpFilterEngine::filterData() should be used instead.

    Returns a list of filter attributes used by the custom
    filter \a filterName.
*/
QStringList QHelpEngineCore::filterAttributes(const QString &filterName) const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->filterAttributes(filterName);
}

/*!
    \deprecated
    \property QHelpEngineCore::currentFilter
    \brief the name of the custom filter currently applied.
    \since 4.5

    QHelpFilterEngine::activeFilter() should be used instead.

    Setting this property will save the new custom filter permanently in the
    help collection file. To set a custom filter without saving it
    permanently, disable the auto save filter mode.

    \sa autoSaveFilter()
*/
QString QHelpEngineCore::currentFilter() const
{
    if (!d->setup())
        return {};

    if (d->currentFilter.isEmpty()) {
        const QString &filter =
                d->collectionHandler->customValue("CurrentFilter"_L1, QString()).toString();
        if (!filter.isEmpty() && d->collectionHandler->customFilters().contains(filter))
            d->currentFilter = filter;
    }
    return d->currentFilter;
}

void QHelpEngineCore::setCurrentFilter(const QString &filterName)
{
    if (!d->setup() || filterName == d->currentFilter)
        return;
    d->currentFilter = filterName;
    if (d->autoSaveFilter)
        d->collectionHandler->setCustomValue("CurrentFilter"_L1, d->currentFilter);
    emit currentFilterChanged(d->currentFilter);
}

/*!
    \deprecated

    QHelpFilterEngine::filterData() should be used instead.

    Returns a list of filter attributes for the different filter sections
    defined in the Qt compressed help file with the given namespace
    \a namespaceName.
*/
QList<QStringList> QHelpEngineCore::filterAttributeSets(const QString &namespaceName) const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->filterAttributeSets(namespaceName);
}

/*!
    \deprecated

    files() should be used instead.

    Returns a list of files contained in the Qt compressed help file \a
    namespaceName. The files can be filtered by \a filterAttributes as
    well as by their extension \a extensionFilter (e.g. 'html').
*/
QList<QUrl> QHelpEngineCore::files(const QString namespaceName,
                                   const QStringList &filterAttributes,
                                   const QString &extensionFilter)
{
    QList<QUrl> res;
    if (!d->setup())
        return res;

    QUrl url;
    url.setScheme("qthelp"_L1);
    url.setAuthority(namespaceName);

    const QStringList &files = d->collectionHandler->files(
                namespaceName, filterAttributes, extensionFilter);
    for (const QString &file : files) {
        url.setPath("/"_L1 + file);
        res.append(url);
    }
    return res;
}

/*!
    Returns a list of files contained in the Qt compressed help file
    for \a namespaceName. The files can be filtered by \a filterName as
    well as by their extension \a extensionFilter (for example, 'html').
*/
QList<QUrl> QHelpEngineCore::files(const QString namespaceName,
    const QString &filterName,
    const QString &extensionFilter)
{
    QList<QUrl> res;
    if (!d->setup())
        return res;

    QUrl url;
    url.setScheme("qthelp"_L1);
    url.setAuthority(namespaceName);

    const QStringList &files = d->collectionHandler->files(
                namespaceName, filterName, extensionFilter);
    for (const QString &file : files) {
        url.setPath("/"_L1 + file);
        res.append(url);
    }
    return res;
}

/*!
    Returns the corrected URL for the \a url that may refer to
    a different namespace defined by the virtual folder defined
    as a part of the \a url. If the virtual folder matches the namespace
    of the \a url, the method just checks if the file exists and returns
    the same \a url. When the virtual folder doesn't match the namespace
    of the \a url, it tries to find the best matching namespace according
    to the active filter. When the namespace is found, it returns the
    corrected URL if the file exists, otherwise it returns an invalid URL.
*/
QUrl QHelpEngineCore::findFile(const QUrl &url) const
{
    if (!d->setup())
        return url;

    QUrl result = d->usesFilterEngine
            ? d->collectionHandler->findFile(url, d->filterEngine->activeFilter())
            : d->collectionHandler->findFile(url, filterAttributes(currentFilter())); // obsolete
    if (!result.isEmpty())
        return result;

    result = d->usesFilterEngine
            ? d->collectionHandler->findFile(url, QString())
            : d->collectionHandler->findFile(url, QStringList()); // obsolete
    if (!result.isEmpty())
        return result;

    return url;
}

/*!
    Returns the data of the file specified by \a url. If the
    file does not exist, an empty QByteArray is returned.

    \sa findFile()
*/
QByteArray QHelpEngineCore::fileData(const QUrl &url) const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->fileData(url);
}

/*!
    \since 5.15

    Returns a list of all the document links found for the \a id.
    The returned list contents depend on the current filter, and therefore only the keywords
    registered for the current filter will be returned.
*/
QList<QHelpLink> QHelpEngineCore::documentsForIdentifier(const QString &id) const
{
    return documentsForIdentifier(
            id, d->usesFilterEngine ? d->filterEngine->activeFilter() : d->currentFilter);
}

/*!
    \since 5.15

    Returns a list of the document links found for the \a id, filtered by \a filterName.
    The returned list contents depend on the passed filter, and therefore only the keywords
    registered for this filter will be returned. If you want to get all results unfiltered,
    pass empty string as \a filterName.
*/
QList<QHelpLink> QHelpEngineCore::documentsForIdentifier(const QString &id, const QString &filterName) const
{
    if (!d->setup())
        return {};

    if (d->usesFilterEngine)
        return d->collectionHandler->documentsForIdentifier(id, filterName);
    return d->collectionHandler->documentsForIdentifier(id, filterAttributes(filterName));
}

/*!
    \since 5.15

    Returns a list of all the document links found for the \a keyword.
    The returned list contents depend on the current filter, and therefore only the keywords
    registered for the current filter will be returned.
*/
QList<QHelpLink> QHelpEngineCore::documentsForKeyword(const QString &keyword) const
{
    return documentsForKeyword(
            keyword, d->usesFilterEngine ? d->filterEngine->activeFilter() : d->currentFilter);
}

/*!
    \since 5.15

    Returns a list of the document links found for the \a keyword, filtered by \a filterName.
    The returned list contents depend on the passed filter, and therefore only the keywords
    registered for this filter will be returned. If you want to get all results unfiltered,
    pass empty string as \a filterName.
*/
QList<QHelpLink> QHelpEngineCore::documentsForKeyword(const QString &keyword, const QString &filterName) const
{
    if (!d->setup())
        return {};

    if (d->usesFilterEngine)
        return d->collectionHandler->documentsForKeyword(keyword, filterName);
    return d->collectionHandler->documentsForKeyword(keyword, filterAttributes(filterName));
}

/*!
    Removes the \a key from the settings section in the
    collection file. Returns true if the value was removed
    successfully, otherwise false.

    \sa customValue(), setCustomValue()
*/
bool QHelpEngineCore::removeCustomValue(const QString &key)
{
    d->error.clear();
    return d->collectionHandler->removeCustomValue(key);
}

/*!
    Returns the value assigned to the \a key. If the requested
    key does not exist, the specified \a defaultValue is
    returned.

    \sa setCustomValue(), removeCustomValue()
*/
QVariant QHelpEngineCore::customValue(const QString &key, const QVariant &defaultValue) const
{
    if (!d->setup())
        return {};
    return d->collectionHandler->customValue(key, defaultValue);
}

/*!
    Save the \a value under the \a key. If the key already exist,
    the value will be overwritten. Returns true if the value was
    saved successfully, otherwise false.

    \sa customValue(), removeCustomValue()
*/
bool QHelpEngineCore::setCustomValue(const QString &key, const QVariant &value)
{
    d->error.clear();
    return d->collectionHandler->setCustomValue(key, value);
}

/*!
    Returns the meta data for the Qt compressed help file \a
    documentationFileName. If there is no data available for
    \a name, an invalid QVariant() is returned. The meta
    data is defined when creating the Qt compressed help file and
    cannot be modified later. Common meta data includes e.g.
    the author of the documentation.
*/
QVariant QHelpEngineCore::metaData(const QString &documentationFileName,
                                   const QString &name)
{
    QHelpDBReader reader(documentationFileName, "GetMetaData"_L1, nullptr);

    if (reader.init())
        return reader.metaData(name);
    return {};
}

/*!
    Returns a description of the last error that occurred.
*/
QString QHelpEngineCore::error() const
{
    return d->error;
}

/*!
    \property QHelpEngineCore::autoSaveFilter
    \brief whether QHelpEngineCore is in auto save filter mode or not.
    \since 4.5

    If QHelpEngineCore is in auto save filter mode, the current filter is
    automatically saved when it is changed by the QHelpFilterEngine::setActiveFilter()
    function. The filter is saved persistently in the help collection file.

    By default, this mode is on.
*/
void QHelpEngineCore::setAutoSaveFilter(bool save)
{
    d->autoSaveFilter = save;
}

bool QHelpEngineCore::autoSaveFilter() const
{
    return d->autoSaveFilter;
}

/*!
    \since 5.13

    Enables or disables the new filter engine functionality
    inside the help engine, according to the passed \a uses parameter.

    \sa filterEngine()
*/
void QHelpEngineCore::setUsesFilterEngine(bool uses)
{
    d->usesFilterEngine = uses;
}

/*!
    \since 5.13

    Returns whether the help engine uses the new filter functionality.

    \sa filterEngine()
*/
bool QHelpEngineCore::usesFilterEngine() const
{
    return d->usesFilterEngine;
}

#if QT_CONFIG(future)
static QUrl constructUrl(const QString &namespaceName, const QString &folderName,
                         const QString &relativePath)
{
    const int idx = relativePath.indexOf(u'#');
    const QString &rp = idx < 0 ? relativePath : relativePath.left(idx);
    const QString anchor = idx < 0 ? QString() : relativePath.mid(idx + 1);
    return QHelpCollectionHandler::buildQUrl(namespaceName, folderName, rp, anchor);
}

using ContentProviderResult = QList<QHelpCollectionHandler::ContentsData>;
using ContentProvider = std::function<ContentProviderResult(const QString &)>;
using ContentResult = std::shared_ptr<QHelpContentItem>;

// This trick is needed because the c'tor of QHelpContentItem is private.
QHelpContentItem *createContentItem(const QString &name = {}, const QUrl &link = {},
                                    QHelpContentItem *parent = {})
{
    return new QHelpContentItem(name, link, parent);
}

static void requestContentHelper(QPromise<ContentResult> &promise, const ContentProvider &provider,
                                 const QString &collectionFile)
{
    ContentResult rootItem(createContentItem());
    const ContentProviderResult result = provider(collectionFile);
    for (const auto &contentsData : result) {
        const QString namespaceName = contentsData.namespaceName;
        const QString folderName = contentsData.folderName;
        for (const QByteArray &contents : contentsData.contentsList) {
            if (promise.isCanceled())
                return;

            if (contents.isEmpty())
                continue;

            QList<QHelpContentItem *> stack;
            QDataStream s(contents);
            while (true) {
                int depth = 0;
                QString link, title;
                s >> depth;
                s >> link;
                s >> title;
                if (title.isEmpty())
                    break;

// The example input (depth, link, title):
//
// 0 "graphicaleffects5.html" "Qt 5 Compatibility APIs: Qt Graphical Effects"
// 1 "qtgraphicaleffects5-index.html" "QML Types"
// 2 "qml-qt5compat-graphicaleffects-blend.html" "Blend Type Reference"
// 3 "qml-qt5compat-graphicaleffects-blend-members.html" "List of all members"
// 2 "qml-qt5compat-graphicaleffects-brightnesscontrast.html" "BrightnessContrast Type Reference"
//
// Thus, the valid order of depths is:
// 1. Whenever the item's depth is < 0, we insert the item as its depth is 0.
// 2. The first item's depth must be 0, otherwise we insert the item as its depth is 0.
// 3. When the previous depth was N, the next depth must be in range [0, N+1] inclusively.
//    If next item's depth is M > N+1, we insert the item as its depth is N+1.

                if (depth <= 0) {
                    stack.clear();
                } else if (depth < stack.size()) {
                    stack = stack.sliced(0, depth);
                } else if (depth > stack.size()) {
                    // Fill the gaps with the last item from the stack (or with the root).
                    // This branch handles the case when depths are broken, e.g. 0, 2, 2, 1.
                    // In this case, the 1st item is a root, and 2nd - 4th are all direct
                    // children of the 1st.
                    QHelpContentItem *substituteItem =
                            stack.isEmpty() ? rootItem.get() : stack.constLast();
                    while (depth > stack.size())
                        stack.append(substituteItem);
                }

                const QUrl url = constructUrl(namespaceName, folderName, link);
                QHelpContentItem *parent = stack.isEmpty() ? rootItem.get() : stack.constLast();
                stack.push_back(createContentItem(title, url, parent));
            }
        }
    }
    promise.addResult(rootItem);
}

static ContentProvider contentProviderFromFilterEngine(const QString &filter)
{
    return [filter](const QString &collectionFile) -> ContentProviderResult {
        QHelpCollectionHandler collectionHandler(collectionFile);
        if (!collectionHandler.openCollectionFile())
            return {};
        return collectionHandler.contentsForFilter(filter);
    };
}

static ContentProvider contentProviderFromAttributes(const QStringList &attributes)
{
    return [attributes](const QString &collectionFile) -> ContentProviderResult {
        QHelpCollectionHandler collectionHandler(collectionFile);
        if (!collectionHandler.openCollectionFile())
            return {};
        return collectionHandler.contentsForFilter(attributes);
    };
}

QFuture<ContentResult> QHelpEngineCore::requestContentForCurrentFilter() const
{
    const ContentProvider provider = usesFilterEngine()
            ? contentProviderFromFilterEngine(filterEngine()->activeFilter())
            : contentProviderFromAttributes(filterAttributes(d->currentFilter));
    return QtConcurrent::run(requestContentHelper, provider, collectionFile());
}

QFuture<ContentResult> QHelpEngineCore::requestContent(const QString &filter) const
{
    const ContentProvider provider = usesFilterEngine()
            ? contentProviderFromFilterEngine(filter)
            : contentProviderFromAttributes(filterAttributes(filter));
    return QtConcurrent::run(requestContentHelper, provider, collectionFile());
}

using IndexProvider = std::function<QStringList(const QString &)>;
using IndexResult = QStringList;

static IndexProvider indexProviderFromFilterEngine(const QString &filter)
{
    return [filter](const QString &collectionFile) -> IndexResult {
        QHelpCollectionHandler collectionHandler(collectionFile);
        if (!collectionHandler.openCollectionFile())
            return {};
        return collectionHandler.indicesForFilter(filter);
    };
}

static IndexProvider indexProviderFromAttributes(const QStringList &attributes)
{
    return [attributes](const QString &collectionFile) -> IndexResult {
        QHelpCollectionHandler collectionHandler(collectionFile);
        if (!collectionHandler.openCollectionFile())
            return {};
        return collectionHandler.indicesForFilter(attributes);
    };
}

QFuture<IndexResult> QHelpEngineCore::requestIndexForCurrentFilter() const
{
    const IndexProvider provider = usesFilterEngine()
            ? indexProviderFromFilterEngine(filterEngine()->activeFilter())
            : indexProviderFromAttributes(filterAttributes(d->currentFilter));
    return QtConcurrent::run(std::move(provider), collectionFile());
}

QFuture<IndexResult> QHelpEngineCore::requestIndex(const QString &filter) const
{
    const IndexProvider provider = usesFilterEngine()
            ? indexProviderFromFilterEngine(filter)
            : indexProviderFromAttributes(filterAttributes(filter));
    return QtConcurrent::run(std::move(provider), collectionFile());
}
#endif

QT_END_NAMESPACE
