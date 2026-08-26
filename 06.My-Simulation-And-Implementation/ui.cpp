/*
 * ui.cpp  —  Monte Carlo Simulator
 * ----------------------------------
 * Dear ImGui + DirectX 9 + Win32
 *
 * Two tabs:
 *   Tab 1 — PI Estimator   (scatter plot, experiment table)
 *   Tab 2 — Random Walk    (path canvas, stats panel, per-particle list)
 *
 * Layout per tab:
 *   ┌──────────┬───────────────────────┬──────────────────┐
 *   │  MENU    │   CANVAS              │   RESULTS        │
 *   └──────────┴───────────────────────┴──────────────────┘
 */

/* ── Windows / DX9 ─────────────────────────────────────────────────*/
#include <windows.h>
#include <d3d9.h>

/* ── ImGui ──────────────────────────────────────────────────────── */
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_win32.h"
#include "third_party/imgui/imgui_impl_dx9.h"

/* ── Simulation headers ──────────────────────────────────────────── */
#include "tests.h"
#include "../08.My-Random-Walk/walk.h"

/* ── std ─────────────────────────────────────────────────────────── */
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

/* ════════════════════════════════════════════════════════════════════
 *  DX9 / WINDOW GLOBALS
 * ════════════════════════════════════════════════════════════════════ */

static LPDIRECT3D9           g_d3d        = nullptr;
static LPDIRECT3DDEVICE9     g_d3d_device = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp      = {};
static HWND                  g_hwnd       = nullptr;
static WNDCLASSEXW           g_wc         = {};
static bool                  g_running    = true;

/* ════════════════════════════════════════════════════════════════════
 *  TAB 1 — PI ESTIMATOR STATE
 * ════════════════════════════════════════════════════════════════════ */

enum class PiState { MENU, RESULT, EXPERIMENT };

static Simulator              g_piSim;
static PiState                g_piState       = PiState::MENU;
static RunResult              g_piResult;
static std::vector<SimResult> g_piExpResults;
static int                    g_piHighlight   = 1;
static bool                   g_piCustomOpen  = false;
static char                   g_piCustomBuf[32] = "10000";
static std::string            g_piStatus      = "Click a size or use arrow keys + Enter";

/* ════════════════════════════════════════════════════════════════════
 *  TAB 2 — RANDOM WALK STATE
 * ════════════════════════════════════════════════════════════════════ */

enum class WalkState { MENU, RESULT };

static WalkSimulation   g_walkSim;
static WalkState        g_walkState         = WalkState::MENU;
static SimulationStats  g_walkStats;
static int              g_walkHighlight     = 1;   // particle menu (1-6)
static int              g_walkStepHL        = 1;   // step menu (1-5)
static bool             g_walkParticleOpen  = false;
static bool             g_walkStepOpen      = false;
static char             g_walkParticleBuf[32] = "10";
static char             g_walkStepBuf[32]     = "1000";
static int              g_walkParticles     = 1;
static int              g_walkSteps         = 1000;
static int              g_walkViewParticle  = 0;   // which particle path to draw
static std::string      g_walkStatus        = "Choose particle count, then step count";

/* ════════════════════════════════════════════════════════════════════
 *  DX9 HELPERS
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
    HRESULT hr = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_SIZE:
            if (g_d3d_device && wParam != SIZE_MINIMIZED) {
                g_d3dpp.BackBufferWidth  = LOWORD(lParam);
                g_d3dpp.BackBufferHeight = HIWORD(lParam);
                ResetDevice();
            }
            return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

/* ════════════════════════════════════════════════════════════════════
 *  HELPERS
 * ════════════════════════════════════════════════════════════════════ */

static std::string fmtD(double v, int prec) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(prec) << v;
    return ss.str();
}

// Draws a styled button, highlights it if selected
static bool MenuButton(const char *label, float width, bool selected) {
    if (selected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.45f, 0.55f, 1.0f));
    else
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.55f, 0.65f, 1.0f));
    bool clicked = ImGui::Button(label, ImVec2(width, 28));
    ImGui::PopStyleColor(2);
    ImGui::Spacing();
    return clicked;
}

