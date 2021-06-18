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
float speed = 10.f;
f32 unit_size = 50.f;
int score = 0;

enum gameStates{Pre, Playing} gameState;
gameState = Pre;

#define arena_size 1000
gs_vec2 arena_upper_left = {-arena_size, -arena_size};
gs_vec2 arena_lower_right = {arena_size,arena_size};

typedef struct {
    gs_vec2 pos;
    small type;
    gs_vec2 target;
    gs_vec2 vel;
} Unit;

gs_dyn_array(Unit) enemies = NULL;
Unit hero;
#define HERO_MAX_HEALTH 10
int hero_health = HERO_MAX_HEALTH;

float lerp(float t, float a, float b ){
    return (1-t)*a + t*b;
}
gs_vec2 camPos = {0,0};
gs_vec2 get_point_between_bounds(){
    float x = lerp(stb_frand(), arena_upper_left.x, arena_lower_right.x);
    float y = lerp(stb_frand(), arena_upper_left.y, arena_lower_right.y);
    return gs_vec2_ctor(x,y);

}
makeUnit(short t){
    f32 spawn_radius = 1000.f;
    Unit enemy;
    enemy.pos = get_point_between_bounds();
    enemy.type = t;
    enemy.target = enemy.pos;
    enemy.vel = gs_vec2_ctor(0,0);

    gs_dyn_array_push(enemies, enemy);
}

