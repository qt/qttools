TEMPLATE = aux

DISTFILES += data/qt_attribution.json data/LICENSE

run_qtattributionsscanner.commands = $$[QT_HOST_BINS]/qtattributionsscanner -o generated.qdoc $$PWD

check.depends = run_qtattributionsscanner

QMAKE_EXTRA_TARGETS += run_qtattributionsscanner check

QMAKE_CLEAN += generated.qdoc
