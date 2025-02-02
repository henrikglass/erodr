
#include "ui.h"
#include "shaders/shaders.h"

#include "raylib.h"
#include "rlgl.h"

#define RAYMATH_STATIC_INLINE
#include "raymath.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* Yes, intertia is frame rate-dependent. Idc lol. */
#define PAN_INERTIA        0.9f
#define ZOOM_INERTIA       0.7f
#define FOV_Y             45.0f
#define ZOOM_DEFAULT     100.0f
#define PLANE_SIZE        64.0f
#define MESH_RES         256
#define SCREEN_WIDTH    1920
#define SCREEN_HEIGHT   1080

static float sample_hmap(ErodrImage *hmap, float xf, float yf)
{
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if 0
    /* nearest */
    int x = xf * hmap->width;
    int y = yf * hmap->height;
    return hmap->data[y * hmap->width + x];
#else
    /* bilinear */
    float x = xf * hmap->width;
    float y = yf * hmap->height;
    float t_x = x - roundf(x - 0.5f);
    float t_y = y - roundf(y - 0.5f);
    int ileft   = (int) x;
    int itop    = (int) y;
    int iright  = MIN(ileft + 1, hmap->width - 1);
    int ibottom = MIN(itop + 1, hmap->height - 1);
    float top_left     = hmap->data[itop * hmap->width + ileft];
    float top_right    = hmap->data[itop * hmap->width + iright];
    float bottom_left  = hmap->data[ibottom * hmap->width + ileft];
    float bottom_right = hmap->data[ibottom * hmap->width + iright];
    float top = (1.0f - t_x) * top_left + t_x * top_right;
    float bottom = (1.0f - t_x) * bottom_left + t_x * bottom_right;
    return ((1.0f - t_y) * top + t_y * bottom);
#endif
}

static float clamp(float value, float min, float max)
{
	if (value < min) return min;
	if (value > max) return max;
    return value;
}

