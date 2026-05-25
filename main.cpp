#include "raylib.h"
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sys/stat.h>
#include <algorithm>

struct Crop {
    int x, y, w, h;
};

static void EnsureDir(const std::string& p) {
    mkdir(p.c_str(), 0755);
}

static std::string DirOf(const std::string& p) {
    size_t slash = p.find_last_of("/\\");
    if (slash == std::string::npos) return ".";
    return p.substr(0, slash);
}

static std::string StripExt(const std::string& p) {
    size_t slash = p.find_last_of("/\\");
    size_t dot = p.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return p;
    return p.substr(0, dot);
}

static std::string LowerExt(const std::string& p) {
    size_t dot = p.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string e = p.substr(dot);
    for (auto& c : e) c = (char)tolower((unsigned char)c);
    return e;
}

static bool SaveCuts(const std::string& path, const std::vector<Crop>& crops) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return false;
    fprintf(f, "# spraysheetcut layout v1\n");
    for (const auto& c : crops) fprintf(f, "%d %d %d %d\n", c.x, c.y, c.w, c.h);
    fclose(f);
    return true;
}

static bool LoadCuts(const std::string& path, std::vector<Crop>& crops) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return false;
    crops.clear();
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        int x, y, w, h;
        if (sscanf(line, "%d %d %d %d", &x, &y, &w, &h) == 4) crops.push_back({x, y, w, h});
    }
    fclose(f);
    return true;
}

static Rectangle Normalized(float ax, float ay, float bx, float by) {
    float x = std::min(ax, bx);
    float y = std::min(ay, by);
    float w = std::fabs(bx - ax);
    float h = std::fabs(by - ay);
    return { x, y, w, h };
}

