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

typedef struct {
    gs_vec2 pos;
    small type;
    gs_vec2 target;
} Enemy;

gs_dyn_array(Enemy) enemies = NULL;
gs_vec2 camPos = {0,0};
gs_vec2 heroPos = {0,0};
gs_vec2 get_point_within_distance(gs_vec2 origin, int distance){
    gs_vec2 point = gs_vec2_ctor(stb_frand()*distance*2-distance,stb_frand()*distance*2-distance);
    return gs_vec2_add(origin, point);
}
makeEnemy(short t){
    f32 spawn_radius = 1000.f;
        Enemy enemy;
        enemy.pos = gs_vec2_ctor(stb_frand()*spawn_radius,stb_frand()*spawn_radius);
        enemy.type = t;
        enemy.target = enemy.pos;

        gs_dyn_array_push(enemies, enemy);
}

void init()
{
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new();

    gs_asset_font_load_from_file("./assets/font.ttf", &font, 48);

    // gs_graphics_texture_desc_t desc = {0};
    // gs_asset_texture_load_from_file("./assets/champ.png", &tex, NULL, true, false);

    // Init enemies
    int number_of_enemies = 10;
    for(int i = 0; i < number_of_enemies; i++){
        makeEnemy(1);
    }
    for(int i = 0; i < number_of_enemies; i++){
        makeEnemy(2);
    }
    for(int i = 0; i < number_of_enemies; i++){
        makeEnemy(3);
    }

}
float get_distance(gs_vec2* a, gs_vec2* b){
    f32 xDelta = (a->x-b->x);
    f32 yDelta = (a->y-b->y);
    return sqrt(xDelta*xDelta + yDelta*yDelta);
}
void moveToTargetDynamicSpeed(gs_vec2* self, gs_vec2 target) {
    float speed = gs_interp_linear(0.f,20.f, get_distance(self, &target)/300.f);
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
bool moveToTarget(gs_vec2* self, gs_vec2 target, int speed) {
    float bigA = target.x - self->x;
    float bigB = target.y - self->y;
    double bigC = sqrt(bigA*bigA + bigB*bigB);
    if(bigC < speed){
        // Reached target
        return true;
    }
    float a = speed*bigA/bigC;
    float b = speed*bigB/bigC;
    self->x += a;
    self->y += b;
    // Haven't yet reached target
    return false;
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

	gs_vec2 screenSize = gs_platform_window_sizev(gs_platform_main_window());

    camPos = gs_vec2_mul(heroPos, gs_v2(-1,-1));
    camPos = gs_vec2_add(camPos, gs_vec2_div(screenSize, gs_v2(2,2)));

    // Set up 2D camera for projection matrix
    gsi_camera2D(&gsi);

    // Draw grid in objective space so player knows how much they're moving
    int skipAmount = 200;
    for(int i = 0; i <= (int)screenSize.x; i += skipAmount){
        float x = (int)camPos.x % skipAmount + i;
        float y = (int)camPos.y % skipAmount + i;
        gsi_linev(&gsi, gs_v2(x, 0.f), gs_v2(x, screenSize.y), gs_color(0, 255, 0, 255));
        gsi_linev(&gsi, gs_v2(0.f, y), gs_v2(screenSize.x, y), gs_color(0, 255, 0, 255));
    }

    const gs_vec2 screenMousePos = gs_platform_mouse_positionv();
    const gs_vec2 objectiveMousePos = gs_vec2_sub(screenMousePos, camPos);

   
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    // Translate everything to inverse of camera to simulate "camera" motion
    gsi_transf(&gsi, camPos.x, camPos.y, 0.f);

    gsi_rectv(&gsi, gs_v2(0.f, 0.f), screenSize, GS_COLOR_RED, GS_GRAPHICS_PRIMITIVE_LINES);


    // Draw enemies
    for (uint32_t i = 0; i < gs_dyn_array_size(enemies); ++i) {     // Iterate size of array, access elements via index `i`
        Enemy* e = &enemies[i];
        short r = 0;
        short g = 0;
        short b = 0;
        int speed = 2;
        switch(e->type) {
            case 1:
            // Wanderer
            r = 255;
            g = 150;
            break;
            case 2:
            // Chaser
            r = 255;
            e->target = heroPos;
            speed = 2;
            break;
            case 3:
            // Fleer
            b = 255;
            break;
        }
        bool reachedTarget = moveToTarget(&e->pos, e->target, speed);
        if(reachedTarget){
            if(e->type == 1){
                // Pick random wander target:
                e->target = get_point_within_distance(e->pos, 1000);
            gs_println("wander pos: %f, %f", e->target.x, e->target.y);
            }
        }
        gsi_circle(&gsi, e->pos.x, e->pos.y, 50.f, 20, r, g, b, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }

// HERO
    // Move hero to pointer
    moveToTargetDynamicSpeed(&heroPos, objectiveMousePos);
    // Draw hero
    gsi_circle(&gsi, heroPos.x, heroPos.y, 50.f, 20, 0, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
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






