/*================================================================
    * Copyright: 2020 John Jackson
    * immediate_draw

    The purpose of this example is to demonstrate how to use the optional 
    immediate drawing utility, mainly to be used for quick debug rendering.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>
#include <math.h>       // sqrt

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

gs_command_buffer_t  cb   = {0};
gs_immediate_draw_t  gsi  = {0};
gs_asset_font_t      font = {0};
gs_asset_texture_t   tex  = {0};

double duration = 0;
float size = 200;
float speed = 10.f;
gs_vec2 camPos = {0,0};
gs_vec2 heroPos = {0,0};
void init()
{
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new();

    gs_asset_font_load_from_file("./assets/font.ttf", &font, 48);

    gs_graphics_texture_desc_t desc = {0};
    gs_asset_texture_load_from_file("./assets/champ.png", &tex, NULL, true, false);

}
float get_distance(gs_vec2* a, gs_vec2* b){
    f32 xDelta = (a->x-b->x);
    f32 yDelta = (a->y-b->y);
        // gs_println("deltas: %f, %f, %f", xDelta, yDelta, sqrt(xDelta*xDelta + yDelta*yDelta));
    return sqrt(xDelta*xDelta + yDelta*yDelta);
}
void moveToTarget(gs_vec2* self, gs_vec2 target) {
    float speed = gs_interp_linear(0.f,10.f, get_distance(self, &target)/300.f);
    float bigA = target.x - self->x;
    float bigB = target.y - self->y;
    double bigC = sqrt(bigA*bigA + bigB*bigB);
    if(bigC < speed){
        return;
    }
    float a = speed*bigA/bigC;
    float b = speed*bigB/bigC;
    self->x += a;
    self->y += b;
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

	gs_vec2 screenSize = gs_platform_window_sizev(gs_platform_main_window());
    // gs_println("screen size: %f, %f", screenSize.x, screenSize.y);
    camPos = gs_vec2_mul(heroPos, gs_v2(-1,-1));
    camPos = gs_vec2_add(camPos, gs_vec2_div(screenSize, gs_v2(2,2)));
    //     gs_println("camPos: %f, %f", camPos.x, camPos.y);

    // Set up 2D camera for projection matrix
    gsi_camera2D(&gsi);

    const gs_vec2 screenMousePos = gs_platform_mouse_positionv();
    const gs_vec2 objectiveMousePos = gs_vec2_sub(screenMousePos, camPos);
        gs_println("obj mouse pos: %f, %f", objectiveMousePos.x, objectiveMousePos.y);
    // Move hero to pointer
    moveToTarget(&heroPos, objectiveMousePos);
   
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    // Translate everything to inverse of camera to simulate "camera" motion
    gsi_transf(&gsi, camPos.x, camPos.y, 0.f);

    gsi_rectv(&gsi, gs_v2(0.f, 0.f), screenSize, GS_COLOR_GREEN, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_circle(&gsi, heroPos.x, heroPos.y, 50.f, 20, 100, 150, 220, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    // gs_println("hero pos: %f, %f", heroPos.x, heroPos.y);
    gsi_pop_matrix(&gsi);

    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .window_width = 800,
        .window_height = 800,
        .update = update
    };
}   






