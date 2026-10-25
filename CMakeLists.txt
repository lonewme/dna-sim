cmake_minimum_required(VERSION 3.20)
project(DNASimulation LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CMake 4.x rejects very old libraries unless we allow this
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

# --- GLFW (window + input) ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

# --- Dear ImGui (UI widgets) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.0
)

# --- doctest (unit tests) ---
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
)
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw imgui doctest)

find_package(OpenGL REQUIRED)

# ImGui is not a CMake project, so we build it ourselves as a library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# --- Core library: all DNA logic, no GUI code ---
add_library(dna_core STATIC
    core/src/Nucleotide.cpp
    core/src/DnaSequence.cpp
    core/src/FixedLengthCodec.cpp
    core/src/Mutator.cpp
    core/src/Simulation.cpp
    core/src/Crc.cpp
    core/src/ChunkedCodec.cpp
    core/src/StrandSimulation.cpp
)
target_include_directories(dna_core PUBLIC core/include)

# --- Tests ---
enable_testing()
add_executable(dna_tests
    tests/test_codec.cpp
    tests/test_mutator.cpp
    tests/test_simulation.cpp
    tests/test_chunked.cpp
    tests/test_strand_simulation.cpp
)
target_link_libraries(dna_tests PRIVATE dna_core doctest::doctest)
add_test(NAME dna_tests COMMAND dna_tests)

# --- The application: GUI on top of the core ---
add_executable(dna_sim app/main.cpp app/SimulationPanel.cpp app/StrandPanel.cpp)
target_link_libraries(dna_sim PRIVATE dna_core imgui)

# MinGW: bundle the runtime so the .exe runs anywhere
if(MINGW)
    target_link_options(dna_sim PRIVATE -static)
endif()