void *ui_run(void *args)
{
    /* args */
    UiArgs *ui_args = (UiArgs *) args;
    SimulationParameters *sim_params = ui_args->sim_params;
    ErodrImage *hmap = ui_args->hmap;
    HglChan *c = ui_args->chan;

    /* Window */
    int screen_width  = SCREEN_WIDTH;
    int screen_height = SCREEN_HEIGHT;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screen_width, screen_height, "Erodr UI");

    /* Camera */
    Camera camera = {
        .position   = Vector3Scale(Vector3Normalize((Vector3){ 18.0f, 21.0f, 18.0f }), ZOOM_DEFAULT),
        .target     = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up         = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy       = FOV_Y,
        .projection = CAMERA_PERSPECTIVE,
    };

    /* Heightmap mesh */
    Mesh hmap_mesh = GenMeshPlane(PLANE_SIZE, PLANE_SIZE, MESH_RES - 1, MESH_RES - 1);
    assert(hmap_mesh.vertexCount <= 0x10000); // Raylib only supports short int indicies.. :/

    /* Heightmap tform */
    Matrix    hmap_tform    = MatrixTranslate(0.0f, 0.10f, 0.0f);

    /* Heightmap material */
    Material  hmap_material = LoadMaterialDefault();
    Image hmap_image = (Image) {
        .data = hmap->data,
        .width = hmap->width,
        .height = hmap->height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32,
    };
    Texture2D hmap_texture  = LoadTextureFromImage(hmap_image);
    SetTextureFilter(hmap_texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(hmap_texture, TEXTURE_WRAP_MIRROR_REPEAT);
    SetMaterialTexture(&hmap_material, MATERIAL_MAP_ALBEDO, hmap_texture);

    /* Heightmap material shader */
    hmap_material.shader = LoadShaderFromMemory(NULL, SHADERS_HMAP_FRAG_SRC);
    int shader_mode_loc         = GetShaderLocation(hmap_material.shader, "mode");
    int shader_res_loc          = GetShaderLocation(hmap_material.shader, "res");
    int shader_snow_thresh_loc  = GetShaderLocation(hmap_material.shader, "snow_threshold");
    int shader_snow_pooling_loc = GetShaderLocation(hmap_material.shader, "snow_pooling");
    int shader_brightness_loc   = GetShaderLocation(hmap_material.shader, "brightness");
    int shader_mode           = 0;
    float shader_res          = (float) hmap->width;
    float shader_snow_thresh  = 0.002f;
    float shader_snow_pooling = 3.000f;
    float shader_brightness   = 1.39f;
    SetShaderValue(hmap_material.shader, shader_mode_loc, &shader_mode, SHADER_UNIFORM_INT);
    SetShaderValue(hmap_material.shader, shader_res_loc, &shader_res, SHADER_UNIFORM_FLOAT);
    SetShaderValue(hmap_material.shader, shader_snow_thresh_loc, &shader_snow_thresh, SHADER_UNIFORM_FLOAT);
    SetShaderValue(hmap_material.shader, shader_snow_pooling_loc, &shader_snow_pooling, SHADER_UNIFORM_FLOAT);
    SetShaderValue(hmap_material.shader, shader_brightness_loc, &shader_brightness, SHADER_UNIFORM_FLOAT);

    /* Camera controls */
    Vector2 mouse = Vector2Zero();
    float zoom = 0.0f;
    
    /* misc */
    float terrain_height = 16.0f;
    bool running = true;

    while (running) {
        /* ====== update ================================ */

        /* handle resizing */
        if (IsWindowResized() && !IsWindowFullscreen()) {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
        }

        /* reset heightmap */
        if (IsKeyPressed(KEY_R)) {
            hgl_chan_send(c, (void *)CMD_RESET_HMAP);
        }

        /* reload simulation parameters */
        if (IsKeyPressed(KEY_E)) {
            hgl_chan_send(c, (void *)CMD_RELOAD_SIMPARAMS);
        }

        /* save image */
        if (IsKeyPressed(KEY_S)) {
            hgl_chan_send(c, (void *)CMD_SAVE_HMAP);
        }
        
        /* view mode */
        if (IsKeyPressed(KEY_V)) {
            shader_mode = (shader_mode + 1) % 4;
            SetShaderValue(hmap_material.shader, shader_mode_loc, &shader_mode, SHADER_UNIFORM_INT);
        } 

        /* handle closing */
        running = !WindowShouldClose();
        if (IsKeyPressed(KEY_Q)) {
            running = false;
            break;
        }

        /* re-run simulation */
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            hgl_chan_send(c, (void *)CMD_RERUN_SIMULATION);
        }

        /* toggle fullscreen */
        if (IsKeyPressed(KEY_F11)) {
            int display = GetCurrentMonitor();
            if (IsWindowFullscreen()) {
                SetWindowSize(screen_width, screen_height);
            } else {
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            }
            ToggleFullscreen();
        }

        /* toggle projection mode */
        if (IsKeyPressed(KEY_P)) {
            if (camera.projection == CAMERA_ORTHOGRAPHIC) {
                camera.projection = CAMERA_PERSPECTIVE;
            } else if (camera.projection == CAMERA_PERSPECTIVE) {
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.position = Vector3Scale(Vector3Normalize(Vector3Subtract(camera.position, camera.target)), ZOOM_DEFAULT);
                zoom = 0.0f;
            }
        }

        /* adjust camera target/position based on terrain_height setting */
        camera.target.y = hmap_tform.m13 + terrain_height/2.0f;

        /* terrain_height adjustment */
        Vector2 mouse_delta = GetMouseDelta();
        if (IsMouseButtonDown(1)) {
            terrain_height -= 0.1f*mouse_delta.y;
        }

        /* "snow" threshold adjustment */
        if (IsMouseButtonDown(2)) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                shader_snow_pooling -= 0.01f*mouse_delta.y;
                shader_snow_pooling = clamp(shader_snow_pooling, 1.0, 10.0);
                SetShaderValue(hmap_material.shader, shader_snow_pooling_loc, &shader_snow_pooling, SHADER_UNIFORM_FLOAT);
            } else {
                shader_snow_thresh -= 0.00001f*mouse_delta.y;
                shader_snow_thresh = clamp(shader_snow_thresh, 0.0, 10.0);
                SetShaderValue(hmap_material.shader, shader_snow_thresh_loc, &shader_snow_thresh, SHADER_UNIFORM_FLOAT);
            }
        }

        /* camera movement */
        if (IsMouseButtonDown(0)) {
            mouse = mouse_delta;
            zoom = 0.0f;
        }
        Vector3 vcam = Vector3Subtract(camera.position, camera.target);
        vcam = Vector3RotateByAxisAngle(vcam, camera.up, -mouse.x / 360.0f);
        vcam = Vector3RotateByAxisAngle(vcam, Vector3CrossProduct(vcam, camera.up), mouse.y / 360.0f);
        camera.position = Vector3Add(camera.target, vcam);
        mouse = Vector2Scale(mouse, PAN_INERTIA);

        /* camera zoom */
        float zoom_delta = GetMouseWheelMove();
        if ((fabsf(zoom_delta) > 0.001f) && camera.projection == CAMERA_PERSPECTIVE) {
            zoom = zoom_delta;
        }
        Vector3 view_dir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        camera.position = Vector3Add(camera.position, Vector3Scale(view_dir, 3*zoom));
        zoom *= ZOOM_INERTIA;

        /* update mesh & texture */
        for (int y = 0; y < MESH_RES; y++) {
            for (int x = 0; x < MESH_RES; x++) {
                float xf = (float)x / (float)MESH_RES;
                float yf = (float)y / (float)MESH_RES;
                hmap_mesh.vertices[y*MESH_RES*3 + x*3 + 1] = terrain_height * sample_hmap(hmap, xf, yf);
            }
        }
        UpdateMeshBuffer(hmap_mesh, 0, hmap_mesh.vertices, MESH_RES*MESH_RES*3*sizeof(float), 0);
        UpdateTexture(hmap_texture, hmap->data);

        /* ====== draw ================================== */
        BeginDrawing();
            ClearBackground(RAYWHITE);

            /* Draw heightmap */
            BeginMode3D(camera);
                DrawMesh(hmap_mesh, hmap_material, hmap_tform);
            EndMode3D();
        
            /* Section "Commands" */
            DrawFPS(screen_width - 100, 10);
            DrawText("Simulation Controls: ", 10, 10, 38, BLACK);
            DrawText(TextFormat("R - reset heightmap"), 10, 50, 24, BLACK);
            DrawText(TextFormat("E - reload simulation parameters from file"), 10, 80, 24, BLACK);
            DrawText(TextFormat("Enter/Space - run erosion simulation"), 10, 110, 24, BLACK);
            DrawText(TextFormat("S - Save image"), 10, 140, 24, BLACK);
            DrawText(TextFormat("Esc/Q - exit"), 10, 170, 24, BLACK);

            //int xpos = screen_width - 770;
            DrawText("Visualization Controls:", 10, 250, 38, BLACK);
            DrawText(TextFormat("V - Cycle between visualizer modes (%d)", shader_mode + 1), 10, 290, 24, BLACK);
            DrawText(TextFormat("P - projection mode (%s)", (camera.projection == CAMERA_PERSPECTIVE) ? 
                                "perspective" : "orthographic"), 10, 320, 24, BLACK);
            DrawText(TextFormat("Left mouse button/scroll - move camera"), 10, 350, 24, BLACK);
            DrawText(TextFormat("Middle mouse button - change snow cover (%2.5f)", shader_snow_thresh), 10, 380, 24, BLACK);
            DrawText(TextFormat("Lshift + Middle mouse button - change snow pooling (%2.2f)", shader_snow_pooling), 10, 410, 24, BLACK);
            DrawText(TextFormat("Right mouse button - change terrain height (%2.2f)", terrain_height), 10, 440, 24, BLACK);

            /* Section "Image Resolution" */
            int ypos = screen_height - 520;
            DrawText("Image Resolution:", 10, ypos, 38, BLACK);
            DrawText(TextFormat("%dx%d (previewed as 256x256)", hmap->width, hmap->height), 10, ypos + 40, 24, BLACK);

            /* Section "Simulation Parameters" */
            DrawText("Simulation Parameters: ", 10, ypos + 100, 38, BLACK);
            DrawText("# of particles", 10, ypos + 140, 24, BLACK); 
            DrawText(TextFormat("= %d", sim_params->n), 300, ypos + 140, 24, BLACK);
            DrawText("ttl          ", 10, ypos + 170, 24, BLACK); 
            DrawText(TextFormat("= %d", sim_params->ttl), 300, ypos + 170, 24, BLACK);
            DrawText("p_radius     ", 10, ypos + 200, 24, BLACK); 
            DrawText(TextFormat("= %d", sim_params->p_radius), 300, ypos + 200, 24, BLACK);
            DrawText("p_inertia    ", 10, ypos + 230, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_inertia), 300, ypos + 230, 24, BLACK);
            DrawText("p_capacity   ", 10, ypos + 260, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_capacity), 300, ypos + 260, 24, BLACK);
            DrawText("p_gravity    ", 10, ypos + 290, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_gravity), 300, ypos + 290, 24, BLACK);
            DrawText("p_evaporation", 10, ypos + 320, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_evaporation), 300, ypos + 320, 24, BLACK);
            DrawText("p_erosion    ", 10, ypos + 350, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_erosion), 300, ypos + 350, 24, BLACK);
            DrawText("p_deposition ", 10, ypos + 380, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_deposition), 300, ypos + 380, 24, BLACK);
            DrawText("p_min_slope  ", 10, ypos + 410, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_min_slope), 300, ypos + 410, 24, BLACK);
            DrawText("p_initial_velocity  ", 10, ypos + 440, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_initial_velocity), 300, ypos + 440, 24, BLACK);
            DrawText("p_initial_water  ", 10, ypos + 470, 24, BLACK); 
            DrawText(TextFormat("= %f", sim_params->p_initial_water), 300, ypos + 470, 24, BLACK);
        EndDrawing();
    }

    UnloadMesh(hmap_mesh);
    UnloadMaterial(hmap_material);
    UnloadTexture(hmap_texture);

    hgl_chan_send(c, (void *)CMD_EXIT);

    return NULL;
}