int main(int argc, char** argv) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "SpraySheetCut");
    SetTargetFPS(60);

    Image img = { 0 };
    Texture2D tex = { 0 };
    std::string loadedPath;

    auto loadFile = [&](const std::string& path) {
        if (tex.id) UnloadTexture(tex);
        if (img.data) UnloadImage(img);
        img = LoadImage(path.c_str());
        if (img.data) {
            tex = LoadTextureFromImage(img);
            loadedPath = path;
        }
    };

    if (argc > 1) loadFile(argv[1]);

    std::vector<Crop> crops;
    Vector2 camOffset = { 0, 0 };
    float zoom = 1.0f;

    bool dragging = false;
    Vector2 dragStart = { 0, 0 };

    bool panning = false;
    Vector2 panStart = { 0, 0 };
    Vector2 panOffsetStart = { 0, 0 };

    int selected = -1;
    std::string status = "Drag image or .cuts here. LMB=crop  Wheel=zoom  MMB/Space=pan  S=save sprites  E=export .cuts  Z=undo  C=clear  Del=remove  R=reset";

    while (!WindowShouldClose()) {
        if (IsFileDropped()) {
            FilePathList drop = LoadDroppedFiles();
            if (drop.count > 0) {
                std::string p = drop.paths[0];
                if (LowerExt(p) == ".cuts") {
                    if (LoadCuts(p, crops)) {
                        selected = -1;
                        char buf[256];
                        snprintf(buf, sizeof(buf), "Loaded %zu crop(s) from %s", crops.size(), p.c_str());
                        status = buf;
                    } else {
                        status = "Failed to load .cuts file";
                    }
                } else {
                    loadFile(p);
                    crops.clear();
                    selected = -1;
                    camOffset = { 0, 0 };
                    zoom = 1.0f;
                    std::string sidecar = StripExt(p) + ".cuts";
                    struct stat st;
                    if (stat(sidecar.c_str(), &st) == 0 && LoadCuts(sidecar, crops)) {
                        char buf[256];
                        snprintf(buf, sizeof(buf), "Image loaded + auto-applied %zu crops from %s", crops.size(), sidecar.c_str());
                        status = buf;
                    }
                }
            }
            UnloadDroppedFiles(drop);
        }

        Vector2 mouse = GetMousePosition();
        Vector2 worldMouse = { (mouse.x - camOffset.x) / zoom, (mouse.y - camOffset.y) / zoom };

        float wheel = GetMouseWheelMove();
        if (wheel != 0 && tex.id) {
            float newZoom = zoom * (wheel > 0 ? 1.15f : 1.0f/1.15f);
            newZoom = std::max(0.05f, std::min(40.0f, newZoom));
            camOffset.x = mouse.x - worldMouse.x * newZoom;
            camOffset.y = mouse.y - worldMouse.y * newZoom;
            zoom = newZoom;
        }

        bool spaceHeld = IsKeyDown(KEY_SPACE);
        bool wantPan = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
                       (spaceHeld && IsMouseButtonDown(MOUSE_BUTTON_LEFT));

        if (wantPan && !panning && !dragging) {
            panning = true;
            panStart = mouse;
            panOffsetStart = camOffset;
        }
        if (panning) {
            camOffset.x = panOffsetStart.x + (mouse.x - panStart.x);
            camOffset.y = panOffsetStart.y + (mouse.y - panStart.y);
            if (!wantPan) panning = false;
        }

        if (tex.id && !panning) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !spaceHeld) {
                int hit = -1;
                for (int i = (int)crops.size() - 1; i >= 0; i--) {
                    Rectangle r = { (float)crops[i].x, (float)crops[i].y, (float)crops[i].w, (float)crops[i].h };
                    if (CheckCollisionPointRec(worldMouse, r)) { hit = i; break; }
                }
                if (hit >= 0) {
                    selected = hit;
                } else {
                    dragging = true;
                    dragStart = worldMouse;
                    selected = -1;
                }
            }
            if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                Rectangle r = Normalized(dragStart.x, dragStart.y, worldMouse.x, worldMouse.y);
                int x = (int)std::round(r.x);
                int y = (int)std::round(r.y);
                int w = (int)std::round(r.width);
                int h = (int)std::round(r.height);
                x = std::max(0, std::min(x, img.width - 1));
                y = std::max(0, std::min(y, img.height - 1));
                if (x + w > img.width) w = img.width - x;
                if (y + h > img.height) h = img.height - y;
                if (w >= 2 && h >= 2) {
                    crops.push_back({ x, y, w, h });
                    selected = (int)crops.size() - 1;
                }
                dragging = false;
            }
        }

        if (IsKeyPressed(KEY_R)) { camOffset = { 0, 0 }; zoom = 1.0f; }
        if (IsKeyPressed(KEY_Z) && !crops.empty()) {
            crops.pop_back();
            if (selected >= (int)crops.size()) selected = -1;
        }
        if (IsKeyPressed(KEY_C)) { crops.clear(); selected = -1; }
        if (IsKeyPressed(KEY_DELETE) && selected >= 0 && selected < (int)crops.size()) {
            crops.erase(crops.begin() + selected);
            selected = -1;
        }

        if (IsKeyPressed(KEY_E) && !crops.empty()) {
            std::string out = loadedPath.empty() ? std::string("layout.cuts") : (StripExt(loadedPath) + ".cuts");
            if (SaveCuts(out, crops)) {
                char buf[256];
                snprintf(buf, sizeof(buf), "Exported %zu crop(s) to %s", crops.size(), out.c_str());
                status = buf;
            } else {
                status = "Failed to export .cuts";
            }
        }

        if (IsKeyPressed(KEY_S) && img.data && !crops.empty()) {
            std::string outDir = (loadedPath.empty() ? std::string(".") : DirOf(loadedPath)) + "/output";
            EnsureDir(outDir);
            int saved = 0;
            for (size_t i = 0; i < crops.size(); i++) {
                Image sub = ImageFromImage(img, { (float)crops[i].x, (float)crops[i].y, (float)crops[i].w, (float)crops[i].h });
                char name[512];
                snprintf(name, sizeof(name), "%s/sprite_%03zu.png", outDir.c_str(), i + 1);
                if (ExportImage(sub, name)) saved++;
                UnloadImage(sub);
            }
            char buf[512];
            snprintf(buf, sizeof(buf), "Saved %d sprite(s) to %s/", saved, outDir.c_str());
            status = buf;
        }

        BeginDrawing();
        ClearBackground((Color){ 30, 30, 35, 255 });

        if (tex.id) {
            Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
            Rectangle dst = { camOffset.x, camOffset.y, tex.width * zoom, tex.height * zoom };
            DrawTexturePro(tex, src, dst, { 0, 0 }, 0.0f, WHITE);

            for (size_t i = 0; i < crops.size(); i++) {
                Rectangle r = {
                    camOffset.x + crops[i].x * zoom,
                    camOffset.y + crops[i].y * zoom,
                    crops[i].w * zoom,
                    crops[i].h * zoom
                };
                Color c = ((int)i == selected) ? YELLOW : GREEN;
                DrawRectangleLinesEx(r, 2.0f, c);
                char lbl[16];
                snprintf(lbl, sizeof(lbl), "%zu", i + 1);
                DrawText(lbl, (int)r.x + 4, (int)r.y + 2, 18, c);
            }

            if (dragging) {
                Rectangle r = Normalized(dragStart.x, dragStart.y, worldMouse.x, worldMouse.y);
                Rectangle screen = {
                    camOffset.x + r.x * zoom,
                    camOffset.y + r.y * zoom,
                    r.width * zoom,
                    r.height * zoom
                };
                DrawRectangleLinesEx(screen, 2.0f, RED);
            }
        } else {
            DrawText("Drag a PNG/JPG into this window to start", 40, GetScreenHeight()/2 - 10, 22, LIGHTGRAY);
        }

        DrawRectangle(0, 0, GetScreenWidth(), 28, (Color){ 0, 0, 0, 180 });
        DrawText(status.c_str(), 8, 6, 16, RAYWHITE);

        char info[256];
        snprintf(info, sizeof(info), "crops:%zu  zoom:%.2fx  %s", crops.size(), zoom,
                 loadedPath.empty() ? "(no file)" : loadedPath.c_str());
        DrawRectangle(0, GetScreenHeight()-24, GetScreenWidth(), 24, (Color){ 0, 0, 0, 180 });
        DrawText(info, 8, GetScreenHeight()-20, 16, RAYWHITE);

        EndDrawing();
    }

    if (tex.id) UnloadTexture(tex);
    if (img.data) UnloadImage(img);
    CloseWindow();
    return 0;
}