void init()
{
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new();

    gs_asset_font_load_from_file("./assets/font.ttf", &font, 48);

    // gs_graphics_texture_desc_t desc = {0};
    // gs_asset_texture_load_from_file("./assets/champ.png", &tex, NULL, true, false);
    // Init hero
    hero.pos = gs_vec2_ctor(0,0);
    hero.type = 0;
    hero.target = gs_vec2_ctor(0,0);
    hero.vel = gs_vec2_ctor(0,0);
    

    // Init enemies
    int number_of_wanderers = 10;
    int number_of_chasers = 4;
    int number_of_fleers = 7;
    int number_of_mimics = 1;
    for(int i = 0; i < number_of_wanderers; i++){
        makeUnit(1);
    }
    for(int i = 0; i < number_of_chasers; i++){
        makeUnit(2);
    }
    for(int i = 0; i < number_of_fleers; i++){
        makeUnit(3);
    }
    for(int i = 0; i < number_of_mimics; i++){
        makeUnit(4);
    }

}
float get_distance(gs_vec2 a, gs_vec2 b){
    f32 xDelta = (a.x-b.x);
    f32 yDelta = (a.y-b.y);
    return sqrt(xDelta*xDelta + yDelta*yDelta);
}
f32 bounce_velocity = 20.f;
void add_velocity_away(Unit* forUnit, gs_vec2 from){
    float bigA = from.x - forUnit->pos.x;
    float bigB = from.y - forUnit->pos.y;
    double bigC = sqrt(bigA*bigA + bigB*bigB);
    float a = bigA/bigC;
    float b = bigB/bigC;
    forUnit->vel.x -= a*bounce_velocity;
    forUnit->vel.y -= b*bounce_velocity;

}
void doArenaBorderCollisions(Unit* unit){
    if(unit->pos.x > arena_size){
        unit->pos.x = arena_size;
    } 
    if(unit->pos.x < -arena_size){
        unit->pos.x = -arena_size;
    }
    if(unit->pos.y > arena_size){
        unit->pos.y = arena_size;
    } 
    if(unit->pos.y < -arena_size){
        unit->pos.y = -arena_size;
    }
}
// Reduces velocity per tick
f32 drag = 0.94f;
void use_velocity(Unit* unit){
    unit->pos.x += unit->vel.x;
    unit->pos.y += unit->vel.y;

    unit->vel.x *= drag;
    unit->vel.y *= drag;

}
bool areUnitsColliding(Unit* a, Unit* b){
    if(get_distance(a->pos, b->pos) <= unit_size*2){
        return true;
    }
    return false;

}
void moveToTargetDynamicSpeed(gs_vec2* self, gs_vec2 target) {
    float speed = gs_interp_linear(0.f,20.f, get_distance(*self, target)/300.f);
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
int agro_radius = 300;
void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();



	gs_vec2 screenSize = gs_platform_window_sizev(gs_platform_main_window());

    camPos = gs_vec2_mul(hero.pos, gs_v2(-1,-1));
    camPos = gs_vec2_add(camPos, gs_vec2_div(screenSize, gs_v2(2,2)));

    // Set up 2D camera for projection matrix
    gsi_camera2D(&gsi);

    // Draw grid in objective space so player knows how much they're moving
    int skipAmount = 200;
    for(int i = 0; i <= (int)screenSize.x; i += skipAmount){
        float x = (int)camPos.x % skipAmount + i;
        float y = (int)camPos.y % skipAmount + i;
        // Only draw grid within the arena
        if(
            -camPos.x + x < arena_size
            && -camPos.x + x > -arena_size
        ){
            gsi_linev(&gsi, gs_v2(x, 0.f), gs_v2(x, screenSize.y), gs_color(0, 255, 0, 255));
        }
        if(
            -camPos.y + y < arena_size
            && -camPos.y + y > -arena_size
        ){
            gsi_linev(&gsi, gs_v2(0.f, y), gs_v2(screenSize.x, y), gs_color(0, 255, 0, 255));
        }
    }

    const gs_vec2 screenMousePos = gs_platform_mouse_positionv();
    const gs_vec2 objectiveMousePos = gs_vec2_sub(screenMousePos, camPos);
    if(gameState == Pre){
        // When user hovers mouse over hero, start the game
        if(get_distance(objectiveMousePos, hero.pos) <= unit_size){
            gameState = Playing;
        }
    }

   
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    // Translate everything to inverse of camera to simulate "camera" motion
    gsi_transf(&gsi, camPos.x, camPos.y, 0.f);


    // Draw arena
    gsi_rectv(&gsi, arena_upper_left, arena_lower_right, GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_LINES);


    // Draw enemies
    for (uint32_t i = 0; i < gs_dyn_array_size(enemies); ++i) {     // Iterate size of array, access elements via index `i`
        Unit* e = &enemies[i];
        short r = 0;
        short g = 0;
        short b = 0;
        int enemySpeed = 2;
        switch(e->type) {
            case 1:
            // Wanderer
            r = 255;
            g = 150;
            break;
            case 2:
            // Chaser
            r = 255;
            g = 100;
            enemySpeed = 2;
            if(get_distance(e->pos, hero.pos) < agro_radius){
                e->target = hero.pos;
                g = 0;
                enemySpeed = 4;
            }
            break;
            case 3:
            // Fleer
            b = 255;
            if(get_distance(e->pos, hero.pos) < agro_radius){
                // Run away from hero
                e->target = gs_vec2_sub(e->pos, gs_vec2_sub(hero.pos, e->pos));
                b = 255;
                g = 100;
                r = 100;
                // Run away faster
                enemySpeed = 4;
            }
            break;
            case 4:
            // Mimic
            b = 255;
            if(get_distance(e->pos, hero.pos) < agro_radius){
                // Chase hero
                e->target = hero.pos;
                // Turn red
                r = 255;
                b = 255;
                enemySpeed = 4;
            }
            break;
        }

        if(gameState == Playing){
            bool reachedTarget = moveToTarget(&e->pos, e->target, enemySpeed);
            if(reachedTarget){
                // Pick random wander target:
                e->target = get_point_between_bounds();
            }
            // Test collisions with hero:
            if(areUnitsColliding(e, &hero)){
                if(e->type == 3){
                    // Fleer
                    hero_health += 1;
                    // Gain score
                    score++;
                    // Cap health at no greater than max
                    if(hero_health >= HERO_MAX_HEALTH){
                        hero_health = HERO_MAX_HEALTH;
                    }
                    // "Respawn" fleer:
                    e->pos = get_point_between_bounds();
                }else {
                    hero_health -= 1;
                    add_velocity_away(&hero, e->pos);
                    add_velocity_away(e, hero.pos);

                }
            }
            use_velocity(e);
        }

        gsi_circle(&gsi, e->pos.x, e->pos.y, unit_size, 20, r, g, b, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }

    // HERO
    // Move hero to pointer
    if(gameState == Playing){
        moveToTargetDynamicSpeed(&hero.pos, objectiveMousePos);
        doArenaBorderCollisions(&hero);
        use_velocity(&hero);
    }
    // Draw hero
    gsi_circle(&gsi, hero.pos.x, hero.pos.y, unit_size+5, 20, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_circle(&gsi, hero.pos.x, hero.pos.y, unit_size, 20, 0, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_circle(&gsi, hero.pos.x, hero.pos.y, unit_size*hero_health/HERO_MAX_HEALTH, 20, 0, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    // gs_println("hero pos: %f, %f", heroPos.x, heroPos.y);

    gsi_pop_matrix(&gsi);

    if(gameState == Pre){
            // Print Instructions 
            {
                gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
                gsi_scalef(&gsi, 2.f, 2.f, 1.f);

                gsi_text(&gsi, 10.f, 20.f, "Hover over the green circle to begin", NULL, false, 255, 255, 255, 255);

                gsi_pop_matrix(&gsi);
            }

    }else {

        // Print score to screen
        {
            gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
            gsi_scalef(&gsi, 2.f, 2.f, 2.f);

            char buf[12];
            snprintf(buf, 12, "Score: %d", score);
            gsi_text(&gsi, 10.f, 20.f, buf, NULL, false, 255, 255, 255, 255);

            gsi_pop_matrix(&gsi);
        }
    }

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