/* ════════════════════════════════════════════════════════════════════
 *  TAB 1 — PI ESTIMATOR
 * ════════════════════════════════════════════════════════════════════ */

static void PiRunChoice(int choice) {
    static const int sizes[] = { 0, 0, 10, 100, 1000, 10000, 100000, 1000000 };

    if (choice >= 2 && choice <= 7) {
        g_piResult  = g_piSim.run(sizes[choice]);
        g_piState   = PiState::RESULT;
        g_piStatus  = "Press R to run again  |  Esc to go back";
    } else if (choice == 8) {
        int n = atoi(g_piCustomBuf);
        if (n <= 0) n = 1000;
        g_piResult  = g_piSim.run(n);
        g_piState   = PiState::RESULT;
        g_piStatus  = "Press R to run again  |  Esc to go back";
    } else if (choice == 1) {
        g_piExpResults.clear();
        RunResult (*fns[])(Simulator&) = {
            [](Simulator &s){ return s.run10();   },
            [](Simulator &s){ return s.run100();  },
            [](Simulator &s){ return s.run1K();   },
            [](Simulator &s){ return s.run10K();  },
            [](Simulator &s){ return s.run100K(); },
            [](Simulator &s){ return s.run1M();   },
        };
        for (auto fn : fns) {
            RunResult r = fn(g_piSim);
            g_piExpResults.push_back(r.stats);
            g_piResult = r;
        }
        g_piState  = PiState::EXPERIMENT;
        g_piStatus = "Experiment complete  |  Esc to go back";
    }
}

static void DrawPiMenu(float w, float h) {
    ImGui::BeginChild("##pimenu", ImVec2(w, h), true);
    ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "PI Estimator");
    ImGui::Separator(); ImGui::Spacing();

    struct Item { int id; const char *label; };
    static const Item items[] = {
        {1,"Experiment (all sizes)"},{2,"10 Trials"},{3,"100 Trials"},
        {4,"1,000 Trials"},{5,"10,000 Trials"},{6,"100,000 Trials"},
        {7,"1,000,000 Trials"},{8,"Custom Trials"},
    };
    for (auto &it : items) {
        if (MenuButton(it.label, w - 18, g_piHighlight == it.id)) {
            g_piHighlight = it.id;
            if (it.id == 8) g_piCustomOpen = true;
            else            PiRunChoice(it.id);
        }
    }
    ImGui::EndChild();
}

static void DrawPiCanvas(float w, float h) {
    ImGui::BeginChild("##picanvas", ImVec2(w, h), true);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    float  W  = w - 16.0f, H = h - 16.0f;

    dl->AddRectFilled(p0, ImVec2(p0.x+W, p0.y+H), IM_COL32(12,12,20,255));
    dl->AddRect      (p0, ImVec2(p0.x+W, p0.y+H), IM_COL32(60,180,200,180));

    if (g_piState == PiState::MENU) {
        dl->AddText(ImVec2(p0.x + W*0.25f, p0.y + H*0.45f),
                    IM_COL32(80,80,100,255), "Run a simulation to see the scatter plot");
        ImGui::EndChild(); return;
    }

    auto toX = [&](double v){ return p0.x + (float)((v+1.0)*0.5*W); };
    auto toY = [&](double v){ return p0.y + H - (float)((v+1.0)*0.5*H); };

    for (auto &pt : g_piResult.scatter.outside)
        dl->AddCircleFilled(ImVec2(toX(pt.first), toY(pt.second)), 1.5f, IM_COL32(220,60,60,200));
    for (auto &pt : g_piResult.scatter.inside)
        dl->AddCircleFilled(ImVec2(toX(pt.first), toY(pt.second)), 1.5f, IM_COL32(60,210,100,200));

    // Quarter-circle arc
    for (int i = 0; i < 120; i++) {
        double a0 = (M_PI*0.5)*i/120, a1 = (M_PI*0.5)*(i+1)/120;
        dl->AddLine(ImVec2(toX(cos(a0)), toY(sin(a0))),
                    ImVec2(toX(cos(a1)), toY(sin(a1))),
                    IM_COL32(0,220,220,255), 1.5f);
    }

    // Legend
    float lx = p0.x+8, ly = p0.y+H-36;
    dl->AddCircleFilled(ImVec2(lx+5, ly),   4.f, IM_COL32(60,210,100,255));
    dl->AddText(ImVec2(lx+12, ly-6), IM_COL32(200,200,200,255), "Inside");
    dl->AddCircleFilled(ImVec2(lx+65, ly),  4.f, IM_COL32(220,60,60,255));
    dl->AddText(ImVec2(lx+72, ly-6), IM_COL32(200,200,200,255), "Outside");
    dl->AddLine(ImVec2(lx+130,ly-4), ImVec2(lx+142,ly+4), IM_COL32(0,220,220,255), 2.f);
    dl->AddText(ImVec2(lx+146, ly-6), IM_COL32(200,200,200,255), "Arc");

    ImGui::EndChild();
}

