cmake_minimum_required(VERSION 3.10)
project(wangLandauNew LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ──────────────────────────────────────────────────
# Опция отладочных сообщений
# ──────────────────────────────────────────────────
# По умолчанию: ON  для Debug-сборок,
#               OFF для остальных (Release, RelWithDebInfo, MinSizeRel).
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_DEBUG_MESSAGES_DEFAULT ON)
else()
    set(ENABLE_DEBUG_MESSAGES_DEFAULT OFF)
endif()

option(ENABLE_DEBUG_MESSAGES
    "Включить отладочные сообщения (макрос DBG())"
    ${ENABLE_DEBUG_MESSAGES_DEFAULT}
)

if(ENABLE_DEBUG_MESSAGES)
    add_definitions(-DENABLE_DEBUG_MESSAGES)
    message(STATUS "Debug messages: ENABLED")
else()
    message(STATUS "Debug messages: DISABLED")
endif()


# ──────────────────────────────────────────────────
# Boost (serialization)
# ──────────────────────────────────────────────────
# find_package(Boost REQUIRED COMPONENTS serialization)
# if(NOT Boost_FOUND)
#     message(FATAL_ERROR "Boost not found. Install libboost-serialization-dev.")
# endif()

# ──────────────────────────────────────────────────
# Исполняемый файл
# ──────────────────────────────────────────────────
set(SOURCES
    main.cpp
    wlNew.cpp
    MagneticSystem.cpp
    dos2.cpp
    misc.cpp
    gapmanager.cpp
    debug_msg.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# target_link_libraries(${PROJECT_NAME} PRIVATE
#     ${Boost_LIBRARIES}
# )