cmake_minimum_required(VERSION 3.12...3.31)
project(auth_service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
    userver
    COMPONENTS core chaotic sqlite #
    QUIET
)
if(NOT userver_FOUND)
    # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
    download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

# jwt-cpp — header-only JWT library
find_package(jwt-cpp QUIET)
if(NOT jwt-cpp_FOUND)
    message("Loading JWT-CPP")
    include(cmake/get_cpm.cmake)
    CPMAddPackage(
        NAME jwt-cpp
        GITHUB_REPOSITORY Thalhammer/jwt-cpp
        VERSION 0.7.2
        OPTIONS
            "JWT_BUILD_EXAMPLES OFF"
            "JWT_BUILD_TESTS OFF"
            "JWT_CMAKE_FILES_INSTALL ON"
    )
endif()

# Common sources

# Chaotic codegen
file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)
userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    GENERATE_SERIALIZERS
    LAYOUT "/components/schemas/([^/]*)/=auth::dto::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/utils/jwt_utils.cpp
    src/storage/credential_storage.cpp
    src/usecase/auth_usecase.cpp
    src/handlers/register_handler.cpp
    src/handlers/login_handler.cpp
    src/handlers/validate_handler.cpp
)
target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core #
    PUBLIC userver::sqlite #
    PUBLIC jwt-cpp::jwt-cpp #
    PUBLIC ${PROJECT_NAME}-chgen #
)
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Functional testing
option(ENABLE_TESTS "Build and register tests" ON)
if(ENABLE_TESTS)
    userver_testsuite_add_simple()
endif()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)
install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)
