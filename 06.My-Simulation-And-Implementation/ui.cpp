/*
 * ui.cpp  —  Monte Carlo PI Simulator
 * -------------------------------------
 * Dear ImGui + DirectX 9 + Win32  (no external installs — ImGui is
 * bundled in third_party/imgui/, same setup as Tiny-Shell).
 *
 * Layout (three side-by-side panels inside one window):
 *
 *  ┌─────────────────────────────────────────────────────────────────┐
 *  │                  Monte Carlo  PI  Simulator                     │
 *  ├──────────────────┬───────────────────────┬───────────────────── │
 *  │   MENU           │   SCATTER PLOT        │   RESULTS / TABLE   │
 *  │                  │   (ImDrawList canvas) │                     │
 *  │  1. Experiment   │   green  = inside     │   Trials            │
 *  │  2. 10 trials    │   red    = outside    │   In circle         │
 *  │  ...             │   cyan   = arc        │   Estimated PI      │
 *  │  9. Exit         │                       │   Error             │
 *  └──────────────────┴───────────────────────┴─────────────────────┘
 */

/* ── Windows / DX9 ─────────────────────────────────────────────────*/
#include <windows.h>
#include <d3d9.h>

/* ── ImGui ──────────────────────────────────────────────────────── */
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_win32.h"
#include "third_party/imgui/imgui_impl_dx9.h"

/* ── Our simulation code ─────────────────────────────────────────── */
#include "tests.h"

/* ── std ─────────────────────────────────────────────────────────── */
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>

/* ════════════════════════════════════════════════════════════════════
 *  GLOBALS
 * ════════════════════════════════════════════════════════════════════ */

static LPDIRECT3D9           g_d3d        = nullptr;
static LPDIRECT3DDEVICE9     g_d3d_device = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp      = {};
static HWND                  g_hwnd       = nullptr;
static WNDCLASSEXW           g_wc         = {};
static bool                  g_running    = true;

// Simulator instance — owns the RNG, replaces bare g_gen
static Simulator g_sim;

// App state
enum class AppState {
    MENU,           // waiting for a selection
    RUNNING,        // simulation in progress (shown as spinner)
    RESULT,         // single run result displayed
    EXPERIMENT      // experiment table displayed
};
static AppState g_state = AppState::MENU;

// Current result / scatter
static RunResult              g_result;
static std::vector<SimResult> g_experimentResults;

// Menu
static int  g_menuHighlight  = 1;          // 1-9
static bool g_customOpen     = false;       // custom-trials input popup
static char g_customBuf[32]  = "10000";

// Status bar message
static std::string g_statusMsg = "Use arrow keys or click to navigate  |  Enter to select";

/* ════════════════════════════════════════════════════════════════════
 *  DX9 HELPERS  (identical pattern to Tiny-Shell)
 * ════════════════════════════════════════════════════════════════════ */

static bool CreateDeviceD3D(HWND hWnd) {
    g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_d3d) return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed               = TRUE;
    g_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = g_d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &g_d3dpp, &g_d3d_device);
    return SUCCEEDED(hr);
}

static void CleanupDeviceD3D() {
    if (g_d3d_device) { g_d3d_device->Release(); g_d3d_device = nullptr; }
    if (g_d3d)        { g_d3d->Release();         g_d3d        = nullptr; }
}

