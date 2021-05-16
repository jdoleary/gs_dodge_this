

#define GS_IMPL
#include <gs/gs.h>

void init(){

}
void update(){

}
gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .window_width = 800,
        .window_height = 600,
        .window_title = "Pong"
        .init = init,
        .update = update
    };
}   






