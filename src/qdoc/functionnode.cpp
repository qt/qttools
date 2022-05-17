// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "functionnode.h"

#include "propertynode.h"

QT_BEGIN_NAMESPACE

/*!
  \class FunctionNode

  This node is used to represent any kind of function being
  documented. It can represent a C++ class member function,
  a C++ global function, a QML method, a javascript method,
  or a macro, with or without parameters.

  A C++ function can be a signal a slot, a constructor of any
  kind, a destructor, a copy or move assignment operator, or
  just a plain old member function or global function.

  A QML or javascript method can be a plain old method, or a
  signal or signal handler.

  If the function is not an overload, its overload flag is
  false. If it is an overload, its overload flag is true.
  If it is not an overload but it has overloads, its next
  overload pointer will point to an overload function. If it
  is an overload function, its overload flag is true, and it
  may or may not have a non-null next overload pointer.

  So all the overloads of a function are in a linked list
  using the next overload pointer. If a function has no
  overloads, its overload flag is false and its overload
  pointer is null.

  The function node also has an overload number. If the
  node's overload flag is set, this overload number is
  positive; otherwise, the overload number is 0.
 */

/*!
  Construct a function node for a C++ function. It's parent
  is \a parent, and it's name is \a name.

  \note The function node's overload flag is set to false, and
  its overload number is set to 0. These data members are set
  in normalizeOverloads(), when all the overloads are known.
 */
FunctionNode::FunctionNode(Aggregate *parent, const QString &name)
    : Node(Function, parent, name),
      m_const(false),
      m_default(false),
      m_static(false),
      m_reimpFlag(false),
      m_attached(false),
      m_overloadFlag(false),
      m_isFinal(false),
      m_isOverride(false),
      m_isRef(false),
      m_isRefRef(false),
      m_isInvokable(false),
      m_metaness(Plain),
      m_virtualness(NonVirtual),
      m_overloadNumber(0),
      m_nextOverload(nullptr)
{
    // nothing
}

/*!
  Construct a function node for a QML method or signal, specified
  by ther Metaness value \a type. It's parent is \a parent, and
  it's name is \a name. If \a attached is true, it is an attached
  method or signal.

  \note The function node's overload flag is set to false, and
  its overload number is set to 0. These data members are set
  in normalizeOverloads(), when all the overloads are known.
 */