static void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_d3d_device->Reset(&g_d3dpp);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
        case WM_SIZE:
            if (g_d3d_device && wParam != SIZE_MINIMIZED) {
                g_d3dpp.BackBufferWidth  = LOWORD(lParam);
                g_d3dpp.BackBufferHeight = HIWORD(lParam);
                ResetDevice();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

/* ════════════════════════════════════════════════════════════════════
 *  SIMULATION DISPATCH
 * ════════════════════════════════════════════════════════════════════ */

static void runChoice(int choice) {
    g_state = AppState::RUNNING;

    if (choice >= 2 && choice <= 7) {
        static const int sizes[] = { 0, 0, 10, 100, 1000, 10000, 100000, 1000000 };
        g_result = g_sim.run(sizes[choice]);
        g_state  = AppState::RESULT;
        g_statusMsg = "Press R to run again  |  Esc to go back";
    }
    else if (choice == 8) {
        int n = atoi(g_customBuf);
        if (n <= 0) n = 1000;
        g_result = g_sim.run(n);
        g_state  = AppState::RESULT;
        g_statusMsg = "Press R to run again  |  Esc to go back";
    }
    else if (choice == 1) {
        g_experimentResults.clear();
        RunResult (*fns[])(Simulator&) = {
            [](Simulator &s){ return s.run10();   },
            [](Simulator &s){ return s.run100();  },
            [](Simulator &s){ return s.run1K();   },
            [](Simulator &s){ return s.run10K();  },
            [](Simulator &s){ return s.run100K(); },
            [](Simulator &s){ return s.run1M();   },
        };
        for (auto fn : fns) {
            RunResult r = fn(g_sim);
            g_experimentResults.push_back(r.stats);
            g_result = r;
        }
        g_state = AppState::EXPERIMENT;
        g_statusMsg = "Experiment complete  |  Esc to go back";
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  SCATTER PLOT  (drawn with ImDrawList into a child window)
 * ════════════════════════════════════════════════════════════════════ */

static void DrawScatterPlot(ImVec2 size) {
    ImGui::BeginChild("##scatter", size, true);

    ImDrawList *dl    = ImGui::GetWindowDrawList();
    ImVec2      p0    = ImGui::GetCursorScreenPos();
    float       W     = size.x  - 16.0f;
    float       H     = size.y  - 16.0f;

    // Background
    dl->AddRectFilled(p0, ImVec2(p0.x + W, p0.y + H),
                      IM_COL32(12, 12, 20, 255));
    dl->AddRect(p0, ImVec2(p0.x + W, p0.y + H),
                IM_COL32(60, 180, 200, 180));

    if (g_state == AppState::MENU) {
        // placeholder text
        dl->AddText(ImVec2(p0.x + W * 0.3f, p0.y + H * 0.45f),
                    IM_COL32(80, 80, 100, 255),
                    "Run a simulation to see the scatter plot");
        ImGui::EndChild();
        return;
    }

    // Lambda: map [-1,1] → canvas pixel
    auto toX = [&](double v) { return p0.x + (float)((v + 1.0) * 0.5 * W); };
    auto toY = [&](double v) { return p0.y + H - (float)((v + 1.0) * 0.5 * H); };

    // Outside points — red dots
    for (auto &pt : g_result.scatter.outside)
        dl->AddCircleFilled(ImVec2(toX(pt.first), toY(pt.second)),
                            1.5f, IM_COL32(220, 60, 60, 200));

    // Inside points — green dots
    for (auto &pt : g_result.scatter.inside)
        dl->AddCircleFilled(ImVec2(toX(pt.first), toY(pt.second)),
                            1.5f, IM_COL32(60, 210, 100, 200));

    // Quarter-circle arc — cyan
    const int ARC_SEGS = 120;
    for (int i = 0; i < ARC_SEGS; i++) {
        double a0 = (M_PI * 0.5) * i       / ARC_SEGS;
        double a1 = (M_PI * 0.5) * (i + 1) / ARC_SEGS;
        // arc is in [0,1] normalised, but our coords are [-1,1]
        // arc radius = 1 in normalised space, map using toX/toY
        float x0 = toX(std::cos(a0));
        float y0 = toY(std::sin(a0));
        float x1 = toX(std::cos(a1));
        float y1 = toY(std::sin(a1));
        dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1),
                    IM_COL32(0, 220, 220, 255), 1.5f);
    }

    // Legend
    float lx = p0.x + 8.0f;
    float ly = p0.y + H - 36.0f;
    dl->AddCircleFilled(ImVec2(lx + 5,  ly),      4.f, IM_COL32(60,  210, 100, 255));
    dl->AddText(ImVec2(lx + 12, ly - 6),  IM_COL32(200,200,200,255), "Inside");
    dl->AddCircleFilled(ImVec2(lx + 65, ly),      4.f, IM_COL32(220, 60,  60,  255));
    dl->AddText(ImVec2(lx + 72, ly - 6),  IM_COL32(200,200,200,255), "Outside");
    dl->AddLine(ImVec2(lx + 130, ly - 4), ImVec2(lx + 142, ly + 4),
                IM_COL32(0, 220, 220, 255), 2.f);
    dl->AddText(ImVec2(lx + 146, ly - 6), IM_COL32(200,200,200,255), "Arc");

    ImGui::EndChild();
}

/* ════════════════════════════════════════════════════════════════════
 *  RESULTS PANEL
 * ════════════════════════════════════════════════════════════════════ */

static void DrawResultsPanel(ImVec2 size) {
    ImGui::BeginChild("##results", size, true);

    if (g_state == AppState::MENU || g_state == AppState::RUNNING) {
        ImGui::TextDisabled("No results yet.");
        ImGui::EndChild();
        return;
    }

    // Helper: format double
    auto fmt = [](double v, int prec) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(prec) << v;
        return ss.str();
    };

    if (g_state == AppState::RESULT) {
        const SimResult &r = g_result.stats;

        ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Simulation Results");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Trials              : %d", r.trials);
        ImGui::Text("Points inside circle: %d", r.inCircle);
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.1f, 1.0f),
                           "Estimated PI        : %s",
                           fmt(r.estimatedPi, 9).c_str());
        ImGui::Text("Actual PI           : 3.141592653");

        ImGui::Spacing();

        // Colour the error green/yellow/red based on magnitude
        ImVec4 errCol = r.error < 0.001
                        ? ImVec4(0.2f,0.9f,0.3f,1.0f)
                        : r.error < 0.01
                          ? ImVec4(1.0f,0.85f,0.1f,1.0f)
                          : ImVec4(1.0f,0.3f,0.3f,1.0f);

        ImGui::TextColored(errCol,
                           "Absolute Error      : %s",
                           fmt(r.error, 9).c_str());
        ImGui::TextColored(ImVec4(0.4f,0.8f,0.9f,1.0f),
                           "Theoretical Bound   : %s",
                           fmt(r.theoreticalBound, 6).c_str());

        ImGui::Spacing();
        ImGui::Separator();

        // Simple error bar
        ImGui::Spacing();
        ImGui::Text("Error relative to bound:");
        float ratio = (r.theoreticalBound > 0)
                      ? (float)(r.error / r.theoreticalBound)
                      : 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                              errCol);
        ImGui::ProgressBar(ratio, ImVec2(-1, 10), "");
        ImGui::PopStyleColor();
    }
    else if (g_state == AppState::EXPERIMENT) {
        ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Experiment Mode — All Sizes");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.9f,0.9f,0.4f,1.0f),
                           "%-12s %-16s %-12s %s",
                           "Trials", "Estimated PI", "Error", "Bound");
        ImGui::Separator();

        for (const auto &r : g_experimentResults) {
            ImVec4 c = r.error < 0.001
                       ? ImVec4(0.2f,0.9f,0.3f,1.0f)
                       : r.error < 0.01
                         ? ImVec4(1.0f,0.85f,0.1f,1.0f)
                         : ImVec4(1.0f,0.3f,0.3f,1.0f);

            ImGui::TextColored(c,
                "%-12d %-16s %-12s %s",
                r.trials,
                fmt(r.estimatedPi, 9).c_str(),
                fmt(r.error, 9).c_str(),
                fmt(r.theoreticalBound, 6).c_str());
        }
    }

    ImGui::EndChild();
}

