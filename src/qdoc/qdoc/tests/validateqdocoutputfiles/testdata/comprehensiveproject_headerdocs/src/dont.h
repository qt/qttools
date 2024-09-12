// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

/*!
    \class UnseenClass
    \inmodule TestCPP
    \brief A public but undocumented class.
*/
class UnseenClass
{
public:
    UnseenClass();
};

/*!
    \class SeenClass
    \inmodule TestCPP
    \brief A public but undocumented class.
*/
class SeenClass : public UnseenClass
{
public:
    SeenClass();
};
