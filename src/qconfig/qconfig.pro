TEMPLATE	= app
QT += widgets
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
HEADERS		= feature.h featuretreemodel.h graphics.h
SOURCES		= main.cpp feature.cpp featuretreemodel.cpp
TARGET		= qconfig