FunctionNode::FunctionNode(Metaness kind, Aggregate *parent, const QString &name, bool attached)
    : Node(Function, parent, name),
      m_const(false),
      m_default(false),
      m_static(false),
      m_reimpFlag(false),
      m_attached(attached),
      m_overloadFlag(false),
      m_isFinal(false),
      m_isOverride(false),
      m_isRef(false),
      m_isRefRef(false),
      m_isInvokable(false),
      m_metaness(kind),
      m_virtualness(NonVirtual),
      m_overloadNumber(0),
      m_nextOverload(nullptr)
{
    setGenus(getGenus(m_metaness));
    if (!isCppNode() && name.startsWith("__"))
        setStatus(Internal);
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *FunctionNode::clone(Aggregate *parent)
{
    auto *fn = new FunctionNode(*this); // shallow copy
    fn->setParent(nullptr);
    fn->setNextOverload(nullptr);
    parent->addChild(fn);
    return fn;
}

/*!
  Returns this function's virtualness value as a string
  for use as an attribute value in index files.
 */
QString FunctionNode::virtualness() const
{
    switch (m_virtualness) {
    case FunctionNode::NormalVirtual:
        return QLatin1String("virtual");
    case FunctionNode::PureVirtual:
        return QLatin1String("pure");
    case FunctionNode::NonVirtual:
    default:
        break;
    }
    return QLatin1String("non");
}

/*!
  Sets the function node's virtualness value based on the value
  of string \a value, which is the value of the function's \e{virtual}
  attribute in an index file. If \a value is \e{pure}, and if the
  parent() is a C++ class, set the parent's \e abstract flag to
  \c {true}.
 */
void FunctionNode::setVirtualness(const QString &value)
{
    if (value == QLatin1String("non"))
        m_virtualness = NonVirtual;
    else if (value == QLatin1String("virtual"))
        m_virtualness = NormalVirtual;
    else if (value == QLatin1String("pure")) {
        m_virtualness = PureVirtual;
        if (parent() && parent()->isClassNode())
            parent()->setAbstract(true);
    }
}

static QMap<QString, FunctionNode::Metaness> metanessMap_;
static void buildMetanessMap()
{
    metanessMap_["plain"] = FunctionNode::Plain;
    metanessMap_["signal"] = FunctionNode::Signal;
    metanessMap_["slot"] = FunctionNode::Slot;
    metanessMap_["constructor"] = FunctionNode::Ctor;
    metanessMap_["copy-constructor"] = FunctionNode::CCtor;
    metanessMap_["move-constructor"] = FunctionNode::MCtor;
    metanessMap_["destructor"] = FunctionNode::Dtor;
    metanessMap_["macro"] = FunctionNode::MacroWithParams;
    metanessMap_["macrowithparams"] = FunctionNode::MacroWithParams;
    metanessMap_["macrowithoutparams"] = FunctionNode::MacroWithoutParams;
    metanessMap_["copy-assign"] = FunctionNode::CAssign;
    metanessMap_["move-assign"] = FunctionNode::MAssign;
    metanessMap_["native"] = FunctionNode::Native;
    metanessMap_["qmlsignal"] = FunctionNode::QmlSignal;
    metanessMap_["qmlsignalhandler"] = FunctionNode::QmlSignalHandler;
    metanessMap_["qmlmethod"] = FunctionNode::QmlMethod;
    metanessMap_["jssignal"] = FunctionNode::JsSignal;
    metanessMap_["jssignalhandler"] = FunctionNode::JsSignalHandler;
    metanessMap_["jsmethos"] = FunctionNode::JsMethod;
}

static QMap<QString, FunctionNode::Metaness> topicMetanessMap_;
static void buildTopicMetanessMap()
{
    topicMetanessMap_["fn"] = FunctionNode::Plain;
    topicMetanessMap_["qmlsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlattachedsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlmethod"] = FunctionNode::QmlMethod;
    topicMetanessMap_["qmlattachedmethod"] = FunctionNode::QmlMethod;
    topicMetanessMap_["jssignal"] = FunctionNode::JsSignal;
    topicMetanessMap_["jsattachedsignal"] = FunctionNode::JsSignal;
    topicMetanessMap_["jsmethod"] = FunctionNode::JsMethod;
    topicMetanessMap_["jsattachedmethod"] = FunctionNode::JsMethod;
}

/*!
  Determines the Genus value for this FunctionNode given the
  Metaness value \a metaness. Returns the Genus value. \a metaness must be
  one of the values of Metaness. If not, Node::DontCare is
  returned.
 */
Node::Genus FunctionNode::getGenus(FunctionNode::Metaness metaness)
{
    switch (metaness) {
    case FunctionNode::Plain:
    case FunctionNode::Signal:
    case FunctionNode::Slot:
    case FunctionNode::Ctor:
    case FunctionNode::Dtor:
    case FunctionNode::CCtor:
    case FunctionNode::MCtor:
    case FunctionNode::MacroWithParams:
    case FunctionNode::MacroWithoutParams:
    case FunctionNode::Native:
    case FunctionNode::CAssign:
    case FunctionNode::MAssign:
        return Node::CPP;
    case FunctionNode::QmlSignal:
    case FunctionNode::QmlSignalHandler:
    case FunctionNode::QmlMethod:
        return Node::QML;
    case FunctionNode::JsSignal:
    case FunctionNode::JsSignalHandler:
    case FunctionNode::JsMethod:
        return Node::JS;
    }
    return Node::DontCare;
}

/*!
  This static function converts the string \a value to an enum
  value for the kind of function named by \a value.
 */
FunctionNode::Metaness FunctionNode::getMetaness(const QString &value)
{
    if (metanessMap_.isEmpty())
        buildMetanessMap();
    return metanessMap_[value];
}

/*!
  This static function converts the topic string \a topic to an enum
  value for the kind of function this FunctionNode represents.
 */
FunctionNode::Metaness FunctionNode::getMetanessFromTopic(const QString &topic)
{
    if (topicMetanessMap_.isEmpty())
        buildTopicMetanessMap();
    return topicMetanessMap_[topic];
}

/*!
  If this function node's metaness is \a from, change the
  metaness to \a to and return \c true. Otherwise return
  false. This function is used to change Qml function node
  metaness values to Javascript function node metaness,
  values because these nodes are created as Qml function
  nodes before it is discovered that what the function node
  represents is not a Qml function but a javascript function.

  Note that if the function returns true, which means the node
  type was indeed changed, then the node's Genus is also changed
  from QML to JS.

  The function also works in the other direction, but there is
  no use case for that.
 */
bool FunctionNode::changeMetaness(Metaness from, Metaness to)
{
    if (m_metaness == from) {
        m_metaness = to;
        switch (to) {
        case QmlSignal:
        case QmlSignalHandler:
        case QmlMethod:
            setGenus(Node::QML);
            break;
        case JsSignal:
        case JsSignalHandler:
        case JsMethod:
            setGenus(Node::JS);
            break;
        default:
            setGenus(Node::CPP);
            break;
        }
        return true;
    }
    return false;
}

/*!
  Sets the function node's overload number to \a number. If \a number
  is 0, the function node's overload flag is set to false. If
  \a number is greater than 0, the overload flag is set to true.
 */
void FunctionNode::setOverloadNumber(signed short number)
{
    m_overloadNumber = number;
    m_overloadFlag = (number > 0);
}

/*!
  Appends \a functionNode to the linked list of overloads for this function.

  \note Although this function appends an overload function to the list of
  overloads for this function's name, it does not set the function's
  overload number or it's overload flag. If the function has the
  \c{\\overload} in its QDoc comment, that will set the overload
  flag. But qdoc treats the \c{\\overload} command as a hint that the
  function should be documented as an overload. The hint is almost
  always correct, but QDoc reserves the right to decide which function
  should be the primary function and which functions are the overloads.
  These decisions are made in Aggregate::normalizeOverloads().
 */
void FunctionNode::appendOverload(FunctionNode *functionNode)
{
    auto current = this;
    while (current->m_nextOverload)
        current = current->m_nextOverload;
    current->m_nextOverload = functionNode;
    functionNode->m_nextOverload = nullptr;
}

/*!
  Removes \a functionNode from the linked list of function overloads.
*/
void FunctionNode::removeOverload(FunctionNode *functionNode)
{
    auto head = this;
    auto **indirect = &head;
    while ((*indirect) != functionNode) {
        if (!(*indirect)->m_nextOverload)
            return;
        indirect = &(*indirect)->m_nextOverload;
    }
    *indirect = functionNode->m_nextOverload;
}

/*!
  Returns the primary function - the first function
  from the linked list of overloads that is \e not
  marked as an overload. If found, the primary function
  is removed from the list and returned. Otherwise
  returns \c nullptr.
 */
FunctionNode *FunctionNode::findPrimaryFunction()
{
    auto current = this;
    while (current->m_nextOverload && current->m_nextOverload->isOverload())
        current = current->m_nextOverload;

    auto primary = current->m_nextOverload;
    if (primary)
        current->m_nextOverload = primary->m_nextOverload;

    return primary;
}

/*!
  \fn void FunctionNode::setReimpFlag()

  Sets the function node's reimp flag to \c true, which means
  the \e {\\reimp} command was used in the qdoc comment. It is
  supposed to mean that the function reimplements a virtual
  function in a base class.
 */

/*!
  Returns a string representing the kind of function this
  Function node represents, which depends on the Metaness
  value.
 */
QString FunctionNode::kindString() const
{
    switch (m_metaness) {
    case FunctionNode::QmlSignal:
        return "QML signal";
    case FunctionNode::QmlSignalHandler:
        return "QML signal handler";
    case FunctionNode::QmlMethod:
        return "QML method";
    case FunctionNode::JsSignal:
        return "JS signal";
    case FunctionNode::JsSignalHandler:
        return "JS signal handler";
    case FunctionNode::JsMethod:
        return "JS method";
    default:
        return "function";
    }
}

/*!
  Returns a string representing the Metaness enum value for
  this function. It is used in index files.
 */
QString FunctionNode::metanessString() const
{
    switch (m_metaness) {
    case FunctionNode::Plain:
        return "plain";
    case FunctionNode::Signal:
        return "signal";
    case FunctionNode::Slot:
        return "slot";
    case FunctionNode::Ctor:
        return "constructor";
    case FunctionNode::CCtor:
        return "copy-constructor";
    case FunctionNode::MCtor:
        return "move-constructor";
    case FunctionNode::Dtor:
        return "destructor";
    case FunctionNode::MacroWithParams:
        return "macrowithparams";
    case FunctionNode::MacroWithoutParams:
        return "macrowithoutparams";
    case FunctionNode::Native:
        return "native";
    case FunctionNode::CAssign:
        return "copy-assign";
    case FunctionNode::MAssign:
        return "move-assign";
    case FunctionNode::QmlSignal:
        return "qmlsignal";
    case FunctionNode::QmlSignalHandler:
        return "qmlsignalhandler";
    case FunctionNode::QmlMethod:
        return "qmlmethod";
    case FunctionNode::JsSignal:
        return "jssignal";
    case FunctionNode::JsSignalHandler:
        return "jssignalhandler";
    case FunctionNode::JsMethod:
        return "jsmethod";
    default:
        return "plain";
    }
}

/*!
  Adds the "associated" property \a p to this function node.
  The function might be the setter or getter for a property,
  for example.
 */
void FunctionNode::addAssociatedProperty(PropertyNode *p)
{
    m_associatedProperties.append(p);
}

/*!
    \reimp

    Returns \c true if this is an access function for an obsolete property,
    otherwise calls the base implementation of isDeprecated().
*/
bool FunctionNode::isDeprecated() const
{
    auto it = std::find_if_not(m_associatedProperties.begin(), m_associatedProperties.end(),
                               [](const Node *p) -> bool { return p->isDeprecated(); });

    if (!m_associatedProperties.isEmpty() && it == m_associatedProperties.end())
        return true;

    return Node::isDeprecated();
}

/*! \fn unsigned char FunctionNode::overloadNumber() const
  Returns the overload number for this function.
 */

/*!
  Reconstructs and returns the function's signature. If \a values
  is \c true, the default values of the parameters are included.
  The return type is included unless \a noReturnType is \c true.
  Function templates are prefixed with \c {template <parameter_list>}
  if \a templateParams is \c true.
 */
QString FunctionNode::signature(bool values, bool noReturnType, bool templateParams) const
{
    QStringList elements;

    if (templateParams)
        elements << templateDecl();
    if (!noReturnType)
        elements << m_returnType;
    elements.removeAll(QString());

    if (!isMacroWithoutParams()) {
        elements << name() + QLatin1Char('(') + m_parameters.signature(values) + QLatin1Char(')');
        if (!isMacro()) {
            if (isConst())
                elements << QStringLiteral("const");
            if (isRef())
                elements << QStringLiteral("&");
            else if (isRefRef())
                elements << QStringLiteral("&&");
        }
    } else {
        elements << name();
    }
    return elements.join(QLatin1Char(' '));
}

/*!
  Print some information used for debugging qdoc. Only used when debugging.
 */
void FunctionNode::debug() const
{
    qDebug("QML METHOD %s m_returnType %s m_parentPath %s", qPrintable(name()),
           qPrintable(m_returnType), qPrintable(m_parentPath.join(' ')));
}

/*!
  Compares this FunctionNode to \a node. If \a sameParent is \c true, compares
  also the parent of the two nodes. Returns \c true if they describe
  the same function.
 */
bool FunctionNode::compare(const Node *node, bool sameParent) const
{
    if (!node || !node->isFunction())
        return false;

    const auto *functionNode = static_cast<const FunctionNode *>(node);
    if (metaness() != functionNode->metaness())
        return false;
    if (sameParent && parent() != functionNode->parent())
        return false;
    if (m_returnType != functionNode->returnType())
        return false;
    if (isConst() != functionNode->isConst())
        return false;
    if (isAttached() != functionNode->isAttached())
        return false;
    const Parameters &p = functionNode->parameters();
    if (m_parameters.count() != p.count())
        return false;
    if (!p.isEmpty()) {
        for (int i = 0; i < p.count(); ++i) {
            if (m_parameters.at(i).type() != p.at(i).type())
                return false;
        }
    }
    return true;
}

/*!
  In some cases, it is ok for a public function to be not documented.
  For example, the macro Q_OBJECT adds several functions to the API of
  a class, but these functions are normally not meant to be documented.
  So if a function node doesn't have documentation, then if its name is
  in the list of functions that it is ok not to document, this function
  returns true. Otherwise, it returns false.

  These are the member function names added by macros.  Usually they
  are not documented, but they can be documented, so this test avoids
  reporting an error if they are not documented.

  But maybe we should generate a standard text for each of them?
 */
bool FunctionNode::isIgnored() const
{
    if (!hasDoc() && !hasSharedDoc()) {
        if (name().startsWith(QLatin1String("qt_")) || name() == QLatin1String("metaObject")
            || name() == QLatin1String("tr") || name() == QLatin1String("trUtf8")
            || name() == QLatin1String("d_func")) {
            return true;
        }
        QString s = signature(false, false);
        if (s.contains(QLatin1String("enum_type")) && s.contains(QLatin1String("operator|")))
            return true;
    }
    return false;
}

/*!
  Returns true if this function has overloads. Otherwise false.
  First, if this function node's overload pointer is not nullptr,
  return true. Next, if this function node's overload flag is true
  return true. Finally, if this function's parent Aggregate has a
  function by the same name as this one in its function map and
  that function has overloads, return true. Otherwise return false.

  There is a failsafe way to test it under any circumstances.
 */
bool FunctionNode::hasOverloads() const
{
    if (m_nextOverload != nullptr)
        return true;
    if (m_overloadFlag)
        return true;
    if (parent())
        return parent()->hasOverloads(this);
    return false;
}

QT_END_NAMESPACE
