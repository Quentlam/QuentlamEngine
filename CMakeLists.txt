cmake_minimum_required(VERSION 3.10)
project(ParkourGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SRC_FILES "ParkourGame/src/*.cpp" "ParkourGame/src/*.h")

add_executable(ParkourGame ${SRC_FILES})

# Includes
target_include_directories(ParkourGame PRIVATE
    "QuentlamEngine/src"
    "QuentlamEngine/vendor"
    "QuentlamEngine/vendor/spdlog/include"
    "QuentlamEngine/vendor/glm"
    "QuentlamEngine/vendor/imgui"
    "QuentlamEngine/vendor/entt/src"
    "QuentlamEngine/vendor/Glad/include"
    "QuentlamEngine/vendor/Box2D/include"
    "QuentlamEngine/vendor/JoltPhysics"
    "QuentlamEngine/vendor/ImGuizmo"
)

# Definitions
target_compile_definitions(ParkourGame PRIVATE 
    QL_PLATFORM_WINDOWS
    ENTT_INCLUDE_NATVIS
)

# Linking with QuentlamEngine (assuming it's built separately or we can link the static lib)
# For Windows
if(WIN32)
    target_link_libraries(ParkourGame PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug-windows-x86_64/QuentlamEngine/QuentlamEngine.lib"
        opengl32.lib
    )
else()
    # Linux linking
    target_link_libraries(ParkourGame PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug-linux-x86_64/QuentlamEngine/libQuentlamEngine.a"
        GL
    )
endif()