static void DrawPiResults(float w, float h) {
    ImGui::BeginChild("##piresults", ImVec2(w, h), true);

    if (g_piState == PiState::MENU) {
        ImGui::TextDisabled("No results yet."); ImGui::EndChild(); return;
    }

    if (g_piState == PiState::RESULT) {
        const SimResult &r = g_piResult.stats;
        ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "PI Simulation Results");
        ImGui::Separator(); ImGui::Spacing();
        ImGui::Text("Trials              : %d", r.trials);
        ImGui::Text("Points inside circle: %d", r.inCircle);
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f,0.85f,0.1f,1.0f),
                           "Estimated PI        : %s", fmtD(r.estimatedPi,9).c_str());
        ImGui::Text("Actual PI           : 3.141592654");
        ImGui::Spacing();
        ImVec4 ec = r.error < 0.001 ? ImVec4(0.2f,0.9f,0.3f,1.0f)
                  : r.error < 0.01  ? ImVec4(1.0f,0.85f,0.1f,1.0f)
                                    : ImVec4(1.0f,0.3f,0.3f,1.0f);
        ImGui::TextColored(ec, "Absolute Error      : %s", fmtD(r.error,9).c_str());
        ImGui::TextColored(ImVec4(0.4f,0.8f,0.9f,1.0f),
                           "Theoretical Bound   : %s", fmtD(r.theoreticalBound,6).c_str());
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        ImGui::Text("Error relative to bound:");
        float ratio = (r.theoreticalBound > 0) ? (float)(r.error/r.theoreticalBound) : 0.f;
        if (ratio > 1.f) ratio = 1.f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ec);
        ImGui::ProgressBar(ratio, ImVec2(-1, 10), "");
        ImGui::PopStyleColor();
    }
    else if (g_piState == PiState::EXPERIMENT) {
        ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Experiment Mode — All Sizes");
        ImGui::Separator(); ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.9f,0.9f,0.4f,1.0f),
                           "%-12s %-16s %-12s %s", "Trials","Estimated PI","Error","Bound");
        ImGui::Separator();
        for (const auto &r : g_piExpResults) {
            ImVec4 c = r.error < 0.001 ? ImVec4(0.2f,0.9f,0.3f,1.0f)
                     : r.error < 0.01  ? ImVec4(1.0f,0.85f,0.1f,1.0f)
                                       : ImVec4(1.0f,0.3f,0.3f,1.0f);
            ImGui::TextColored(c, "%-12d %-16s %-12s %s",
                r.trials, fmtD(r.estimatedPi,9).c_str(),
                fmtD(r.error,9).c_str(), fmtD(r.theoreticalBound,6).c_str());
        }
    }
    ImGui::EndChild();
}

