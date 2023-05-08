cmake_minimum_required (VERSION 2.6)

project (uvm)

SET(SRC_LIST UvmDev.cpp Uvm.cpp)

add_compile_options(-fPIC -Wl,--no-as-needed)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common/include)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})

if (${UVM_LIB_MODE} MATCHES "static")
    add_library(uvm STATIC ${SRC_LIST})
else()
    add_library(uvm SHARED ${SRC_LIST})
endif()
TARGET_LINK_LIBRARIES(uvm pthread)

set_target_properties(uvm PROPERTIES SOVERSION 0 VERSION 0.0.0)

install(TARGETS uvm LIBRARY DESTINATION ${LIB_DIR})
install(DIRECTORY include/ DESTINATION ${INCLUDE_DIR})
