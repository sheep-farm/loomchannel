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
    src/text_constraints.h
    src/text_constraints.cpp
    src/sender_dock.h
    src/sender_dock.cpp
    src/receiver_base.h
    src/receiver_base.cpp
    src/receiver_dock.h
    src/receiver_dock.cpp
    src/filtered_receiver_dock.h
    src/filtered_receiver_dock.cpp
    src/keyword_receiver_dock.h
    src/keyword_receiver_dock.cpp
)

target_link_libraries(loomchannel_qtdemo PRIVATE
    loomchannel
    Qt6::Widgets
)

if(NOT MSVC)
    target_compile_options(loomchannel_qtdemo PRIVATE -Wall -Wextra -Wpedantic)
endif()
