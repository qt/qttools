TEMPLATE = subdirs

module_qttools_src.subdir = src
module_qttools_src.target = module-qttools-src

module_qttools_tests.subdir = tests
module_qttools_tests.target = module-qttools-tests
module_qttools_tests.depends = module_qttools_src
module_qttools_tests.CONFIG = no_default_target no_default_install

SUBDIRS += module_qttools_src \
           module_qttools_tests \