static void DrawPiCustomPopup() {
    if (!g_piCustomOpen) return;
    ImGui::OpenPopup("Custom Trials");
    ImGui::SetNextWindowSize(ImVec2(300,110), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Custom Trials", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::Spacing();
        ImGui::Text("Enter number of trials:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##pici", g_piCustomBuf, sizeof(g_piCustomBuf),
                         ImGuiInputTextFlags_CharsDecimal);
        ImGui::Spacing();
        if (ImGui::Button("Run", ImVec2(120,0))) {
            g_piCustomOpen = false; PiRunChoice(8); ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120,0))) {
            g_piCustomOpen = false; ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  TAB 2 — RANDOM WALK
 * ════════════════════════════════════════════════════════════════════ */

static void WalkRunSim() {
    g_walkStats        = g_walkSim.run(g_walkParticles, g_walkSteps);
    g_walkState        = WalkState::RESULT;
    g_walkViewParticle = 0;
    g_walkStatus       = "Press R to run again  |  Esc to go back  |  < > to switch particle";
}

static void DrawWalkMenu(float w, float h) {
    ImGui::BeginChild("##walkmenu", ImVec2(w, h), true);
    ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Random Walk");
    ImGui::Separator(); ImGui::Spacing();

    // ── Particle count ───────────────────────────────────────────
    ImGui::TextColored(ImVec4(0.9f,0.9f,0.4f,1.0f), "Particles:");
    ImGui::Spacing();

    struct PItem { int id; const char *label; int count; };
    static const PItem pitems[] = {
        {1,"1 Particle",1},{2,"5 Particles",5},
        {3,"10 Particles",10},{4,"100 Particles",100},{5,"Custom"},
    };
    for (auto &it : pitems) {
        if (MenuButton(it.label, w - 18, g_walkHighlight == it.id)) {
            g_walkHighlight = it.id;
            if (it.id == 5) g_walkParticleOpen = true;
            else            g_walkParticles = it.count;
        }
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // ── Step count ───────────────────────────────────────────────
    ImGui::TextColored(ImVec4(0.9f,0.9f,0.4f,1.0f), "Steps:");
    ImGui::Spacing();

    struct SItem { int id; const char *label; int steps; };
    static const SItem sitems[] = {
        {1,"100 Steps",100},{2,"1,000 Steps",1000},
        {3,"10,000 Steps",10000},{4,"100,000 Steps",100000},{5,"Custom"},
    };
    for (auto &it : sitems) {
        if (MenuButton(it.label, w - 18, g_walkStepHL == it.id)) {
            g_walkStepHL = it.id;
            if (it.id == 5) g_walkStepOpen = true;
            else            g_walkSteps = it.steps;
        }
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // ── Run button ───────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.6f,0.2f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f,0.75f,0.25f,1.0f));
    if (ImGui::Button("Run Simulation", ImVec2(w-18, 32))) WalkRunSim();
    ImGui::PopStyleColor(2);

    ImGui::EndChild();
}

static void DrawWalkCanvas(float w, float h) {
    ImGui::BeginChild("##walkcanvas", ImVec2(w, h), true);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    float  W  = w - 16.0f, H = h - 16.0f;

    dl->AddRectFilled(p0, ImVec2(p0.x+W, p0.y+H), IM_COL32(12,12,20,255));
    dl->AddRect      (p0, ImVec2(p0.x+W, p0.y+H), IM_COL32(60,180,200,180));

    if (g_walkState == WalkState::MENU || g_walkStats.walks.empty()) {
        dl->AddText(ImVec2(p0.x + W*0.2f, p0.y + H*0.45f),
                    IM_COL32(80,80,100,255), "Run a simulation to see the random walk path");
        ImGui::EndChild(); return;
    }

    const WalkResult &walk = g_walkStats.walks[g_walkViewParticle];
    const auto &path = walk.path;
    if (path.size() < 2) { ImGui::EndChild(); return; }

    // Find bounding box of the path to scale it to the canvas
    double minX = path[0].first,  maxX = path[0].first;
    double minY = path[0].second, maxY = path[0].second;
    for (auto &pt : path) {
        if (pt.first  < minX) minX = pt.first;
        if (pt.first  > maxX) maxX = pt.first;
        if (pt.second < minY) minY = pt.second;
        if (pt.second > maxY) maxY = pt.second;
    }
    // Add padding so the path doesn't touch the edges
    double rangeX = (maxX - minX) * 1.1 + 1.0;
    double rangeY = (maxY - minY) * 1.1 + 1.0;
    double cx     = (minX + maxX) * 0.5;
    double cy     = (minY + maxY) * 0.5;

    auto toX = [&](double v){ return p0.x + (float)((v - cx + rangeX*0.5) / rangeX * W); };
    auto toY = [&](double v){ return p0.y + H - (float)((v - cy + rangeY*0.5) / rangeY * H); };

    // Draw origin crosshair
    float ox = toX(0), oy = toY(0);
    dl->AddLine(ImVec2(ox-8,oy), ImVec2(ox+8,oy), IM_COL32(80,80,80,180), 1.f);
    dl->AddLine(ImVec2(ox,oy-8), ImVec2(ox,oy+8), IM_COL32(80,80,80,180), 1.f);

    // Draw the path — colour fades from white (start) to yellow (end)
    for (size_t i = 1; i < path.size(); i++) {
        float t   = (float)i / (float)(path.size() - 1);
        ImU32 col = IM_COL32((int)(80  + 175*t),
                             (int)(180 - 80*t),
                             (int)(220 - 220*t), 200);
        dl->AddLine(ImVec2(toX(path[i-1].first), toY(path[i-1].second)),
                    ImVec2(toX(path[i].first),   toY(path[i].second)),
                    col, 1.2f);
    }

    // Start (green circle) and End (red circle)
    dl->AddCircleFilled(ImVec2(toX(path.front().first), toY(path.front().second)),
                        5.f, IM_COL32(60,220,80,255));
    dl->AddCircleFilled(ImVec2(toX(path.back().first),  toY(path.back().second)),
                        5.f, IM_COL32(220,60,60,255));

    // Legend
    float lx = p0.x+8, ly = p0.y+H-36;
    dl->AddCircleFilled(ImVec2(lx+5,ly),  4.f, IM_COL32(60,220,80,255));
    dl->AddText(ImVec2(lx+12,ly-6), IM_COL32(200,200,200,255), "Start");
    dl->AddCircleFilled(ImVec2(lx+60,ly), 4.f, IM_COL32(220,60,60,255));
    dl->AddText(ImVec2(lx+67,ly-6), IM_COL32(200,200,200,255), "End");

    // Particle label
    char lbl[32];
    snprintf(lbl, sizeof(lbl), "Particle %d / %d",
             g_walkViewParticle+1, (int)g_walkStats.walks.size());
    dl->AddText(ImVec2(p0.x + W - 130, p0.y + 8), IM_COL32(180,180,100,255), lbl);

    ImGui::EndChild();
}

static void DrawWalkResults(float w, float h) {
    ImGui::BeginChild("##walkresults", ImVec2(w, h), true);

    if (g_walkState == WalkState::MENU) {
        ImGui::TextDisabled("No results yet."); ImGui::EndChild(); return;
    }

    ImGui::TextColored(ImVec4(0.3f,0.9f,1.0f,1.0f), "Random Walk Results");
    ImGui::Separator(); ImGui::Spacing();

    ImGui::Text("Particles           : %d", g_walkStats.numParticles);
    ImGui::Text("Steps per particle  : %d", g_walkStats.numSteps);
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f,0.8f,0.9f,1.0f),
                       "Expected dist (sqrt(N)) : %s",
                       fmtD(g_walkStats.expectedDist, 4).c_str());
    ImGui::TextColored(ImVec4(1.0f,0.85f,0.1f,1.0f),
                       "Avg final distance      : %s",
                       fmtD(g_walkStats.avgFinalDist, 4).c_str());
    ImGui::Text("Avg max distance        : %s",
                fmtD(g_walkStats.avgMaxDist, 4).c_str());

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // Per-particle table
    ImGui::TextColored(ImVec4(0.9f,0.9f,0.4f,1.0f),
                       "%-12s %-14s %-14s", "Particle","Final Dist","Max Dist");
    ImGui::Separator();

    for (int i = 0; i < (int)g_walkStats.walks.size(); i++) {
        const WalkResult &r = g_walkStats.walks[i];
        bool viewing = (i == g_walkViewParticle);
        ImVec4 c = viewing ? ImVec4(0.3f,0.9f,1.0f,1.0f) : ImVec4(0.75f,0.75f,0.75f,1.0f);
        ImGui::TextColored(c, "%-12d %-14s %-14s",
                           i+1,
                           fmtD(r.finalDist,4).c_str(),
                           fmtD(r.maxDist,4).c_str());
    }

    // Particle navigator
    if (g_walkStats.walks.size() > 1) {
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        ImGui::Text("View particle path:");
        ImGui::SameLine();
        if (ImGui::ArrowButton("##prev", ImGuiDir_Left)) {
            if (g_walkViewParticle > 0) g_walkViewParticle--;
        }
        ImGui::SameLine();
        ImGui::Text("%d", g_walkViewParticle + 1);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##next", ImGuiDir_Right)) {
            if (g_walkViewParticle < (int)g_walkStats.walks.size()-1)
                g_walkViewParticle++;
        }
    }

    ImGui::EndChild();
}

