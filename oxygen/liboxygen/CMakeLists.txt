################# X11 #################
add_definitions( -DHAVE_X11=${X11_FOUND} )

################# liboxygenstyle #################
set(oxygenstyle_LIB_SRCS
    oxygenanimation.cpp
    oxygenhelper.cpp
    oxygenitemmodel.cpp
    oxygenshadowcache.cpp
    oxygentileset.cpp
    oxygenutil.cpp
)

kconfig_add_kcfg_files(oxygenstyle_LIB_SRCS
  oxygenactiveshadowconfiguration.kcfgc
  oxygeninactiveshadowconfiguration.kcfgc
)

add_library(oxygenstyle ${oxygenstyle_LIB_SRCS})
generate_export_header(oxygenstyle BASE_NAME "OXYGEN" EXPORT_FILE_NAME oxygen_export.h)

# NB: although we install no headers, we still need to have a link
#     interface, since other Oxygen targets link to this library
target_link_libraries(oxygenstyle
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        KF5::ConfigCore
        KF5::ConfigWidgets
    PRIVATE
        KF5::GuiAddons
)
target_include_directories(oxygenstyle PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
if( X11_FOUND )
    target_link_libraries(oxygenstyle PRIVATE ${XCB_LIBRARIES} Qt5::X11Extras)
endif()

set_target_properties(oxygenstyle PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS oxygenstyle ${INSTALL_TARGETS_DEFAULT_ARGS} )

################# liboxygenstyleconfig #################
set(oxygenstyleconfig_LIB_SRCS
    oxygenanimationconfigitem.cpp
    oxygenbaseanimationconfigwidget.cpp
    oxygengenericanimationconfigitem.cpp
    oxygenshadowconfigwidget.cpp
)

set(oxygenstyleconfig_LIB_FORMS
    ui/oxygenanimationconfigitem.ui
    ui/oxygenanimationconfigwidget.ui
    ui/oxygengenericanimationconfigbox.ui
    ui/oxygenshadowconfigurationui.ui
)

qt5_wrap_ui(oxygenstyleconfig_LIB_FORMS_HEADERS ${oxygenstyleconfig_LIB_FORMS})

kconfig_add_kcfg_files(oxygenstyleconfig_LIB_SRCS
    oxygenactiveshadowconfiguration.kcfgc
    oxygeninactiveshadowconfiguration.kcfgc
)

add_library(oxygenstyleconfig
    ${oxygenstyleconfig_LIB_SRCS}
    ${oxygenstyleconfig_LIB_FORMS_HEADERS} )

# NB: although we install no headers, we still need to have a link
#     interface, since other Oxygen targets link to this library
target_link_libraries(oxygenstyleconfig
    PUBLIC
        oxygenstyle
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    PRIVATE
        KF5::I18n
        KF5::KDE4Support
)

set_target_properties(oxygenstyleconfig PROPERTIES VERSION 5.0.0 SOVERSION 5 DEFINE_SYMBOL MAKE_OXYGENSTYLE_LIB )
install(TARGETS oxygenstyleconfig ${INSTALL_TARGETS_DEFAULT_ARGS} )

# Note: no headers installed
