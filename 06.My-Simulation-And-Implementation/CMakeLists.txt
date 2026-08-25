cmake_minimum_required(VERSION 3.16)
project(MonteCarloPi CXX)

set(CMAKE_CXX_STANDARD 17)

# ── ImGui sources (bundled) ──────────────────────────────────────────
set(IMGUI_DIR third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx9.cpp
)

# ── Our sources ──────────────────────────────────────────────────────
set(APP_SOURCES
    ui.cpp
    tests.cpp
    validity-check.cpp
)

# ── Executable ───────────────────────────────────────────────────────
add_executable(monte-carlo-pi
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

# ── Includes ─────────────────────────────────────────────────────────
target_include_directories(monte-carlo-pi PRIVATE
    ${IMGUI_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ── Windows libraries ────────────────────────────────────────────────
target_link_libraries(monte-carlo-pi PRIVATE
    d3d9
    dwmapi
)

# ── GUI subsystem (no console window behind it) ──────────────────────
if(WIN32)
    set_target_properties(monte-carlo-pi PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
