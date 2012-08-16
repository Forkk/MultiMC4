SET(TOOLBAR_ICONS
NewInstIcon.png
ImportCPIcon.png
AboutIcon.png
CheckUpdateIcon.png
HelpIcon.png
RefreshInstIcon.png
SettingsIcon.png
ViewFolderIcon.png
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h ${TOOLBAR_ICONS}
                   DEPENDS wxinclude
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tbaricons
                   COMMENT "Generating toolbar icon resources."
                   VERBATIM)

SET(INSTANCE_ICONS
brick.png
diamond.png
dirt.png
enderman.png
gold.png
grass.png
infinity.png
iron.png
planks.png
stone.png
tnt.png
creeper.png
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/insticons.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/insticons.h ${INSTANCE_ICONS}
                   DEPENDS wxinclude
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/insticons
                   COMMENT "Generating instance icon resources."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_headers DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/insticons.h ${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h)
