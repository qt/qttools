<!--
    Copyright (C) 2024 The Qt Company Ltd.
    SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only
-->

# tst_multiModuleConfigurations

This test exercises QDoc behavior that requires more than one module to build
together. Each fixture declares an explicit build order, and the test driver
runs `qdoc` once per module, chaining `-indexdir` flags so that downstream
modules can resolve dependencies declared by `depends` on earlier modules.

This driver complements `tst_validateTemplateGeneratorOutput`, which covers
the single-module case. It also provides a migration target for the bespoke
multi-module assertions in `tst_generatedOutput`, which are brittle and
hard to extend.

## Fixture Layout

Each fixture is a subdirectory of `testdata/`. A fixture must contain:

- A `build-order.txt` file listing one qdocconf basename per line, in
  dependency order. Lines beginning with `#` are comments; empty lines are
  ignored. The last entry is the consumer module whose output is compared
  against `expected/`.
- One `.qdocconf` file per declared module, with the basename used in
  `build-order.txt` (e.g., `upstream_widgets.qdocconf`).
- Source files for each module, organised however the qdocconfs reference
  them (typically a `src/` subdirectory).
- An `expected/` directory holding the consumer module's reference output,
  diffed against the test run via `git diff`.

By convention the consumer module's qdocconf basename matches the fixture
directory name (e.g., a `testdata/cross_module_links/` fixture declares
`cross_module_links` as the last entry in `build-order.txt`), but this is
not enforced.

A per-fixture `args.txt` file is supported and, if present, its contents
are appended to every qdoc invocation in the fixture.

## Adding a New Fixture

1. Create `testdata/<fixture-name>/`.
2. Add `build-order.txt` listing the modules in dependency order:
   ```
   # Upstream module first; consumer module last.
   upstream_module
   <fixture-name>
   ```
3. Author each module's `.qdocconf` next to `build-order.txt`. Use
   `depends = <other-module>` to declare cross-module references.
4. Add source files to the locations each qdocconf references.
5. Run the test once with regen enabled, then hand-verify the output:
   ```sh
   cd build/qttools
   QDOC_REGENERATE_TESTDATA=1 ctest -R "multiModuleConfigurations.*<fixture-name>"
   ```
6. Inspect the populated `expected/` directory before committing.

## Regenerating Expected Output

```sh
QDOC_REGENERATE_TESTDATA=1 ctest -R multiModuleConfigurations
```

The driver removes each fixture's `expected/` directory before regenerating,
so any hand-edits there will be lost. Treat regen as a starting point for
hand-verification, not a substitute for it.

## Relationship to Other Test Drivers

- **`tst_validateTemplateGeneratorOutput`**: single-module template generator
  fixtures. Use that driver when a fixture exercises one qdocconf.
- **`tst_validateQdocOutputFiles`**: single-module legacy generator fixtures.
- **`tst_generatedOutput`**: legacy hand-written multi-module assertions
  (`indexLinking`, `crossModuleLinking`, `singleExec`). The current driver
  is intended to absorb those over time.

