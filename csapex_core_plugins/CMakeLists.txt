cmake_minimum_required(VERSION 2.8.3)
project(csapex_core_plugins)

include (GenerateExportHeader)

find_package(csapex REQUIRED)
find_package(csapex_testing REQUIRED)

list(APPEND CMAKE_PREFIX_PATH ${CSAPEX_CMAKE_MODULE_PATHS})

csapex_package(
	INCLUDE_DIRS include
        LIBRARIES csapex_core_lib csapex_core_lib_qt
        CATKIN_DEPENDS csapex
)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams)

set(CMAKE_AUTOMOC ON)

qt5_add_resources(QT_RESOURCES
    res/csapex_core_resources.qrc
)


###########
## Build ##
###########


include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include
    ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS}
    ${csapex_INCLUDE_DIRS}
    ${catkin_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
)

add_library(csapex_core_lib SHARED
    src/core/composite_message.cpp
    src/core/point_message.cpp
    src/core/timestamp_message.cpp
    src/core/duration_message.cpp
    src/core/interactive_node.cpp
    src/core/collection_node.cpp
    src/core/vector_process_node.cpp
    src/core/key_value_message.cpp
    src/core/map_message.cpp

    ${core_HEADERS}

    ${QT_RESOURCES}
)
generate_export_header(csapex_core_lib)

target_link_libraries(csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core
)

add_library(${PROJECT_NAME}_core SHARED
    src/core/register_core_plugins.cpp
)


target_link_libraries(${PROJECT_NAME}_core csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core Qt5::Gui 
)



add_library(${PROJECT_NAME}_node SHARED
    src/tools/unix_signal_emitter.cpp
    src/tools/string_to_int.cpp
    src/tools/trigger_if_int_equal.cpp
    src/tools/accumulate_messages.cpp
    src/tools/overwrite_frame.cpp
    src/text/replace_string.cpp
    src/tools/overwrite_timestamp.cpp
    src/tools/vector_size.cpp
    src/tools/triggered_latch.cpp
    src/tools/lookup_map_entry.cpp
    src/text/text_compare.cpp
    src/tools/decompose_map_message.cpp
    src/tools/create_map_message.cpp
    src/tools/create_key_value_message.cpp
    src/tools/message_event.cpp
    src/tools/marker_message_detector.cpp
    src/tools/latch.cpp

    src/io/export_cout.cpp
    src/io/export_file.cpp
    src/io/file_importer.cpp
    src/io/import_cin.cpp

    src/math/number_input.cpp
    src/math/number_range_generator.cpp
    src/math/statistics.cpp

    src/text/say_text.cpp
    src/text/text_convert.cpp
    src/text/text_input.cpp
    src/text/generic_text_combiner.cpp

    src/tools/cache.cpp
    src/tools/throttle.cpp
    src/tools/calculate_duration.cpp
    src/tools/composer.cpp
    src/tools/decomposer.cpp
    src/tools/delay.cpp
    src/tools/duration_convertion.cpp
    src/tools/holdable_buffer.cpp
    src/tools/relay.cpp
    src/tools/shutdown.cpp
    src/tools/signal_delay_measurement.cpp
    src/tools/stamp_delay_measurement.cpp
    src/tools/synchronizer.cpp
    src/tools/vector_merge.cpp
    src/tools/vector_generate.cpp
    src/tools/select_vector_component.cpp
    src/tools/count_messages_in_vectors.cpp
    src/tools/count_messages.cpp
    src/tools/mux.cpp
    src/tools/demux.cpp

    src/display/output_display.cpp
    src/display/text_display.cpp
    src/display/signal_light.cpp
)

target_link_libraries(${PROJECT_NAME}_node csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    ${Boost_LIBRARIES}
    Qt5::Core
)

generate_export_header(${PROJECT_NAME}_node)


add_library(csapex_core_lib_qt
    src/tools/parameter_dialog.cpp
    src/display/image_widget.cpp

    include/${PROJECT_NAME}/parameter_dialog.h
    include/${PROJECT_NAME}/image_widget.h
)

target_link_libraries(csapex_core_lib_qt
    #${PROJECT_NAME}_node
    csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core Qt5::Gui Qt5::Widgets
)

add_library(${PROJECT_NAME}_drag_io SHARED
    src/core/file_drag_io.cpp
)


target_link_libraries(${PROJECT_NAME}_drag_io
    csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core Qt5::Gui Qt5::Widgets
)

add_library(${PROJECT_NAME}_qt SHARED
    src/display/output_display_adapter_remote.cpp
    src/display/text_display_adapter.cpp
    src/display/signal_light_adapter.cpp
)


target_link_libraries(${PROJECT_NAME}_qt
    #${PROJECT_NAME}_node
    csapex_core_lib_qt
    csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core Qt5::Gui Qt5::Widgets
)



add_library(${PROJECT_NAME}_direct_qt SHARED
    src/tools/create_map_message_adapter.cpp
    src/display/output_display_adapter.cpp
)


target_link_libraries(${PROJECT_NAME}_direct_qt
    #${PROJECT_NAME}_node
    csapex_core_lib_qt
    csapex_core_lib
    ${catkin_LIBRARIES}
    ${csapex_LIBRARIES}
    Qt5::Core Qt5::Gui Qt5::Widgets
)

#
# TESTS
#

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)

#
# INSTALL
#

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(DIRECTORY snippets
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS csapex_core_lib ${PROJECT_NAME}_core ${PROJECT_NAME}_node ${PROJECT_NAME}_qt
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

if(WIN32)
	file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csapex_core_lib_export.h
		 DESTINATION ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/)
	install(FILES ${PROJECT_BINARY_DIR}/csapex_core_lib_export.h
			DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}/${PROJECT_NAME}/)
else()
	file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csapex_core_lib_export.h
		 DESTINATION ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/)
	install(FILES ${PROJECT_BINARY_DIR}/csapex_core_lib_export.h
			DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
endif()


if(WIN32)
	install(DIRECTORY include/${PROJECT_NAME}/
			DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}/${PROJECT_NAME})
else()
	install(DIRECTORY include/${PROJECT_NAME}/
                        #LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
			DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
endif()

if(NOT ${catkin_FOUND})
        install(FILES cmake/${PROJECT_NAME}-config.cmake
			DESTINATION ${CSAPEX_MODULE_DESTINATION})
endif()
