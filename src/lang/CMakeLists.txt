FILE(GLOB PO_FILES *.po)

# Compiling .po files.
FOREACH (FILE ${PO_FILES})
	GET_FILENAME_COMPONENT(LANGNAME ${FILE} NAME_WE)
	SET(OUT "${LANGNAME}.mo")
	SET(IN "${LANGNAME}.po")

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OUT}
					   COMMAND msgfmt ${IN} -o ${CMAKE_CURRENT_BINARY_DIR}/${OUT}
					   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					   COMMENT "Compiling localization ${LANGNAME}."
					   VERBATIM)
	
	SET (LANGFILE_DEP ${LANGFILE_DEP}
		${CMAKE_CURRENT_BINARY_DIR}/${OUT})

	SET (LANGFILES ${LANGFILES} ${OUT})
ENDFOREACH ()

ADD_CUSTOM_TARGET(languages DEPENDS ${LANGFILE_DEP})


# Embedding .mo files in the executable.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/langfiles.h
                   COMMAND wxinclude ARGS ${CMAKE_CURRENT_BINARY_DIR}/langfiles.h ${LANGFILES}
                   DEPENDS wxinclude languages
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating header for localization files."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_langheader DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/langfiles.h)
