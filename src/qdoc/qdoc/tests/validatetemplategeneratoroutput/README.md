# tst_validateTemplateGeneratorOutput

This test validates the template-based generator output in QDoc. It's only built
and run when the template generator is enabled via the
`QT_QDOC_ENABLE_TEMPLATE_GENERATOR` CMake option.

## Test Structure

This test follows the same structure as `tst_validateQdocOutputFiles`:

- **testdata/**: Contains test projects organized in subdirectories.
- Each test project must have:
  - A `.qdocconf` file with the same name as its parent directory.
  - A `src/` directory with source files.
  - An `expected/` directory with expected output.
- The test runs QDoc on each project and compares the actual output to the
  expected output using `git diff`.

## Adding a New Test Case

1. Create a new directory in `testdata/` with a descriptive name
   (e.g., `my_template_test`).
2. Create a `.qdocconf` file with the same name:
   `testdata/my_template_test/my_template_test.qdocconf`.
3. Set `outputformats = template` in the config file.
4. Add source files to `testdata/my_template_test/src/`.
5. Run QDoc to generate output, verify correctness.
6. Copy the generated output to `testdata/my_template_test/expected/`.

The test will automatically discover and run your new test case.

## Regenerating Expected Output

To regenerate expected output for all test cases:

```bash
QDOC_REGENERATE_TESTDATA=1 ctest -R validateTemplateGeneratorOutput
```

## Relationship to tst_validateQdocOutputFiles

This test is separate from `tst_validateQdocOutputFiles` because:
- It requires the template generator feature to be enabled.
- It tests template-specific functionality.
- It allows template tests to run only when QDoc is built with template support.
- It prevents test failures when the template generator is disabled (the default).

