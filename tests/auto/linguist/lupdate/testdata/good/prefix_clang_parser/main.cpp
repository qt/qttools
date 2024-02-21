// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

























#include "main.h"

QString foo()
{
	return QCoreApplication::translate("Foo","XXX","YYY");
}

Foo::Foo()
{
	tr("CTOR");
}

void Foo::bar()
{
	tr("BAR");
}

class BitFields : public QObject
{
    int bits : 20;
    QString str = tr("text BitFields");
};

Bibi::Bibi()
{
    //int bits : 32;
    tr("text Bibi");
    Babebi::tr("text Babebi");
}
