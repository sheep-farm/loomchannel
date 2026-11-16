# Qt6 Dock Windows demo for the LoomChannel bus.
# Open this file in Qt Creator (File > Open File or Project...).
#
# Two QDockWidgets: one transmitter that owns and opens/closes a
# channel, and one receiver that shows messages and the channel
# open/closed state as reported by the bus events (Broadcast / Stop).

QT += core gui widgets

CONFIG += c++17
TEMPLATE = app
TARGET = loomchannel_qtdemo

# The library is compiled in, so the project is self-contained and
# builds from Qt Creator without a prebuilt libloomchannel.a.
LOOM_ROOT = ..
INCLUDEPATH += $$LOOM_ROOT/include

SOURCES += \
    $$LOOM_ROOT/src/bus.cpp \
    $$LOOM_ROOT/src/channel.cpp \
    $$LOOM_ROOT/src/ordered_channel.cpp

SOURCES += \
    src/main.cpp \
    src/app_theme.cpp \
    src/transmitter_pane.cpp \
    src/editor_pane.cpp \
    src/receiver_pane.cpp \
    src/text_constraints.cpp \
    src/keyword_pane.cpp

HEADERS += \
    src/channels.h \
    src/app_theme.h \
    src/text_constraints.h \
    src/transmitter_pane.h \
    src/editor_pane.h \
    src/receiver_pane.h \
    src/keyword_pane.h

# Offscreen verification: run with a file path argument to save a
# screenshot and exit (QT_QPA_PLATFORM=offscreen).