static void DrawWalkCustomPopups() {
    // Particle count popup
    if (g_walkParticleOpen) {
        ImGui::OpenPopup("Custom Particles");
        ImGui::SetNextWindowSize(ImVec2(280,100), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("Custom Particles", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Spacing();
            ImGui::Text("Enter number of particles:");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##wpc", g_walkParticleBuf, sizeof(g_walkParticleBuf),
                             ImGuiInputTextFlags_CharsDecimal);
            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(110,0))) {
                int n = atoi(g_walkParticleBuf);
                if (n > 0) g_walkParticles = n;
                g_walkParticleOpen = false; ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(110,0))) {
                g_walkParticleOpen = false; ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // Step count popup
    if (g_walkStepOpen) {
        ImGui::OpenPopup("Custom Steps");
        ImGui::SetNextWindowSize(ImVec2(280,100), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("Custom Steps", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Spacing();
            ImGui::Text("Enter number of steps:");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##wsc", g_walkStepBuf, sizeof(g_walkStepBuf),
                             ImGuiInputTextFlags_CharsDecimal);
            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(110,0))) {
                int n = atoi(g_walkStepBuf);
                if (n > 0) g_walkSteps = n;
                g_walkStepOpen = false; ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(110,0))) {
                g_walkStepOpen = false; ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  KEYBOARD — handles both tabs
 * ════════════════════════════════════════════════════════════════════ */

static void HandleKeyboard(int activeTab) {
    if (activeTab == 0) {
        // PI tab
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)   && g_piHighlight > 1) g_piHighlight--;
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)  && g_piHighlight < 8) g_piHighlight++;
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            if (g_piHighlight == 8) g_piCustomOpen = true;
            else PiRunChoice(g_piHighlight);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R) && g_piState != PiState::MENU)
            PiRunChoice(g_piHighlight);
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && g_piState != PiState::MENU) {
            g_piState  = PiState::MENU;
            g_piStatus = "Click a size or use arrow keys + Enter";
        }
    } else {
        // Walk tab
        if (ImGui::IsKeyPressed(ImGuiKey_R) && g_walkState != WalkState::MENU)
            WalkRunSim();
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && g_walkState != WalkState::MENU) {
            g_walkState  = WalkState::MENU;
            g_walkStatus = "Choose particle count, then step count";
        }
        // Left/right to flip through particles
        if (g_walkState == WalkState::RESULT && !g_walkStats.walks.empty()) {
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && g_walkViewParticle > 0)
                g_walkViewParticle--;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) &&
                g_walkViewParticle < (int)g_walkStats.walks.size()-1)
                g_walkViewParticle++;
        }
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  MAIN RENDER FRAME
 * ════════════════════════════════════════════════════════════════════ */