/* ════════════════════════════════════════════════════════════════════
 *  MENU PANEL
 * ════════════════════════════════════════════════════════════════════ */

static void DrawMenuPanel(ImVec2 size) {
    ImGui::BeginChild("##menu", size, true);

    ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Simulation Size");
    ImGui::Separator();
    ImGui::Spacing();

    struct MenuItem { int id; const char *label; };
    static const MenuItem items[] = {
        {1, "Experiment (all sizes)"},
        {2, "10 Trials"},
        {3, "100 Trials"},
        {4, "1,000 Trials"},
        {5, "10,000 Trials"},
        {6, "100,000 Trials"},
        {7, "1,000,000 Trials"},
        {8, "Custom Trials"},
        {9, "Exit"},
    };

    for (auto &item : items) {
        bool selected = (g_menuHighlight == item.id);

        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.05f, 0.45f, 0.55f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.15f, 0.15f, 0.20f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.10f, 0.55f, 0.65f, 1.0f));

        if (ImGui::Button(item.label, ImVec2(size.x - 18, 28))) {
            g_menuHighlight = item.id;
            if (item.id == 9) {
                g_running = false;
            } else if (item.id == 8) {
                g_customOpen = true;
            } else {
                runChoice(item.id);
            }
        }

        ImGui::PopStyleColor(2);
        ImGui::Spacing();
    }

    ImGui::EndChild();
}

/* ════════════════════════════════════════════════════════════════════
 *  CUSTOM TRIALS POPUP
 * ════════════════════════════════════════════════════════════════════ */

static void DrawCustomPopup() {
    if (!g_customOpen) return;

    ImGui::OpenPopup("Custom Trials");
    ImGui::SetNextWindowSize(ImVec2(300, 110), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Custom Trials", nullptr,
                               ImGuiWindowFlags_NoResize)) {
        ImGui::Spacing();
        ImGui::Text("Enter number of trials:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##custominput", g_customBuf, sizeof(g_customBuf),
                         ImGuiInputTextFlags_CharsDecimal);
        ImGui::Spacing();

        if (ImGui::Button("Run", ImVec2(120, 0))) {
            g_customOpen = false;
            runChoice(8);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            g_customOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  KEYBOARD HANDLING
 * ════════════════════════════════════════════════════════════════════ */

static void HandleKeyboard() {
    ImGuiIO &io = ImGui::GetIO();

    // Arrow navigation (menu state only)
    if (g_state == AppState::MENU || g_state == AppState::RESULT ||
        g_state == AppState::EXPERIMENT)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            if (g_menuHighlight > 1) g_menuHighlight--;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            if (g_menuHighlight < 9) g_menuHighlight++;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
            ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        {
            if (g_menuHighlight == 9) g_running = false;
            else if (g_menuHighlight == 8) g_customOpen = true;
            else runChoice(g_menuHighlight);
        }
    }

    // R = run again (result/experiment state)
    if ((g_state == AppState::RESULT || g_state == AppState::EXPERIMENT) &&
        ImGui::IsKeyPressed(ImGuiKey_R))
    {
        runChoice(g_menuHighlight);
    }

    // Esc = back to menu
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        if (g_state != AppState::MENU) {
            g_state     = AppState::MENU;
            g_statusMsg = "Use arrow keys or click to navigate  |  Enter to select";
        }
    }

    (void)io;
}

