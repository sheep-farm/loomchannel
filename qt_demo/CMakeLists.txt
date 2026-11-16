# ---------------------------------------------------------------
# Qt6 Dock Window demo, built on top of the loomchannel library.
# Two QDockWidgets: one transmitter that owns and opens/closes a
# channel, and one receiver that shows messages and the channel
# open/closed state as reported by the bus events.
# ---------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(loomchannel_qtdemo
    src/main.cpp
    src/channels.h
    src/app_theme.h
    src/app_theme.cpp
    src/text_constraints.h
    src/text_constraints.cpp
    src/transmitter_pane.h
    src/transmitter_pane.cpp
    src/editor_pane.h
    src/editor_pane.cpp
    src/receiver_pane.h
    src/receiver_pane.cpp
    src/keyword_pane.h
    src/keyword_pane.cpp
)

target_link_libraries(loomchannel_qtdemo PRIVATE
    loomchannel
    Qt6::Widgets
)

if(NOT MSVC)
    target_compile_options(loomchannel_qtdemo PRIVATE -Wall -Wextra -Wpedantic)
endif()