static void RenderFrame() {
    RECT rc; GetClientRect(g_hwnd, &rc);
    float W = (float)(rc.right - rc.left);
    float H = (float)(rc.bottom - rc.top);

    ImGui::SetNextWindowPos (ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(W,H));
    ImGui::Begin("##root", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    // ── Title bar ─────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f,0.22f,0.28f,1.0f));
    ImGui::BeginChild("##title", ImVec2(0,36), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(12,9));
    ImGui::TextColored(ImVec4(1.0f,0.85f,0.2f,1.0f), "Monte Carlo Simulator");
    ImGui::SameLine(0,30);
    ImGui::TextColored(ImVec4(0.5f,0.8f,0.85f,1.0f),
                       "Click or arrow keys to navigate  |  Enter to run  |  R to repeat  |  Esc to back");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // ── Tab bar ───────────────────────────────────────────────────
    static int activeTab = 0;
    ImGui::SetCursorPosY(36.0f);

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("PI Estimator")) {
            activeTab = 0;
            HandleKeyboard(0);

            float contentH = H - 36.0f - 30.0f - 28.0f;
            float menuW    = 220.0f;
            float rightW   = W - menuW - 8.0f;
            float canvasW  = rightW * 0.55f;
            float resultsW = rightW * 0.45f - 8.0f;

            ImGui::SetCursorPosX(4.0f);
            DrawPiMenu   (menuW,    contentH);
            ImGui::SameLine(0,4);
            DrawPiCanvas (canvasW,  contentH);
            ImGui::SameLine(0,4);
            DrawPiResults(resultsW, contentH);
            DrawPiCustomPopup();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Random Walk")) {
            activeTab = 1;
            HandleKeyboard(1);

            float contentH = H - 36.0f - 30.0f - 28.0f;
            float menuW    = 230.0f;
            float rightW   = W - menuW - 8.0f;
            float canvasW  = rightW * 0.58f;
            float resultsW = rightW * 0.42f - 8.0f;

            ImGui::SetCursorPosX(4.0f);
            DrawWalkMenu   (menuW,    contentH);
            ImGui::SameLine(0,4);
            DrawWalkCanvas (canvasW,  contentH);
            ImGui::SameLine(0,4);
            DrawWalkResults(resultsW, contentH);
            DrawWalkCustomPopups();

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // ── Status bar ────────────────────────────────────────────────
    ImGui::SetCursorPosY(H - 26.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f,0.14f,0.18f,1.0f));
    ImGui::BeginChild("##status", ImVec2(0,24), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(8,4));
    const std::string &msg = (activeTab == 0) ? g_piStatus : g_walkStatus;
    ImGui::TextColored(ImVec4(0.4f,0.8f,0.85f,1.0f), "%s", msg.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}

/* ════════════════════════════════════════════════════════════════════
 *  WIN32 ENTRY POINT
 * ════════════════════════════════════════════════════════════════════ */

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    g_wc = { sizeof(g_wc), CS_CLASSDC, WndProc, 0L, 0L,
             GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
             L"MonteCarlo", nullptr };
    RegisterClassExW(&g_wc);

    g_hwnd = CreateWindowW(L"MonteCarlo", L"Monte Carlo Simulator",
                           WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
                           nullptr, nullptr, g_wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 3.0f;
    style.TabRounding       = 4.0f;
    style.ItemSpacing       = ImVec2(6,5);
    style.Colors[ImGuiCol_WindowBg]      = ImVec4(0.06f,0.06f,0.09f,1.0f);
    style.Colors[ImGuiCol_ChildBg]       = ImVec4(0.08f,0.08f,0.12f,1.0f);
    style.Colors[ImGuiCol_FrameBg]       = ImVec4(0.10f,0.10f,0.15f,1.0f);
    style.Colors[ImGuiCol_Border]        = ImVec4(0.20f,0.50f,0.60f,0.5f);
    style.Colors[ImGuiCol_Tab]           = ImVec4(0.10f,0.20f,0.26f,1.0f);
    style.Colors[ImGuiCol_TabHovered]    = ImVec4(0.15f,0.40f,0.50f,1.0f);
    style.Colors[ImGuiCol_TabActive]     = ImVec4(0.05f,0.45f,0.55f,1.0f);
    style.Colors[ImGuiCol_PopupBg]       = ImVec4(0.08f,0.10f,0.14f,1.0f);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(g_d3d_device);

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

        g_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                            D3DCOLOR_RGBA(12,12,18,255), 1.0f, 0);

        if (g_d3d_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_d3d_device->EndScene();
        }
        g_d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
    return 0;
}
