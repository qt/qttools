// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "functionnode.h"
#include "propertynode.h"

#include <string>

QT_BEGIN_NAMESPACE

/*!
  \class FunctionNode

  This node is used to represent any kind of function being
  documented. It can represent a C++ class member function, a C++
  global function, a QML method, or a macro, with or without
  parameters.

  A C++ function can be a signal, a slot, a constructor of any
  kind, a destructor, a copy or move assignment operator, or
  just a plain old member function or a global function.

  A QML method can be a plain old method, or a
  signal or signal handler.

  If the function is an overload, its overload flag is
  true.

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
    : Node(NodeType::Function, parent, name),
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
      m_explicit{false},
      m_constexpr{false},
      m_metaness(Plain),
      m_virtualness(NonVirtual),
      m_overloadNumber(0)
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
    : Node(NodeType::Function, parent, name),
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
      m_explicit{false},
      m_constexpr{false},
      m_metaness(kind),
      m_virtualness(NonVirtual),
      m_overloadNumber(0)
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
    if (value == QLatin1String("pure")) {
        m_virtualness = PureVirtual;
        if (parent() && parent()->isClassNode())
            parent()->setAbstract(true);
        return;
    }

    m_virtualness = (value == QLatin1String("virtual")) ? NormalVirtual : NonVirtual;
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
}

static QMap<QString, FunctionNode::Metaness> topicMetanessMap_;
static void buildTopicMetanessMap()
{
    topicMetanessMap_["fn"] = FunctionNode::Plain;
    topicMetanessMap_["qmlsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlattachedsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlmethod"] = FunctionNode::QmlMethod;
    topicMetanessMap_["qmlattachedmethod"] = FunctionNode::QmlMethod;
}

/*!
  Determines the Genus value for this FunctionNode given the
  Metaness value \a metaness. Returns the Genus value. \a metaness must be
  one of the values of Metaness. If not, Node::DontCare is
  returned.
 */
Genus FunctionNode::getGenus(FunctionNode::Metaness metaness)
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
        return Genus::CPP;
    case FunctionNode::QmlSignal:
    case FunctionNode::QmlSignalHandler:
    case FunctionNode::QmlMethod:
        return Genus::QML;
    }

    return Genus::DontCare;
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
  Returns the \e primary associated property, if this is an
  access function for one or more properties.

  An associated property is considered a primary if this
  function's name starts with the property name. If there's
  no such property, return the first one available as a
  fallback.

  If no associated properties exist, returns \nullptr.
 */
const PropertyNode *FunctionNode::primaryAssociatedProperty() const
{
    if (m_associatedProperties.isEmpty())
        return nullptr;
    if (m_associatedProperties.size() == 1)
        return m_associatedProperties[0];

    auto it = std::find_if(
            m_associatedProperties.cbegin(), m_associatedProperties.cend(),
                    [this](const PropertyNode *p) {
                        return name().startsWith(p->name());
                    });

    return it != m_associatedProperties.cend() ? *it : m_associatedProperties[0];
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
  Reconstructs and returns the function's signature.

  Specific parts of the signature are included according to
  flags in \a options:

  \value Node::SignaturePlain
         Plain signature
  \value Node::SignatureDefaultValues
         Include any default argument values
  \value Node::SignatureReturnType
         Include return type
  \value Node::SignatureTemplateParams
         Include \c {template <parameter_list>} if one exists
 */
QString FunctionNode::signature(Node::SignatureOptions options) const
{
    QStringList elements;

    if (options & Node::SignatureTemplateParams && templateDecl())
        elements << (*templateDecl()).to_qstring();
    if (options & Node::SignatureReturnType)
        elements << m_returnType.first;
    elements.removeAll(QString());

    if (!isMacroWithoutParams()) {
        elements << name() + QLatin1Char('(')
                + m_parameters.signature(options & Node::SignatureDefaultValues)
                + QLatin1Char(')');
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
  \fn int FunctionNode::compare(const FunctionNode *f1, const FunctionNode *f2)

  Compares FunctionNode \a f1 with \a f2, assumed to have identical names.
  Returns an integer less than, equal to, or greater than zero if f1 is
  considered less than, equal to, or greater than f2.

  The main purpose is to provide stable ordering for function overloads.
 */
[[nodiscard]] int compare(const FunctionNode *f1, const FunctionNode *f2)
{
    // Compare parameter count
    int param_count{f1->parameters().count()};

    if (int param_diff = param_count - f2->parameters().count(); param_diff != 0)
        return param_diff;

    // Constness
    if (f1->isConst() != f2->isConst())
        return f1->isConst() ? 1 : -1;

    // Reference qualifiers
    if (f1->isRef() != f2->isRef())
        return f1->isRef() ? 1 : -1;
    if (f1->isRefRef() != f2->isRefRef())
        return f1->isRefRef() ? 1 : -1;

    // Attachedness (applies to QML methods)
    if (f1->isAttached() != f2->isAttached())
        return f1->isAttached() ? 1 : -1;

    // Parameter types
    const Parameters &p1{f1->parameters()};
    const Parameters &p2{f2->parameters()};
    for (qsizetype i = 0; i < param_count; ++i) {
        if (int type_comp = QString::compare(p1.at(i).type(), p2.at(i).type());
                type_comp != 0) {
            return type_comp;
        }
    }

    // Template declarations
    const auto &t1{f1->templateDecl()};
    const auto &t2{f2->templateDecl()};
    if (!t1 && !t2)
        return 0;

    if (t1 && t2)
        return (*t1).to_std_string().compare((*t2).to_std_string());

    return t1 ? 1 : -1;
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
    if (!hasDoc()) {
        if (name().startsWith(QLatin1String("qt_")) || name() == QLatin1String("metaObject")
            || name() == QLatin1String("tr") || name() == QLatin1String("trUtf8")
            || name() == QLatin1String("d_func")) {
            return true;
        }
        QString s = signature(Node::SignatureReturnType);
        if (s.contains(QLatin1String("enum_type")) && s.contains(QLatin1String("operator|")))
            return true;
    }
    return false;
}

/*!
  \fn bool FunctionNode::hasOverloads() const
  Returns \c true if this function has overloads.
 */

/*!
    \internal
    \brief Returns the type of the function as a string.

    The returned string is either the type as declared in the header, or `auto`
    if that's the return type in the `\\fn` command for the function.
 */
QString FunctionNode::returnTypeString() const
{
    if (m_returnType.second.has_value())
        return m_returnType.second.value();
    return m_returnType.first;
}
QT_END_NAMESPACE
