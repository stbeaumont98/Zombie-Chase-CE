#ifndef b_frame_include_file
#define b_frame_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define b_frame_width 15
#define b_frame_height 15
#define b_frame_size 227
#define b_frame ((gfx_sprite_t*)b_frame_data)
extern unsigned char b_frame_data[227];

#ifdef __cplusplus
}
#endif

#endif
