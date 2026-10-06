// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BASES_H
#define BASES_H

/*!
    \module UndocumentedBaseClasses
    \title Undocumented Base Classes
    \brief A test module for inheritance list pruning.
*/

/*!
    \class PublicBase
    \inmodule UndocumentedBaseClasses
    \brief A documented public base class.
*/
class PublicBase
{
};

/*!
    \class ExplicitInternal
    \inmodule UndocumentedBaseClasses
    \internal
    \brief A base class documented with \c \c \internal.
*/
class ExplicitInternal : public PublicBase
{
};

// UndocumentedMiddle has no documentation at all.
class UndocumentedMiddle : public PublicBase
{
};

// UndocumentedRoot has no documentation at all and no base classes.
class UndocumentedRoot
{
};

/*!
    \class ViaExplicit
    \inmodule UndocumentedBaseClasses
    \brief A documented class derived from an explicitly internal base.
*/
class ViaExplicit : public ExplicitInternal
{
};

/*!
    \class ViaUndocumented
    \inmodule UndocumentedBaseClasses
    \brief A documented class derived from an undocumented base.
*/
class ViaUndocumented : public UndocumentedMiddle
{
};

/*!
    \class OnlyUndocumentedRoot
    \inmodule UndocumentedBaseClasses
    \brief A documented class derived only from an undocumented base.
*/
class OnlyUndocumentedRoot : public UndocumentedRoot
{
};

#endif // BASES_H
