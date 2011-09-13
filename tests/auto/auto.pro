TEMPLATE=subdirs
SUBDIRS=\
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

SUBDIRS += linguist uitools
!cross_compile:SUBDIRS += host.pro
