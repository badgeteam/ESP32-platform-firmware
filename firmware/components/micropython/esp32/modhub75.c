#include <sdkconfig.h>

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#include <string.h>

#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "lib/utils/pyexec.h"

#include "extmod/vfs_native.h"

#include <compositor.h>
#include <driver_hub75.h>

#define TAG "esp32/hub75"

STATIC mp_obj_t hub75_background(mp_obj_t r_obj, mp_obj_t g_obj, mp_obj_t b_obj) {
    uint8_t r = mp_obj_get_int(r_obj);
    uint8_t g = mp_obj_get_int(g_obj);
    uint8_t b = mp_obj_get_int(b_obj);
    Color k;
    k.RGB[3] = r;
    k.RGB[2] = g;
    k.RGB[1] = b;
    k.RGB[0] = 255; // alpha
    compositor_setBackground(k);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(hub75_background_obj, hub75_background);

STATIC mp_obj_t hub75_brightness(mp_obj_t bright_obj) {
    int brightness = mp_obj_get_int(bright_obj);

    driver_hub75_set_brightness(brightness);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(hub75_brightness_obj, hub75_brightness);

STATIC mp_obj_t hub75_framerate(mp_obj_t bright_obj) {
    int framerate = mp_obj_get_int(bright_obj);

    driver_hub75_set_framerate(framerate);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(hub75_framerate_obj, hub75_framerate);

STATIC mp_obj_t hub75_clear() {
    compositor_clear();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(hub75_clear_obj, hub75_clear);

STATIC mp_obj_t hub75_disablecomp() {
    compositor_disable();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(hub75_disablecomp_obj, hub75_disablecomp);

STATIC mp_obj_t hub75_enablecomp() {
    compositor_enable();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(hub75_enablecomp_obj, hub75_enablecomp);

STATIC mp_obj_t hub75_text(size_t n_args, const mp_obj_t *args) {
    char *test = (char*)mp_obj_str_get_str(args[0]);

    uint8_t r = mp_obj_get_int(args[1]);
    uint8_t g = mp_obj_get_int(args[2]);
    uint8_t b = mp_obj_get_int(args[3]);
    uint8_t a = mp_obj_get_int(args[4]);
    Color k;
    k.RGB[3] = r;
    k.RGB[2] = g;
    k.RGB[1] = b;
    k.RGB[0] = a;

    int x = mp_obj_get_int(args[5]);
    int y = mp_obj_get_int(args[6]);

    compositor_addText(test, k, x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hub75_text_obj, 7, 7, hub75_text);


STATIC mp_obj_t hub75_scrolltext(size_t n_args, const mp_obj_t *args) {
     char *text = (char*)mp_obj_str_get_str(args[0]);

    uint8_t r = mp_obj_get_int(args[1]);
    uint8_t g = mp_obj_get_int(args[2]);
    uint8_t b = mp_obj_get_int(args[3]);
    uint8_t a = mp_obj_get_int(args[4]);
    Color k;
    k.RGB[3] = r;
    k.RGB[2] = g;
    k.RGB[1] = b;
    k.RGB[0] = a;

    int x = mp_obj_get_int(args[5]);
    int y = mp_obj_get_int(args[6]);
    int sizex = mp_obj_get_int(args[7]);

    compositor_addScrollText(text, k, x, y, sizex);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hub75_scrolltext_obj, 8, 8, hub75_scrolltext);

STATIC mp_obj_t hub75_image(size_t n_args, const mp_obj_t *args) {
    mp_obj_t *mp_arr;
    size_t len;
    mp_obj_get_array(args[0], &len, &mp_arr);

    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int width = mp_obj_get_int(args[3]);
    int height = mp_obj_get_int(args[4]);

    if(len != (width*height)) {
        mp_raise_ValueError("Array not the right size");
        return mp_const_none;
    }

    uint32_t *image = malloc(width*height*4);

    for(int i = 0; i < width*height; i++) {
        image[i] = mp_obj_get_int64(mp_arr[i]);
    }

    compositor_addImage((uint8_t *) image, x, y, width, height);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hub75_image_obj, 5, 5, hub75_image);

STATIC mp_obj_t hub75_pixel(size_t n_args, const mp_obj_t *args) {
     int r = mp_obj_get_int(args[0]);
     int g = mp_obj_get_int(args[1]);
     int b = mp_obj_get_int(args[2]);
     int a = mp_obj_get_int(args[3]);
     int x = mp_obj_get_int(args[4]);
     int y = mp_obj_get_int(args[5]);

    uint32_t *image = malloc(4);

     Color k;
     k.RGB[3] = r;
     k.RGB[2] = g;
     k.RGB[1] = b;
     k.RGB[0] = a;

     image[0] = k.value;

     // We call addImage instead of setPixel here because setPixel
     // gets overridden by the compositor's render function.
     compositor_addImage((uint8_t *) image, x, y, 1, 1);

     return mp_const_none;
 }
 STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hub75_pixel_obj, 6, 6, hub75_pixel);


STATIC mp_obj_t hub75_gif(size_t n_args, const mp_obj_t *args) {
    mp_obj_t *mp_arr;
    size_t len;
    mp_obj_get_array(args[0], &len, &mp_arr);

    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int width = mp_obj_get_int(args[3]);
    int height = mp_obj_get_int(args[4]);
    int numframes = mp_obj_get_int(args[5]);

    if(len != (width*height*numframes)) {
        mp_raise_ValueError("Array not the right size");
        return mp_const_none;
    }

    uint32_t *image = malloc(width*height*4*numframes);

    for(int i = 0; i < width*height*numframes; i++) {
        image[i] = mp_obj_get_int64(mp_arr[i]);
    }

    compositor_addAnimation((uint8_t *) image, x, y, width, height, numframes);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hub75_gif_obj, 6, 6, hub75_gif);

STATIC mp_obj_t hub75_frame(mp_obj_t arr_obj) {
  mp_obj_t *mp_arr;
  size_t len;
  mp_obj_get_array(arr_obj, &len, &mp_arr);
  if(len != (CONFIG_HUB75_WIDTH*CONFIG_HUB75_HEIGHT)) {
      mp_raise_ValueError("Array not the right size");
      return mp_const_none;
  }

  Color* frame = getFrameBuffer();
  for(int i = 0; i < CONFIG_HUB75_WIDTH*CONFIG_HUB75_HEIGHT; i++) {
      Color k;
      k.value = mp_obj_get_int64(mp_arr[i]);
      frame[i] = k;
  }

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(hub75_frame_obj, hub75_frame);

STATIC mp_obj_t hub75_textwidth(mp_obj_t text_obj) {
  char *text = (char*)mp_obj_str_get_str(text_obj);
  unsigned int width = compositor_getTextWidth(text);
  return mp_obj_new_int(width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(hub75_textwidth_obj, hub75_textwidth);

STATIC mp_obj_t hub75_setfont(mp_obj_t index_obj) {
  int index = mp_obj_get_int(index_obj);
  compositor_setFont(index);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(hub75_setfont_obj, hub75_setfont);

STATIC const mp_rom_map_elem_t hub75_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_hub75)},
    {MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&hub75_text_obj)},
    {MP_ROM_QSTR(MP_QSTR_scrolltext), MP_ROM_PTR(&hub75_scrolltext_obj)},
    {MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&hub75_clear_obj)},
    {MP_ROM_QSTR(MP_QSTR_brightness), MP_ROM_PTR(&hub75_brightness_obj)},
    {MP_ROM_QSTR(MP_QSTR_framerate), MP_ROM_PTR(&hub75_framerate_obj)},
    {MP_ROM_QSTR(MP_QSTR_disablecomp), MP_ROM_PTR(&hub75_disablecomp_obj)},
    {MP_ROM_QSTR(MP_QSTR_enablecomp), MP_ROM_PTR(&hub75_enablecomp_obj)},
    {MP_ROM_QSTR(MP_QSTR_background), MP_ROM_PTR(&hub75_background_obj)},
    {MP_ROM_QSTR(MP_QSTR_frame), MP_ROM_PTR(&hub75_frame_obj)},
    {MP_ROM_QSTR(MP_QSTR_gif), MP_ROM_PTR(&hub75_gif_obj)},
    {MP_ROM_QSTR(MP_QSTR_image), MP_ROM_PTR(&hub75_image_obj)},
    {MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&hub75_pixel_obj)},
    {MP_ROM_QSTR(MP_QSTR_textwidth), MP_ROM_PTR(&hub75_textwidth_obj)},
    {MP_ROM_QSTR(MP_QSTR_setfont), MP_ROM_PTR(&hub75_setfont_obj)},
};

STATIC MP_DEFINE_CONST_DICT(hub75_module_globals, hub75_module_globals_table);

const mp_obj_module_t hub75_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&hub75_module_globals,
};

#endif
