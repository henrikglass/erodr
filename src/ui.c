
#include "ui.h"

#include "raylib.h"
#include "rlgl.h"

#define RAYMATH_STATIC_INLINE
#include "raymath.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define PAN_INERTIA 0.8f
#define ZOOM_INERTIA 0.6f
#define ZOOM_DEFAULT 100.0f
#define PLANE_SIZE 64.0f

#define LOW_RES_PREVIEW_MESH_SIZE 256

static float mesh_data[LOW_RES_PREVIEW_MESH_SIZE][LOW_RES_PREVIEW_MESH_SIZE][3];

static float sample_hmap(ErodrImage *hmap, float xf, float yf)
{
    // nearest
    int x = xf * hmap->width;
    int y = yf * hmap->width;
    return hmap->data[y * hmap->width + x];
}

void *ui_run(void *args)
{
    UiArgs *ui_args = (UiArgs *) args;
    ErodrImage *hmap = ui_args->hmap;
    HglChan *c = ui_args->chan;

    int screen_width  = 1600;
    int screen_height =  900;

    float gain = 16.0f;

    /* Window */
    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screen_width, screen_height, "erodr UI");

    /* Camera */
    Camera camera     = { 0 };
    camera.position   = Vector3Scale(Vector3Normalize((Vector3){ 18.0f, 21.0f, 18.0f }), ZOOM_DEFAULT);
    camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };    // Camera up vector (rotation towards target)
    camera.fovy       = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;               // Camera projection type

    /* Heightmap mesh/model */
    Mesh      hmap_mesh     = GenMeshPlane(64.0f, 64.0f, LOW_RES_PREVIEW_MESH_SIZE - 1, 
                                                         LOW_RES_PREVIEW_MESH_SIZE - 1);
    assert(hmap_mesh.vertexCount <= 0x10000); // Raylib only supports short int indicies.. :/
    memcpy(mesh_data, hmap_mesh.vertices, sizeof(mesh_data));
    //printf("vertex count = %d\n", hmap_mesh.vertexCount);
    printf("%d\n", hmap_mesh.vertexCount);
    Matrix    hmap_tform    = MatrixTranslate(0.0f, 0.10f, 0.0f);
    Material  hmap_material = LoadMaterialDefault();
    hmap_material.shader    = LoadShader(0, "src/shaders/hmap_shader.frag");

    Image hmap_image = (Image) {
        .data = hmap->data,
        .width = hmap->width,
        .height = hmap->height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32,
    };

    Texture2D hmap_texture  = LoadTextureFromImage(hmap_image);
    GenTextureMipmaps(&hmap_texture);
    SetTextureFilter(hmap_texture, TEXTURE_FILTER_TRILINEAR);
    SetMaterialTexture(&hmap_material, MATERIAL_MAP_ALBEDO, hmap_texture);


    /* Camera controls */
    Vector2 mouse = Vector2Zero();
    float zoom = 0.0f;

    while (!WindowShouldClose()) {
        /* ====== update ================================ */

        /* handle resizing */
        if (IsWindowResized() && !IsWindowFullscreen()) {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
        }

        /* toggle gain setting */
        if (IsKeyPressed(KEY_G)) {
            if (gain == 0.0f) {
                gain = 0.5f;
            } else {
                gain = 2*gain;
                if (gain > 100) gain = 0.0f;
            }
        }

        /* reset heightmap */
        if (IsKeyPressed(KEY_R)) {
            hgl_chan_send(c, (void *)CMD_RESET_HMAP);
        }

        /* re-run simulation */
        if (IsKeyPressed(KEY_ENTER)) {
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

        /* adjust camera target/position based on gain setting */
        camera.target.y = hmap_tform.m13 + gain/4.0f;

        /* camera movement */
        Vector2 mouse_delta = GetMouseDelta();
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

        /* update mesh */
        for (int y = 0; y < LOW_RES_PREVIEW_MESH_SIZE; y++) {
            for (int x = 0; x < LOW_RES_PREVIEW_MESH_SIZE; x++) {
                float xf = (float)x / (float)LOW_RES_PREVIEW_MESH_SIZE;
                float yf = (float)y / (float)LOW_RES_PREVIEW_MESH_SIZE;
                mesh_data[y][x][1] = gain * sample_hmap(hmap, xf, yf);
                //(void) hmap;
                //(void) xf;
                //(void) yf;
                //mesh_data[y][x][1] = 1.0f;
            }
        }
        UpdateMeshBuffer(hmap_mesh, 0, mesh_data, sizeof(mesh_data), 0);
        UpdateTexture(hmap_texture, hmap->data);

        /* ====== draw ================================== */

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                //rlDisableDepthTest();
                //DrawGrid(100, 1.0f);
                DrawMesh(hmap_mesh, hmap_material, hmap_tform);
            EndMode3D();
            DrawFPS(screen_width - 100, 10);
            DrawText("Commands: ", 10, 10, 30, BLACK);
            DrawText(TextFormat("g - gain (%2.2f)", gain), 10, 50, 24, BLACK);
            DrawText(TextFormat("p - projection mode (%s)", (camera.projection == CAMERA_PERSPECTIVE) ? 
                                "perspective" : "orthographic"), 10, 80, 24, BLACK);
        EndDrawing();
    }

    UnloadMesh(hmap_mesh);
    UnloadMaterial(hmap_material);
    UnloadTexture(hmap_texture);

    hgl_chan_send(c, (void *)CMD_EXIT);

    return NULL;
}

