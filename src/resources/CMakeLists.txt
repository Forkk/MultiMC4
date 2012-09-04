# prefix and suffix each element of list by ${prefix}elemnt${suffix}
macro(PREFIXSUFFIX list_name prefix suffix) 
	# create empty list - necessary?
	SET(${list_name}_TMP)
	# prefix and suffix elements
	foreach(l ${${list_name}})
		list(APPEND ${list_name}_TMP ${prefix}${l}${suffix} )
	endforeach()
	# replace list by tmp list
	SET(${list_name} "${${list_name}_TMP}")
	UNSET(${list_name}_TMP)
endmacro(PREFIXSUFFIX)

SET(TOOLBAR_ICONS
NewInstIcon.png
ImportCPIcon.png
AboutIcon.png
CheckUpdateIcon.png
HelpIcon.png
RefreshInstIcon.png
SettingsIcon.png
ViewFolderIcon.png
CentralModsFolder.png
)

SET(TOOLBAR_ICONS_DEP ${INSTANCE_ICONS})
PREFIXSUFFIX(TOOLBAR_ICONS_DEP "tbaricons/" "")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h ${TOOLBAR_ICONS}
                   DEPENDS wxinclude ${TOOLBAR_ICONS_DEP}
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
squarecreeper.png
skeleton.png
enderpearl.png
)
SET(INSTANCE_ICONS_DEP ${INSTANCE_ICONS})
PREFIXSUFFIX(INSTANCE_ICONS_DEP "insticons/" "")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/insticons.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/insticons.h ${INSTANCE_ICONS}
                   DEPENDS wxinclude ${INSTANCE_ICONS_DEP}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/insticons
                   COMMENT "Generating instance icon resources."
                   VERBATIM)

SET(MAIN_ICON
MultiMC16.png
MultiMC32.png
MultiMC48.png
MultiMC64.png
MultiMC128.png
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/windowicon.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/windowicon.h ${MAIN_ICON}
                   DEPENDS wxinclude ${MAIN_ICON}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   COMMENT "Generating application icon resources."
                   VERBATIM)

SET(CONSOLE_ICON
console.png
console_error.png
console24.png
console_error24.png
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/consoleicon.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/consoleicon.h ${CONSOLE_ICON}
                   DEPENDS wxinclude ${CONSOLE_ICON}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   COMMENT "Generating console icon resources."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_headers
DEPENDS
${CMAKE_CURRENT_BINARY_DIR}/insticons.h
${CMAKE_CURRENT_BINARY_DIR}/toolbaricons.h
${CMAKE_CURRENT_BINARY_DIR}/windowicon.h
${CMAKE_CURRENT_BINARY_DIR}/consoleicon.h
)