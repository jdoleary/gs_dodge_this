

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GAME_FIELDX 10.f
#define GAME_FIELDY 10.f

#define window_size(...) gs_platform_window_sizev(gs_platform_main_window())

typedef struct game_data_t 
{
    gs_command_buffer_t gcb;
    gs_immediate_draw_t gsi;
} game_data_t;
void my_init(){
    // Grab user data pointer from framework
    game_data_t* gd = gs_engine_user_data(game_data_t);

    // Initialize command buffer
    // THIS LINE CAUSES CRASH
    gd->gcb = gs_command_buffer_new();
    // gd->gsi = gs_immediate_draw_new();

}
// void draw_game(game_data_t* gd){
//     // Cache pointers
//     gs_command_buffer_t* gcb = &gd->gcb;
//     gs_immediate_draw_t* gsi = &gd->gsi;

//     // Window size
//     gs_vec2 ws = window_size();
    
//     // 2D Camera (for screen coords)
//     gsi_camera2D(gsi);

//     // Game Field
//     gsi_rect(gsi, GAME_FIELDX, GAME_FIELDY, ws.x - GAME_FIELDX, ws.y - GAME_FIELDY,
//     255,255,255,255, GS_GRAPHICS_PRIMITIVE_LINES);

//     // Final immediate draw submit and render pass
//     gsi_render_pass_submit(gsi, gcb, gs_color(20,20,20,255));
//     // Final graphics backend command buffer submit
//     gs_graphics_submit_command_buffer(gcb);

// }
void my_update(){}
//     // Close game on ESC press
//     if(gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();
//     game_data_t* gd = gs_engine_user_data(game_data_t);
//     draw_game(gd);

// }
void my_shutdown(){
    
}
// Globals
game_data_t gdata = {0};
gs_app_desc_t gs_main(int32_t argc, char** argv) {
    return (gs_app_desc_t){
        // .window_width = 800,
        // .window_height = 600,
        // .window_title = "Pong",
        .init = my_init,
        .update = my_update,
        .shutdown = my_shutdown
    };
}   






