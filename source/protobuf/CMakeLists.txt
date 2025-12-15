# *****************************************************************************
# Project protobuf
# *****************************************************************************
project(protobuf)

find_package(Protobuf REQUIRED MODULE)
find_package(Threads)

include_directories(${PROTOBUF_INCLUDE_DIRS})

file(GLOB ProtoFiles
	"${CMAKE_CURRENT_SOURCE_DIR}/**/*.proto"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.proto"
)

if (MSVC AND BUILD_STATIC_LIBRARY)
	add_library(${PROJECT_NAME} STATIC ${ProtoFiles})
else()
	add_library(${PROJECT_NAME} ${ProtoFiles})
endif()

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		protobuf::libprotobuf
)
# FIX: Manually set location if missing (System mismatch workaround)
if(TARGET protobuf::protoc)
    get_target_property(LOC protobuf::protoc IMPORTED_LOCATION)
    get_target_property(LOC_REL protobuf::protoc IMPORTED_LOCATION_RELEASE)
    if(NOT LOC AND NOT LOC_REL)
        message(STATUS "Manual Fix: Setting IMPORTED_LOCATION for protobuf::protoc to ${Protobuf_PROTOC_EXECUTABLE}")
        set_target_properties(protobuf::protoc PROPERTIES IMPORTED_LOCATION ${Protobuf_PROTOC_EXECUTABLE})
    endif()
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC AND BUILD_STATIC_LIBRARY)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

protobuf_generate(TARGET ${PROJECT_NAME} LANGUAGE cpp)
