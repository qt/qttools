// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtDesigner/extension.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractExtensionFactory

    \brief The QAbstractExtensionFactory class provides an interface
    for extension factories in \QD.

    \inmodule QtDesigner

    QAbstractExtensionFactory is not intended to be instantiated
    directly; use the QExtensionFactory instead.

    In \QD, extension factories are used to look up and create named
    extensions as they are required. For that reason, when
    implementing a custom extension, you must also create a
    QExtensionFactory, i.e a class that is able to make an instance of
    your extension, and register it using \QD's \l
    {QExtensionManager}{extension manager}.

    When an extension is required, \QD's \l
    {QExtensionManager}{extension manager} will run through all its
    registered factories calling QExtensionFactory::createExtension()
    for each until the first one that is able to create the requested
    extension for the selected object, is found. This factory will
    then make an instance of the extension.

    \sa QExtensionFactory, QExtensionManager
*/

/*!
    Destroys the extension factory.
*/
QAbstractExtensionFactory::~QAbstractExtensionFactory()
    = default;

/*!
    \fn QObject *QAbstractExtensionFactory::extension(QObject *object, const QString &iid) const

    Returns the extension specified by \a iid for the given \a object.
*/


/*!
    \class QAbstractExtensionManager

    \brief The QAbstractExtensionManager class provides an interface
    for extension managers in \QD.

    \inmodule QtDesigner

    QAbstractExtensionManager is not intended to be instantiated
    directly; use the QExtensionManager instead.

    In \QD, extension are not created until they are required. For
    that reason, when implementing a custom extension, you must also
    create a QExtensionFactory, i.e a class that is able to make an
    instance of your extension, and register it using \QD's \l
    {QExtensionManager}{extension manager}.

    When an extension is required, \QD's \l
    {QExtensionManager}{extension manager} will run through all its
    registered factories calling QExtensionFactory::createExtension()
    for each until the first one that is able to create the requested
    extension for the selected object, is found. This factory will
    then make an instance of the extension.

    \sa QExtensionManager, QExtensionFactory
*/

/*!
    Destroys the extension manager.
*/
QAbstractExtensionManager::~QAbstractExtensionManager()
    = default;

/*!
    \fn void QAbstractExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid)

    Register the given extension \a factory with the extension
    specified by \a iid.
*/

/*!
    \fn void QAbstractExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid)

    Unregister the given \a factory with the extension specified by \a
    iid.
*/

/*!
    \fn QObject *QAbstractExtensionManager::extension(QObject *object, const QString &iid) const

    Returns the extension, specified by \a iid, for the given \a
    object.
*/

/*!
   \fn template <class T> T qt_extension(QAbstractExtensionManager* manager, QObject *object)

   \relates QExtensionManager

   Returns the extension of the given \a object cast to type T if the
   object is of type T (or of a subclass); otherwise returns 0. The
   extension is retrieved using the given extension \a manager.

   \snippet lib/tools_designer_src_lib_extension_extension.cpp 0

   When implementing a custom widget plugin, a pointer to \QD's
   current QDesignerFormEditorInterface object (\c formEditor) is
   provided by the QDesignerCustomWidgetInterface::initialize()
   function's parameter.

   If the widget in the example above doesn't have a defined
   QDesignerPropertySheetExtension, \c propertySheet will be a null
   pointer.

*/

/*!
   \macro Q_DECLARE_EXTENSION_INTERFACE(ExtensionName, Identifier)

   \relates QExtensionManager

   Associates the given \a Identifier (a string literal) to the
   extension class called \a ExtensionName. The \a Identifier must be
   unique. For example:

   \snippet lib/tools_designer_src_lib_extension_extension.cpp 1

   Using the company and product names is a good way to ensure
   uniqueness of the identifier.

   When implementing a custom extension class, you must use
   Q_DECLARE_EXTENSION_INTERFACE() to enable usage of the
   qt_extension() function. The macro is normally located right after the
   class definition for \a ExtensionName, in the associated header
   file.

   \sa {Q_DECLARE_INTERFACE}{Q_DECLARE_INTERFACE()}
*/

QT_END_NAMESPACE
