#!/usr/bin/env perl
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

sub makeit2($$$)
{
    for (my $i = 0; $i < (1 << $_[0]); $i++) {
        print OUTFILE "\n";
        print OUTFILE "$_[2]\n" unless $3 eq "";
        print OUTFILE "msgid \"singular $_[1] $i\"\n";
        print OUTFILE "msgid_plural \"plural $_[1] $i\"\n";
        for (my $j = 0; $j < $_[0]; $j++) {
            my $tr;
            if (($i & (1 << $j)) == 0) {
                $tr = "translated $_[1] $i $j";
            }
            print OUTFILE "msgstr[$j] \"$tr\"\n";
        }
    }
}

sub makeit($$$)
{
    open OUTFILE, ">${OUTDIR}plural-$_[0].po" || die "cannot write file in $OUTDIR";
    print OUTFILE <<EOF;
msgid ""
msgstr ""
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-FooBar: yup\\n"
"X-Language: $_[1]\\n"
"Plural-Forms: $_[2]\\n"
EOF
    makeit2($_[0], "one", "");
    makeit2($_[0], "two", "#, fuzzy
#| msgid \"old untranslated one\"");
    makeit2($_[0], "three", "#, fuzzy
#| msgid \"old untranslated two\"
#| msgid_plural \"old untranslated plural two\"");
    makeit2($_[0], "four", "#, fuzzy
#| msgid_plural \"old untranslated only plural three\"");
}

$OUTDIR = $ARGV[0];
makeit(1, "zh_CN", "nplurals=1; plural=0;");
makeit(2, "de_DE", "nplurals=2; plural=(n != 1);");
makeit(3, "pl_PL", "nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);");
