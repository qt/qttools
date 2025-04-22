<!--
    Copyright (C) 2024 The Qt Company Ltd.
    SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only
-->

# QDoc output test
This is a test that validates the files QDoc generates. It is not a test for
QDoc itself. The test calls the qdoc executable for each project in the
`testdata` directory (see [Test project structure](# Test project structure),
below), and then compares the output to that in the `expected` directory for
the test project. Each test is identified in the test output by the name of the
test project directory *and* the name of the .qdocconf-file.

The test creates a new temporary directory for each test project. The
comparison is done by running `git diff` on the content in this temporary
directory (the output directory) and the contents in the test's `expected`
directory. A non-zero exit code fails the test for that project, and the test
includes the diff in its output if this happens.

## How to add a new test
1. Create a new directory in the `testdata` directory. The name of the new
   directory should be descriptive of the test project.
2. Create a `.qdocconf` file in the new directory. See
   [The .qdocconf file](# The .qdocconf file) below for further details.
3. If the test requires additional command line arguments passed to QDoc,
   create an `args.txt` file in the same location as the main .qdocconf
   file. See [The args.txt file](# The args.txt file) below.
4. Add the necessary files (.qdoc, .h/.cpp, .qml, etc) so that the project
   can be built in a meaningful manner.
5. Run QDoc on the project.
6. Verify that the output looks correct.
7. Copy the output into `[testdata/[new test directory]/expected`.
8. Run the test executable and verify that the test output includes a **PASS**
   line for the new test-case.
9. Push your change upstream.

## Update the expected content for all tests
If you make a change to QDoc that causes significant changes in output, you may
need to update the expected output for many, or even all, tests. If you set the
environment variable `QDOC_REGENERATE_TESTDATA=1` before running the test, all
current test data will be removed, and QDoc will be run on each project to
generate new output. Note, however, that the comparison per project is skipped
in this scenario.

## Test project structure
The `testdata` directory is where the test looks for projects to test. Each
project has its own directory, a `qdocconf` file, and an `expected`
directory that contains the expected file output from QDoc.

The project directory must contain a .qdocconf whose name matches that of the
project directory, otherwise the project won't be picked up by the test.

### The .qdocconf file
The `.qdocconf` file contains the configuration necessary to build the
QDoc project in the directory. Observe the following:
- **Important!** Make sure to set `locationinfo = false` to avoid test failures
  due to differences in the location information in the generated output.
- Test projects should be warning free, so set `warninglimit.enabled = true`.
- The name of the .qdocconf-file must be identical as that of the directory
  that contains it. This means that for a project **foo**, it should go into
  `testdata/foo` and the configuration file should be
  `testdata/foo/foo.qdocconf`.
- Any other `.qdocconf` file will not be picked up explicitly by the test.
  This means that for a test project **foo** that resides in `testdata/foo`,
  `testdata/foo/foo.qdocconf` will be treated as a test-case, while
  `testdata/foo/bar.qdocconf` will not. However, `foo.qdocconf` may contain
  `include(bar.qdocconf)`, and this will work as expected.
- The `warninglimit` should be set to the number of warnings expected
  from QDoc for that specific project, if any are to be expected at all.
- You probably want to configure all output formats, otherwise only the HTML
  output will be generated. The test will compare all output formats that are
  generated to the expected output, so remember to specify the output directory
  for each format. By convention, the output directories are named after the
     format, for example `html`, `docbook`, `webxml`, etc.
- Place the sources for your new test in a subdirectory of the test project
  directory. By convention, the sources are placed in a directory named `src`.

### The args.txt file
An optional `args.txt` file, located in the same directory as the main .qdocconf
file, is used for passing additional command line arguments to QDoc for a
particular test. The file can contain any argument(s) accepted by QDoc on the
command line.

### The `expected` directory
The `expected` directory contains the expected output from QDoc
for the project. If QDoc generates an empty directory (for example, it
always creates the `images/` directory for a project, whether
the project contains any images or not), that directory isn't tracked
in the `expected` directory, as **git** doesn't track directories, only
files.