/* ════════════════════════════════════════════════════════════════════
 *  MAIN RENDER FRAME
 * ════════════════════════════════════════════════════════════════════ */

static void RenderFrame() {
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    float W = (float)(rc.right  - rc.left);
    float H = (float)(rc.bottom - rc.top);

    ImGui::SetNextWindowPos (ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::Begin("##root", nullptr,
                 ImGuiWindowFlags_NoTitleBar  |
                 ImGuiWindowFlags_NoResize    |
                 ImGuiWindowFlags_NoMove      |
                 ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    HandleKeyboard();

    // ── Title bar ──────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImVec4(0.04f, 0.22f, 0.28f, 1.0f));
    ImGui::BeginChild("##titlebar", ImVec2(0, 36), false,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(12, 9));
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f),
                       "Monte Carlo  PI  Simulator");
    ImGui::SameLine(0, 30);
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.85f, 1.0f),
                       "Arrow keys / click to navigate   |   Enter to run   |   R to repeat   |   Esc to back");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // ── Three-column layout ────────────────────────────────────────
    float contentH  = H - 36.0f - 28.0f;   // titlebar + statusbar
    float menuW     = 220.0f;
    float rightW    = W - menuW - 8.0f;     // 4px gap each side
    float scatterW  = rightW * 0.55f;
    float resultsW  = rightW * 0.45f - 8.0f;

    ImGui::SetCursorPosY(36.0f);

    // Menu
    ImGui::SetCursorPosX(4.0f);
    DrawMenuPanel(ImVec2(menuW, contentH));

    // Scatter
    ImGui::SameLine(0, 4);
    DrawScatterPlot(ImVec2(scatterW, contentH));

    // Results
    ImGui::SameLine(0, 4);
    DrawResultsPanel(ImVec2(resultsW, contentH));

    // Custom trials popup
    DrawCustomPopup();

    // ── Status bar ─────────────────────────────────────────────────
    ImGui::SetCursorPosY(H - 26.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImVec4(0.04f, 0.14f, 0.18f, 1.0f));
    ImGui::BeginChild("##statusbar", ImVec2(0, 24), false,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(8, 4));
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.85f, 1.0f),
                       "%s", g_statusMsg.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}

/* ════════════════════════════════════════════════════════════════════
 *  WIN32 ENTRY POINT
 * ════════════════════════════════════════════════════════════════════ */

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Register window class
    g_wc = { sizeof(g_wc), CS_CLASSDC, WndProc, 0L, 0L,
             GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
             L"MonteCarloPi", nullptr };
    RegisterClassExW(&g_wc);

    // Create window — 1200 × 680
    g_hwnd = CreateWindowW(
        L"MonteCarloPi", L"Monte Carlo  PI  Simulator",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1200, 680,
        nullptr, nullptr, g_wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Tweak style to match Tiny-Shell dark feel
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding     = 0.0f;
    style.ChildRounding      = 4.0f;
    style.FrameRounding      = 3.0f;
    style.ScrollbarRounding  = 3.0f;
    style.ItemSpacing        = ImVec2(6, 5);
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.06f, 0.06f, 0.09f, 1.0f);
    style.Colors[ImGuiCol_ChildBg]        = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.10f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]  = ImVec4(0.04f, 0.20f, 0.26f, 1.0f);
    style.Colors[ImGuiCol_Border]         = ImVec4(0.20f, 0.50f, 0.60f, 0.5f);
    style.Colors[ImGuiCol_PopupBg]        = ImVec4(0.08f, 0.10f, 0.14f, 1.0f);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(g_d3d_device);

    // Main loop
    MSG msg;
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) g_running = false;
        }
        if (!g_running) break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderFrame();

        // Render
        g_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_d3d_device->Clear(0, nullptr,
                            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                            D3DCOLOR_RGBA(12, 12, 18, 255), 1.0f, 0);

        if (g_d3d_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_d3d_device->EndScene();
        }
        g_d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
    }

    // Shutdown
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
    return 0;
}
