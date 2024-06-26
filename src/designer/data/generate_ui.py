#!/usr/bin/python3

# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import os
import subprocess

from argparse import ArgumentParser, RawTextHelpFormatter
from pathlib import Path
from tempfile import NamedTemporaryFile


DESCRIPTION = """
Usage: generate_ui.py

Generates the source files ui4.cpp, ui4.h used in the uic tool, the QtUiTools library and
Qt Widgets Designer from the XML schema used for .ui files.

Requires xalan.
"""


opt_delete_temp_files = True


def read_cpp_license(path):
    """Read out the license from a C++ source"""
    result = ""
    for line in path.read_text().splitlines():
        result += line + "\n"
        if 'SPDX-License-Identifier' in line:
            break
    return result


def replace_xsl_keys(xsl_source_file, license, ui_header_name=None):
    """Replace special keys in XSL files and return a handle to temporary file"""
    xsl = xsl_source_file.read_text()
    xsl = xsl.replace("@LICENSE@", license)
    if ui_header_name:
        xsl = xsl.replace("@HEADER@", ui_header_name)

    result = NamedTemporaryFile(mode='w', suffix='.xsl',
                                dir=Path.cwd(),
                                delete=opt_delete_temp_files)
    result.write(xsl)
    return result


def run_xslt(source, sheet, target):
    """Run xalan."""
    cmd = ['xalan', '-in', os.fspath(source), '-xsl', os.fspath(sheet),
           '-out', os.fspath(target)]
    subprocess.check_call(cmd)


if __name__ == '__main__':
    argument_parser = ArgumentParser(description=DESCRIPTION,
                                     formatter_class=RawTextHelpFormatter)
    argument_parser.add_argument('--keep', '-k', action='store_true',
                                 help='Keep temporary files')
    options = argument_parser.parse_args()
    opt_delete_temp_files = not options.keep

    # Generate uilib header and source.
    xml_dir = Path(__file__).parent.resolve()
    ui4_xsd = xml_dir / 'ui4.xsd'

    designer_dir = xml_dir.parent
    uilib_dir = designer_dir / "src" / "lib" / "uilib"
    uilib_impl = uilib_dir / 'ui4.cpp'
    license = read_cpp_license(uilib_impl)

    print("Running XSLT processor for uilib header...\n")
    header_xsl_source = xml_dir / 'generate_header.xsl'
    header_xsl = replace_xsl_keys(header_xsl_source, license)
    run_xslt(ui4_xsd, header_xsl.name, uilib_dir / 'ui4_p.h')

    print("Running XSLT processor for uilib source...\n")
    impl_xsl_source = xml_dir / 'generate_impl.xsl'
    impl_xsl = replace_xsl_keys(impl_xsl_source, license, 'ui4_p.h')
    run_xslt(ui4_xsd, impl_xsl.name, uilib_impl)

    # uic: Header is called 'ui4.h' instead of 'ui4_p.h'
    uic_dir = designer_dir.parents[2] / "qtbase" / "src" / "tools" / "uic"
    uic_impl = uic_dir / 'ui4.cpp'
    license = read_cpp_license(uic_impl)
    print("Running XSLT processor for uic header...\n")
    header_xsl = replace_xsl_keys(header_xsl_source, license)
    run_xslt(ui4_xsd, header_xsl.name, uic_dir / 'ui4.h')
    print("Running XSLT processor for uic source...\n")
    impl_xsl = replace_xsl_keys(impl_xsl_source, license, 'ui4.h')
    run_xslt(ui4_xsd, impl_xsl.name, uic_impl)

    subprocess.call(['git', 'diff'])
