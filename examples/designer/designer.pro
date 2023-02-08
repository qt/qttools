TEMPLATE    = subdirs
SUBDIRS     = calculatorform

!contains(CONFIG, static) {
        SUBDIRS += calculatorbuilder \
                   containerextension \
                   customwidgetplugin \
                   taskmenuextension
}

# the sun cc compiler has a problem with the include lines for the form.prf
solaris-cc*:SUBDIRS -= calculatorbuilder \
                       worldtimeclockbuilder
