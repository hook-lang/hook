//
// raylib_mod.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "raylib_mod.h"
#include <raylib.h>

#define MAX_IMAGES           32
#define MAX_SHADER_LOCATIONS 32

static HkString *xStr;
static HkString *yStr;
static HkString *zStr;
static HkString *wStr;
static HkString *rStr;
static HkString *gStr;
static HkString *bStr;
static HkString *aStr;
static HkString *m0Str;
static HkString *m1Str;
static HkString *m2Str;
static HkString *m3Str;
static HkString *m4Str;
static HkString *m5Str;
static HkString *m6Str;
static HkString *m7Str;
static HkString *m8Str;
static HkString *m9Str;
static HkString *m10Str;
static HkString *m11Str;
static HkString *m12Str;
static HkString *m13Str;
static HkString *m14Str;
static HkString *m15Str;
static HkString *widthStr;
static HkString *heightStr;
static HkString *dataStr;
static HkString *mipmapsStr;
static HkString *formatStr;
static HkString *idStr;
static HkString *textureStr;
static HkString *depthStr;
static HkString *offsetStr;
static HkString *targetStr;
static HkString *rotationStr;
static HkString *zoomStr;
static HkString *positionStr;
static HkString *upStr;
static HkString *fovyStr;
static HkString *projectionStr;
static HkString *locsStr;
static HkStruct *vector2Struct;
static HkStruct *vector3Struct;
static HkStruct *vector4Struct;
static HkStruct *matrixStruct;
static HkStruct *colorStruct;
static HkStruct *rectangleStruct;
static HkStruct *imageStruct;
static HkStruct *textureStruct;
static HkStruct *renderTextureStruct;
static HkStruct *camera2DStruct;
static HkStruct *camera3DStruct;
static HkStruct *shaderStruct;
static inline void init(void);
static inline Vector2 vector2_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_vector2(Vector2 vector);
static inline Vector3 vector3_from_instance(HkInstance *inst);
static inline Color color_from_instance(HkInstance *inst);
static inline Rectangle rectangle_from_instance(HkInstance *inst);
static inline Image image_from_instance(HkInstance *inst);
static inline void images_from_array(HkArray *arr, Image *images);
static inline Texture texture_from_instance(HkInstance *inst);
static inline RenderTexture render_texture_from_instance(HkInstance *inst);
static inline Camera2D camera2d_from_instance(HkInstance *inst);
static inline Camera3D camera3d_from_instance(HkInstance *inst);
static inline void shader_locations_from_array(HkArray *arr, int *locs);
static inline Shader shader_from_instance(HkInstance *inst);
static inline void load_vector2_struct(HkVM *vm);
static inline void load_vector3_struct(HkVM *vm);
static inline void load_vector4_struct(HkVM *vm);
static inline void load_matrix_struct(HkVM *vm);
static inline void load_color_struct(HkVM *vm);
static inline void load_rectangle_struct(HkVM *vm);
static inline void load_image_struct(HkVM *vm);
static inline void load_texture_struct(HkVM *vm);
static inline void load_render_texture_struct(HkVM *vm);
static inline void load_camera2D_struct(HkVM *vm);
static inline void load_camera3D_struct(HkVM *vm);
static inline void load_shader_struct(HkVM *vm);
static inline void load_colors(HkVM *vm);
static inline void load_functions(HkVM *vm);

// Window-related functions
static void InitWindow_call(HkVM *vm, HkValue *args);
static void CloseWindow_call(HkVM *vm, HkValue *args);
static void WindowShouldClose_call(HkVM *vm, HkValue *args);
static void IsWindowReady_call(HkVM *vm, HkValue *args);
static void IsWindowFullscreen_call(HkVM *vm, HkValue *args);
static void IsWindowHidden_call(HkVM *vm, HkValue *args);
static void IsWindowMinimized_call(HkVM *vm, HkValue *args);
static void IsWindowMaximized_call(HkVM *vm, HkValue *args);
static void IsWindowFocused_call(HkVM *vm, HkValue *args);
static void IsWindowResized_call(HkVM *vm, HkValue *args);
static void IsWindowState_call(HkVM *vm, HkValue *args);
static void SetWindowState_call(HkVM *vm, HkValue *args);
static void ClearWindowState_call(HkVM *vm, HkValue *args);
static void ToggleFullscreen_call(HkVM *vm, HkValue *args);
static void ToggleBorderlessWindowed_call(HkVM *vm, HkValue *args);
static void MaximizeWindow_call(HkVM *vm, HkValue *args);
static void MinimizeWindow_call(HkVM *vm, HkValue *args);
static void RestoreWindow_call(HkVM *vm, HkValue *args);
static void SetWindowIcon_call(HkVM *vm, HkValue *args);
static void SetWindowIcons_call(HkVM *vm, HkValue *args);
static void SetWindowTitle_call(HkVM *vm, HkValue *args);
static void SetWindowPosition_call(HkVM *vm, HkValue *args);
static void SetWindowMonitor_call(HkVM *vm, HkValue *args);
static void SetWindowMinSize_call(HkVM *vm, HkValue *args);
static void SetWindowMaxSize_call(HkVM *vm, HkValue *args);
static void SetWindowSize_call(HkVM *vm, HkValue *args);
static void SetWindowOpacity_call(HkVM *vm, HkValue *args);
static void SetWindowFocused_call(HkVM *vm, HkValue *args);
static void GetScreenWidth_call(HkVM *vm, HkValue *args);
static void GetScreenHeight_call(HkVM *vm, HkValue *args);
static void GetRenderWidth_call(HkVM *vm, HkValue *args);
static void GetRenderHeight_call(HkVM *vm, HkValue *args);
static void GetMonitorCount_call(HkVM *vm, HkValue *args);
static void GetCurrentMonitor_call(HkVM *vm, HkValue *args);
static void GetMonitorPosition_call(HkVM *vm, HkValue *args);
static void GetMonitorWidth_call(HkVM *vm, HkValue *args);
static void GetMonitorHeight_call(HkVM *vm, HkValue *args);
static void GetMonitorPhysicalWidth_call(HkVM *vm, HkValue *args);
static void GetMonitorPhysicalHeight_call(HkVM *vm, HkValue *args);
static void GetMonitorRefreshRate_call(HkVM *vm, HkValue *args);
static void GetWindowPosition_call(HkVM *vm, HkValue *args);
static void GetWindowScaleDPI_call(HkVM *vm, HkValue *args);
static void GetMonitorName_call(HkVM *vm, HkValue *args);
static void SetClipboardText_call(HkVM *vm, HkValue *args);
static void GetClipboardText_call(HkVM *vm, HkValue *args);
static void EnableEventWaiting_call(HkVM *vm, HkValue *args);
static void DisableEventWaiting_call(HkVM *vm, HkValue *args);
// Cursor-related functions
static void ShowCursor_call(HkVM *vm, HkValue *args);
static void HideCursor_call(HkVM *vm, HkValue *args);
static void IsCursorHidden_call(HkVM *vm, HkValue *args);
static void EnableCursor_call(HkVM *vm, HkValue *args);
static void DisableCursor_call(HkVM *vm, HkValue *args);
static void IsCursorOnScreen_call(HkVM *vm, HkValue *args);
// Drawing-related functions
static void ClearBackground_call(HkVM *vm, HkValue *args);
static void BeginDrawing_call(HkVM *vm, HkValue *args);
static void EndDrawing_call(HkVM *vm, HkValue *args);
static void BeginMode2D_call(HkVM *vm, HkValue *args);
static void EndMode2D_call(HkVM *vm, HkValue *args);
static void BeginMode3D_call(HkVM *vm, HkValue *args);
static void EndMode3D_call(HkVM *vm, HkValue *args);
static void BeginTextureMode_call(HkVM *vm, HkValue *args);
static void EndTextureMode_call(HkVM *vm, HkValue *args);
static void BeginShaderMode_call(HkVM *vm, HkValue *args);
static void EndShaderMode_call(HkVM *vm, HkValue *args);
static void BeginBlendMode_call(HkVM *vm, HkValue *args);
static void EndBlendMode_call(HkVM *vm, HkValue *args);
static void BeginScissorMode_call(HkVM *vm, HkValue *args);
static void EndScissorMode_call(HkVM *vm, HkValue *args);
// Timing-related functions
static void SetTargetFPS_call(HkVM *vm, HkValue *args);
static void GetFrameTime_call(HkVM *vm, HkValue *args);
static void GetTime_call(HkVM *vm, HkValue *args);
static void GetFPS_call(HkVM *vm, HkValue *args);
// Input-related functions: keyboard
static void IsKeyPressed_call(HkVM *vm, HkValue *args);
static void IsKeyPressedRepeat_call(HkVM *vm, HkValue *args);
static void IsKeyDown_call(HkVM *vm, HkValue *args);
static void IsKeyReleased_call(HkVM *vm, HkValue *args);
static void IsKeyUp_call(HkVM *vm, HkValue *args);
static void GetKeyPressed_call(HkVM *vm, HkValue *args);
static void GetCharPressed_call(HkVM *vm, HkValue *args);
static void SetExitKey_call(HkVM *vm, HkValue *args);
// Basic shapes drawing functions
static void DrawPixel_call(HkVM *vm, HkValue *args);
static void DrawPixelV_call(HkVM *vm, HkValue*args);
static void DrawLine_call(HkVM *vm, HkValue*args);
static void DrawLineV_call(HkVM *vm, HkValue *args);
static void DrawLineEx_call(HkVM *vm, HkValue*args);
static void DrawLineBezier_call(HkVM *vm, HkValue*args);
static void DrawCircleV_call(HkVM *vm, HkValue *args);
static void DrawRectangle_call(HkVM *vm, HkValue *args);
static void DrawRectangleV_call(HkVM *vm, HkValue *args);
static void DrawRectangleRec_call(HkVM *vm, HkValue *args);
static void DrawRectanglePro_call(HkVM *vm, HkValue *args);
static void DrawRectangleLines_call(HkVM *vm, HkValue *args);
static void DrawRectangleLinesEx_call(HkVM *vm, HkValue *args);
// Text drawing functions
static void DrawText_call(HkVM *vm, HkValue *args);

static inline void init(void)
{
  xStr = hk_string_from_chars(-1, "x");
  yStr = hk_string_from_chars(-1, "y");
  zStr = hk_string_from_chars(-1, "z");
  wStr = hk_string_from_chars(-1, "w");
  rStr = hk_string_from_chars(-1, "r");
  gStr = hk_string_from_chars(-1, "g");
  bStr = hk_string_from_chars(-1, "b");
  aStr = hk_string_from_chars(-1, "a");
  m0Str = hk_string_from_chars(-1, "m0");
  m1Str = hk_string_from_chars(-1, "m1");
  m2Str = hk_string_from_chars(-1, "m2");
  m3Str = hk_string_from_chars(-1, "m3");
  m4Str = hk_string_from_chars(-1, "m4");
  m5Str = hk_string_from_chars(-1, "m5");
  m6Str = hk_string_from_chars(-1, "m6");
  m7Str = hk_string_from_chars(-1, "m7");
  m8Str = hk_string_from_chars(-1, "m8");
  m9Str = hk_string_from_chars(-1, "m9");
  m10Str = hk_string_from_chars(-1, "m10");
  m11Str = hk_string_from_chars(-1, "m11");
  m12Str = hk_string_from_chars(-1, "m12");
  m13Str = hk_string_from_chars(-1, "m13");
  m14Str = hk_string_from_chars(-1, "m14");
  m15Str = hk_string_from_chars(-1, "m15");
  widthStr = hk_string_from_chars(-1, "width");
  heightStr = hk_string_from_chars(-1, "height");
  dataStr = hk_string_from_chars(-1, "data");
  mipmapsStr = hk_string_from_chars(-1, "mipmaps");
  formatStr = hk_string_from_chars(-1, "format");
  idStr = hk_string_from_chars(-1, "id");
  textureStr = hk_string_from_chars(-1, "texture");
  depthStr = hk_string_from_chars(-1, "depth");
  offsetStr = hk_string_from_chars(-1, "offset");
  targetStr = hk_string_from_chars(-1, "target");
  rotationStr = hk_string_from_chars(-1, "rotation");
  zoomStr = hk_string_from_chars(-1, "zoom");
  positionStr = hk_string_from_chars(-1, "position");
  upStr = hk_string_from_chars(-1, "up");
  fovyStr = hk_string_from_chars(-1, "fovy");
  projectionStr = hk_string_from_chars(-1, "projection");
  locsStr = hk_string_from_chars(-1, "locs");
}

static inline Vector2 vector2_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int xi = hk_struct_index_of(ztruct, xStr);
  int yi = hk_struct_index_of(ztruct, yStr);
  float x = (xi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, xi));
  float y = (yi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, yi));
  return (Vector2) { x, y };
}

static inline HkInstance *instance_from_vector2(Vector2 vector)
{
  HkInstance *inst = hk_instance_new(vector2Struct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(vector.x));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(vector.y));
  return inst;
}

static inline Vector3 vector3_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int xi = hk_struct_index_of(ztruct, xStr);
  int yi = hk_struct_index_of(ztruct, yStr);
  int zi = hk_struct_index_of(ztruct, zStr);
  float x = (xi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, xi));
  float y = (yi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, yi));
  float z = (zi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, zi));
  return (Vector3) { x, y, z };
}

static inline Color color_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int ri = hk_struct_index_of(ztruct, rStr);
  int gi = hk_struct_index_of(ztruct, gStr);
  int bi = hk_struct_index_of(ztruct, bStr);
  int ai = hk_struct_index_of(ztruct, aStr);
  unsigned char r = (ri == -1) ? 0 : (unsigned char) hk_as_number(hk_instance_get_field(inst, ri));
  unsigned char g = (gi == -1) ? 0 : (unsigned char) hk_as_number(hk_instance_get_field(inst, gi));
  unsigned char b = (bi == -1) ? 0 : (unsigned char) hk_as_number(hk_instance_get_field(inst, bi));
  unsigned char a = (ai == -1) ? 0 : (unsigned char) hk_as_number(hk_instance_get_field(inst, ai));
  return (Color) { r, g, b, a };
}

static inline Rectangle rectangle_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int xi = hk_struct_index_of(ztruct, xStr);
  int yi = hk_struct_index_of(ztruct, yStr);
  int widthi = hk_struct_index_of(ztruct, widthStr);
  int heighti = hk_struct_index_of(ztruct, heightStr);
  float x = (xi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, xi));
  float y = (yi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, yi));
  float width = (widthi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, widthi));
  float height = (heighti == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, heighti));
  return (Rectangle) { x, y, width, height };
}

static inline Image image_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int datai = hk_struct_index_of(ztruct, dataStr);
  int widthi = hk_struct_index_of(ztruct, widthStr);
  int heighti = hk_struct_index_of(ztruct, heightStr);
  int mipmapsi = hk_struct_index_of(ztruct, mipmapsStr);
  int formati = hk_struct_index_of(ztruct, formatStr);
  void *data = NULL;
  if (datai != -1)
  {
    HkValue val = hk_instance_get_field(inst, datai);
    if (hk_is_string(val))
    {
      HkString *str = hk_as_string(val);
      data = (void *) str->chars;
    }
  }
  int width = (widthi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, widthi));
  int height = (heighti == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, heighti));
  int mipmaps = (mipmapsi == -1) ? 1 : (int) hk_as_number(hk_instance_get_field(inst, mipmapsi));
  int format = (formati == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, formati));
  return (Image) { data, width, height, mipmaps, format };
}

static inline void images_from_array(HkArray *arr, Image *images)
{
  for (int i = 0; i < arr->length && i < MAX_IMAGES; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    Image image = { NULL, 0, 0, 0, 0 };
    if (hk_is_instance(val))
      image = image_from_instance(hk_as_instance(val));
    images[i] = image;
  }
}

static inline Texture texture_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int idi = hk_struct_index_of(ztruct, idStr);
  int widthi = hk_struct_index_of(ztruct, widthStr);
  int heighti = hk_struct_index_of(ztruct, heightStr);
  int mipmapsi = hk_struct_index_of(ztruct, mipmapsStr);
  int formati = hk_struct_index_of(ztruct, formatStr);
  unsigned int id = (idi == -1) ? 0 : (unsigned int) hk_as_number(hk_instance_get_field(inst, idi));
  int width = (widthi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, widthi));
  int height = (heighti == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, heighti));
  int mipmaps = (mipmapsi == -1) ? 1 : (int) hk_as_number(hk_instance_get_field(inst, mipmapsi));
  int format = (formati == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, formati));
  return (Texture) { id, width, height, mipmaps, format };
}

static inline RenderTexture render_texture_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int idi = hk_struct_index_of(ztruct, idStr);
  int texturei = hk_struct_index_of(ztruct, textureStr);
  int depthi = hk_struct_index_of(ztruct, depthStr);
  unsigned int id = (idi == -1) ? 0 : (unsigned int) hk_as_number(hk_instance_get_field(inst, idi));
  Texture texture = (Texture) { 0, 0, 0, 0, 0 };
  if (texturei != -1)
  {
    HkValue val = hk_instance_get_field(inst, texturei);
    if (hk_is_instance(val))
      texture = texture_from_instance(hk_as_instance(val));
  }
  Texture depth = (Texture) { 0, 0, 0, 0, 0 };
  if (depthi != -1)
  {
    HkValue val = hk_instance_get_field(inst, depthi);
    if (hk_is_instance(val))
      depth = texture_from_instance(hk_as_instance(val));
  }
  return (RenderTexture) { id, texture, depth };
}

static inline Camera2D camera2d_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int offseti = hk_struct_index_of(ztruct, offsetStr);
  int targeti = hk_struct_index_of(ztruct, targetStr);
  int rotationi = hk_struct_index_of(ztruct, rotationStr);
  int zoomi = hk_struct_index_of(ztruct, zoomStr);
  Vector2 offset = (Vector2) { 0, 0 };
  if (offseti != -1)
  {
    HkValue val = hk_instance_get_field(inst, offseti);
    if (hk_is_instance(val))
      offset = vector2_from_instance(hk_as_instance(val));
  }
  Vector2 target = (Vector2) { 0, 0 };
  if (targeti != -1)
  {
    HkValue val = hk_instance_get_field(inst, targeti);
    if (hk_is_instance(val))
      target = vector2_from_instance(hk_as_instance(val));
  }
  float rotation = (rotationi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, rotationi));
  float zoom = (zoomi == -1) ? 1 : (float) hk_as_number(hk_instance_get_field(inst, zoomi));
  return (Camera2D) { offset, target, rotation, zoom };
}

static inline Camera3D camera3d_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int positioni = hk_struct_index_of(ztruct, positionStr);
  int targeti = hk_struct_index_of(ztruct, targetStr);
  int upi = hk_struct_index_of(ztruct, upStr);
  int fovyi = hk_struct_index_of(ztruct, fovyStr);
  int projectioni = hk_struct_index_of(ztruct, projectionStr);
  Vector3 position = (Vector3) { 0, 0, 0 };
  if (positioni != -1)
  {
    HkValue val = hk_instance_get_field(inst, positioni);
    if (hk_is_instance(val))
      position = vector3_from_instance(hk_as_instance(val));
  }
  Vector3 target = (Vector3) { 0, 0, 0 };
  if (targeti != -1)
  {
    HkValue val = hk_instance_get_field(inst, targeti);
    if (hk_is_instance(val))
      target = vector3_from_instance(hk_as_instance(val));
  }
  Vector3 up = (Vector3) { 0, 0, 0 };
  if (upi != -1)
  {
    HkValue val = hk_instance_get_field(inst, upi);
    if (hk_is_instance(val))
      up = vector3_from_instance(hk_as_instance(val));
  }
  float fovy = (fovyi == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, fovyi));
  int projection = (projectioni == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, projectioni));
  return (Camera3D) { position, target, up, fovy, projection };
}

static inline void shader_locations_from_array(HkArray *arr, int *locs)
{
  for (int i = 0; i < arr->length && i < MAX_SHADER_LOCATIONS; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    locs[i] = hk_is_number(val) ? (int) hk_as_number(val) : 0;
  }
}

static inline Shader shader_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int idi = hk_struct_index_of(ztruct, idStr);
  int locsi = hk_struct_index_of(ztruct, locsStr);
  unsigned int id = (idi == -1) ? 0 : (unsigned int) hk_as_number(hk_instance_get_field(inst, idi));
  int locs[MAX_SHADER_LOCATIONS] = { 0 };
  if (locsi != -1)
  {
    HkValue val = hk_instance_get_field(inst, locsi);
    if (hk_is_array(val))
      shader_locations_from_array(hk_as_array(val), locs);
  }
  return (Shader) { id, locs };
}

static inline void load_vector2_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Vector2");
  vector2Struct = hk_struct_new(name);
  hk_struct_define_field(vector2Struct, xStr);
  hk_struct_define_field(vector2Struct, yStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, vector2Struct);
  hk_return_if_not_ok(vm);
}

static inline void load_vector3_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Vector3");
  vector3Struct = hk_struct_new(name);
  hk_struct_define_field(vector3Struct, xStr);
  hk_struct_define_field(vector3Struct, yStr);
  hk_struct_define_field(vector3Struct, zStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, vector3Struct);
  hk_return_if_not_ok(vm);
}

static inline void load_vector4_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Vector4");
  vector4Struct = hk_struct_new(name);
  hk_struct_define_field(vector4Struct, xStr);
  hk_struct_define_field(vector4Struct, yStr);
  hk_struct_define_field(vector4Struct, zStr);
  hk_struct_define_field(vector4Struct, wStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, vector4Struct);
  hk_return_if_not_ok(vm);
}

static inline void load_matrix_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Matrix");
  matrixStruct = hk_struct_new(name);
  hk_struct_define_field(matrixStruct, m0Str);
  hk_struct_define_field(matrixStruct, m1Str);
  hk_struct_define_field(matrixStruct, m2Str);
  hk_struct_define_field(matrixStruct, m3Str);
  hk_struct_define_field(matrixStruct, m4Str);
  hk_struct_define_field(matrixStruct, m5Str);
  hk_struct_define_field(matrixStruct, m6Str);
  hk_struct_define_field(matrixStruct, m7Str);
  hk_struct_define_field(matrixStruct, m8Str);
  hk_struct_define_field(matrixStruct, m9Str);
  hk_struct_define_field(matrixStruct, m10Str);
  hk_struct_define_field(matrixStruct, m11Str);
  hk_struct_define_field(matrixStruct, m12Str);
  hk_struct_define_field(matrixStruct, m13Str);
  hk_struct_define_field(matrixStruct, m14Str);
  hk_struct_define_field(matrixStruct, m15Str);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, matrixStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_color_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Color");
  colorStruct = hk_struct_new(name);
  hk_struct_define_field(colorStruct, rStr);
  hk_struct_define_field(colorStruct, gStr);
  hk_struct_define_field(colorStruct, bStr);
  hk_struct_define_field(colorStruct, aStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, colorStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_rectangle_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Rectangle");
  rectangleStruct = hk_struct_new(name);
  hk_struct_define_field(rectangleStruct, xStr);
  hk_struct_define_field(rectangleStruct, yStr);
  hk_struct_define_field(rectangleStruct, widthStr);
  hk_struct_define_field(rectangleStruct, heightStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, rectangleStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_image_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Image");
  imageStruct = hk_struct_new(name);
  hk_struct_define_field(imageStruct, dataStr);
  hk_struct_define_field(imageStruct, widthStr);
  hk_struct_define_field(imageStruct, heightStr);
  hk_struct_define_field(imageStruct, mipmapsStr);
  hk_struct_define_field(imageStruct, formatStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, imageStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_texture_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Texture");
  textureStruct = hk_struct_new(name);
  hk_struct_define_field(textureStruct, idStr);
  hk_struct_define_field(textureStruct, widthStr);
  hk_struct_define_field(textureStruct, heightStr);
  hk_struct_define_field(textureStruct, mipmapsStr);
  hk_struct_define_field(textureStruct, formatStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, textureStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_render_texture_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "RenderTexture");
  renderTextureStruct = hk_struct_new(name);
  hk_struct_define_field(renderTextureStruct, idStr);
  hk_struct_define_field(renderTextureStruct, textureStr);
  hk_struct_define_field(renderTextureStruct, depthStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, renderTextureStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_camera2D_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Camera2D");
  camera2DStruct = hk_struct_new(name);
  hk_struct_define_field(camera2DStruct, offsetStr);
  hk_struct_define_field(camera2DStruct, targetStr);
  hk_struct_define_field(camera2DStruct, rotationStr);
  hk_struct_define_field(camera2DStruct, zoomStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, camera2DStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_camera3D_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Camera3D");
  camera3DStruct = hk_struct_new(name);
  hk_struct_define_field(camera3DStruct, positionStr);
  hk_struct_define_field(camera3DStruct, targetStr);
  hk_struct_define_field(camera3DStruct, upStr);
  hk_struct_define_field(camera3DStruct, fovyStr);
  hk_struct_define_field(camera3DStruct, projectionStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, camera3DStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_shader_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Shader");
  shaderStruct = hk_struct_new(name);
  hk_struct_define_field(shaderStruct, idStr);
  hk_struct_define_field(shaderStruct, locsStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, shaderStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_colors(HkVM *vm)
{
  HkInstance *inst;
  hk_vm_push_string_from_chars(vm, -1, "LIGHTGRAY");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(200));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(200));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(200));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GRAY");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(130));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(130));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(130));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DARKGRAY");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(80));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(80));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(80));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "YELLOW");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(253));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(249));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GOLD");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(203));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ORANGE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(161));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "PINK");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(109));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(194));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "RED");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(230));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(41));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(55));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MAROON");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(190));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(33));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(55));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GREEN");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(228));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(48));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LIME");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(158));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(47));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DARKGREEN");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(117));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(44));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SKYBLUE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(102));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(191));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BLUE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(121));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(241));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DARKBLUE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(82));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(172));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "PURPLE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(200));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(122));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "VIOLET");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(135));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(60));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(190));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DARKPURPLE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(112));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(31));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(126));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BEIGE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(211));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(176));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(131));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BROWN");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(127));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(106));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(79));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DARKBROWN");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(76));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(63));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(47));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "WHITE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BLACK");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BLANK");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(0));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MAGENTA");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(0));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(255));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "RAYWHITE");
  hk_return_if_not_ok(vm);
  inst = hk_instance_new(colorStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(245));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(245));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(245));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(255));
  hk_vm_push_instance(vm, inst);
  hk_return_if_not_ok(vm);
}

static inline void load_functions(HkVM *vm)
{
  // Window-related functions
  hk_vm_push_string_from_chars(vm, -1, "InitWindow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "InitWindow", 3, InitWindow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CloseWindow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CloseWindow", 0, CloseWindow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "WindowShouldClose");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "WindowShouldClose", 0, WindowShouldClose_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowReady");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowReady", 0, IsWindowReady_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowFullscreen");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowFullscreen", 0, IsWindowFullscreen_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowHidden");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowHidden", 0, IsWindowHidden_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowMinimized");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowMinimized", 0, IsWindowMinimized_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowMaximized");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowMaximized", 0, IsWindowMaximized_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowFocused");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowFocused", 0, IsWindowFocused_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowResized");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowResized", 0, IsWindowResized_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsWindowState");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsWindowState", 1, IsWindowState_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowState");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowState", 1, SetWindowState_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ClearWindowState");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ClearWindowState", 1, ClearWindowState_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ToggleFullscreen");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ToggleFullscreen", 0, ToggleFullscreen_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ToggleBorderlessWindowed");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ToggleBorderlessWindowed", 0, ToggleBorderlessWindowed_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MaximizeWindow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "MaximizeWindow", 0, MaximizeWindow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MinimizeWindow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "MinimizeWindow", 0, MinimizeWindow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "RestoreWindow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "RestoreWindow", 0, RestoreWindow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowIcon");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowIcon", 1, SetWindowIcon_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowIcons");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowIcons", 1, SetWindowIcons_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowTitle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowTitle", 1, SetWindowTitle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowPosition");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowPosition", 2, SetWindowPosition_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowMonitor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowMonitor", 1, SetWindowMonitor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowMinSize");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowMinSize", 2, SetWindowMinSize_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowMaxSize");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowMaxSize", 2, SetWindowMaxSize_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowSize");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowSize", 2, SetWindowSize_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowOpacity");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowOpacity", 1, SetWindowOpacity_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetWindowFocused");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetWindowFocused", 0, SetWindowFocused_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetScreenWidth");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetScreenWidth", 0, GetScreenWidth_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetScreenHeight");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetScreenHeight", 0, GetScreenHeight_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetRenderWidth");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetRenderWidth", 0, GetRenderWidth_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetRenderHeight");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetRenderHeight", 0, GetRenderHeight_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorCount");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorCount", 0, GetMonitorCount_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetCurrentMonitor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetCurrentMonitor", 0, GetCurrentMonitor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorPosition");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorPosition", 1, GetMonitorPosition_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorWidth");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorWidth", 1, GetMonitorWidth_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorHeight");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorHeight", 1, GetMonitorHeight_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorPhysicalWidth");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorPhysicalWidth", 1, GetMonitorPhysicalWidth_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorPhysicalHeight");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorPhysicalHeight", 1, GetMonitorPhysicalHeight_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorRefreshRate");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorRefreshRate", 1, GetMonitorRefreshRate_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetWindowPosition");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetWindowPosition", 0, GetWindowPosition_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetWindowScaleDPI");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetWindowScaleDPI", 0, GetWindowScaleDPI_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetMonitorName");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetMonitorName", 1, GetMonitorName_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetClipboardText");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetClipboardText", 1, SetClipboardText_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetClipboardText");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetClipboardText", 0, GetClipboardText_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EnableEventWaiting");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EnableEventWaiting", 0, EnableEventWaiting_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DisableEventWaiting");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DisableEventWaiting", 0, DisableEventWaiting_call);
  hk_return_if_not_ok(vm);
  // Cursor-related functions
  hk_vm_push_string_from_chars(vm, -1, "ShowCursor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ShowCursor", 0, ShowCursor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "HideCursor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "HideCursor", 0, HideCursor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsCursorHidden");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsCursorHidden", 0, IsCursorHidden_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EnableCursor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EnableCursor", 0, EnableCursor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DisableCursor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DisableCursor", 0, DisableCursor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsCursorOnScreen");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsCursorOnScreen", 0, IsCursorOnScreen_call);
  hk_return_if_not_ok(vm);
  // Drawing-related functions
  hk_vm_push_string_from_chars(vm, -1, "ClearBackground");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ClearBackground", 1, ClearBackground_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginDrawing");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginDrawing", 0, BeginDrawing_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndDrawing");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndDrawing", 0, EndDrawing_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginMode2D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginMode2D", 1, BeginMode2D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndMode2D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndMode2D", 0, EndMode2D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginMode3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginMode3D", 1, BeginMode3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndMode3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndMode3D", 0, EndMode3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginTextureMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginTextureMode", 1, BeginTextureMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndTextureMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndTextureMode", 0, EndTextureMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginShaderMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginShaderMode", 1, BeginShaderMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndShaderMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndShaderMode", 0, EndShaderMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginBlendMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginBlendMode", 1, BeginBlendMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndBlendMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndBlendMode", 0, EndBlendMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "BeginScissorMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "BeginScissorMode", 4, BeginScissorMode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "EndScissorMode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "EndScissorMode", 0, EndScissorMode_call);
  hk_return_if_not_ok(vm);
  // Timing-related functions
  hk_vm_push_string_from_chars(vm, -1, "SetTargetFPS");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetTargetFPS", 1, SetTargetFPS_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetFrameTime");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetFrameTime", 0, GetFrameTime_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetTime");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetTime", 0, GetTime_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetFPS");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetFPS", 0, GetFPS_call);
  hk_return_if_not_ok(vm);
  // Input-related functions: keyboard
  hk_vm_push_string_from_chars(vm, -1, "IsKeyPressed");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsKeyPressed", 1, IsKeyPressed_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsKeyPressedRepeat");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsKeyPressedRepeat", 1, IsKeyPressedRepeat_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsKeyDown");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsKeyDown", 1, IsKeyDown_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsKeyReleased");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsKeyReleased", 1, IsKeyReleased_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IsKeyUp");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "IsKeyUp", 1, IsKeyUp_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetKeyPressed");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetKeyPressed", 0, GetKeyPressed_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetCharPressed");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetCharPressed", 0, GetCharPressed_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetExitKey");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetExitKey", 1, SetExitKey_call);
  hk_return_if_not_ok(vm);
  // Basic shapes drawing functions
  hk_vm_push_string_from_chars(vm, -1, "DrawPixel");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPixel", 3, DrawPixel_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPixelV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPixelV", 2, DrawPixelV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLine");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLine", 4, DrawLine_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLineV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLineV", 2, DrawLineV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLineEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLineEx", 4, DrawLineEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLineBezier");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLineBezier", 4, DrawLineBezier_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleV", 3, DrawCircleV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangle", 5, DrawRectangle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleV", 3, DrawRectangleV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleRec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleRec", 2, DrawRectangleRec_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectanglePro");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectanglePro", 4, DrawRectanglePro_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleLines", 4, DrawRectangleLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleLinesEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleLinesEx", 3, DrawRectangleLinesEx_call);
  hk_return_if_not_ok(vm);
  // Text drawing functions
  hk_vm_push_string_from_chars(vm, -1, "DrawText");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawText", 5, DrawText_call);
  hk_return_if_not_ok(vm);
}

static void InitWindow_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  int width = (int) hk_as_number(args[1]);
  int height = (int) hk_as_number(args[2]);
  const char *title = (const char *) hk_as_string(args[3])->chars;
  InitWindow(width, height, title);
  hk_vm_push_nil(vm);
}

static void CloseWindow_call(HkVM *vm, HkValue *args)
{
  (void) args;
  CloseWindow();
  hk_vm_push_nil(vm);
}

static void WindowShouldClose_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_bool(vm, WindowShouldClose());
}

static void IsWindowReady_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_bool(vm, IsWindowReady());
}

static void IsWindowFullscreen_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowFullscreen();
  hk_vm_push_bool(vm, result);
}

static void IsWindowHidden_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowHidden();
  hk_vm_push_bool(vm, result);
}

static void IsWindowMinimized_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowMinimized();
  hk_vm_push_bool(vm, result);
}

static void IsWindowMaximized_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowMaximized();
  hk_vm_push_bool(vm, result);
}

static void IsWindowFocused_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowFocused();
  hk_vm_push_bool(vm, result);
}

static void IsWindowResized_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsWindowResized();
  hk_vm_push_bool(vm, result);
}

static void IsWindowState_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  unsigned int flag = (unsigned int) hk_as_number(args[1]);
  bool result = IsWindowState(flag);
  hk_vm_push_bool(vm, !!result);
}

static void SetWindowState_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  unsigned int flags = (unsigned int) hk_as_number(args[1]);
  SetWindowState(flags);
  hk_vm_push_nil(vm);
}

static void ClearWindowState_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  unsigned int flags = (unsigned int) hk_as_number(args[1]);
  ClearWindowState(flags);
  hk_vm_push_nil(vm);
}

static void ToggleFullscreen_call(HkVM *vm, HkValue *args)
{
  (void) args;
  ToggleFullscreen();
  hk_vm_push_nil(vm);
}

static void ToggleBorderlessWindowed_call(HkVM *vm, HkValue *args)
{
  (void) args;
  ToggleBorderlessWindowed();
  hk_vm_push_nil(vm);
}

static void MaximizeWindow_call(HkVM *vm, HkValue *args)
{
  (void) args;
  MaximizeWindow();
  hk_vm_push_nil(vm);
}

static void MinimizeWindow_call(HkVM *vm, HkValue *args)
{
  (void) args;
  MinimizeWindow();
  hk_vm_push_nil(vm);
}

static void RestoreWindow_call(HkVM *vm, HkValue *args)
{
  (void) args;
  RestoreWindow();
  hk_vm_push_nil(vm);
}

static void SetWindowIcon_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Image image = image_from_instance(hk_as_instance(args[1]));
  SetWindowIcon(image);
  hk_vm_push_nil(vm);
}

static void SetWindowIcons_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  Image images[MAX_IMAGES];
  HkArray *arr = hk_as_array(args[1]);
  images_from_array(arr, images);
  SetWindowIcons(images, arr->length);
  hk_vm_push_nil(vm);
}

static void SetWindowTitle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  const char *title = (const char *) hk_as_string(args[1])->chars;
  SetWindowTitle(title);
  hk_vm_push_nil(vm);
}

static void SetWindowPosition_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int x = (int) hk_as_number(args[1]);
  int y = (int) hk_as_number(args[2]);
  SetWindowPosition(x, y);
  hk_vm_push_nil(vm);
}

static void SetWindowMonitor_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  SetWindowMonitor(monitor);
  hk_vm_push_nil(vm);
}

static void SetWindowMinSize_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int width = (int) hk_as_number(args[1]);
  int height = (int) hk_as_number(args[2]);
  SetWindowMinSize(width, height);
  hk_vm_push_nil(vm);
}

static void SetWindowMaxSize_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int width = (int) hk_as_number(args[1]);
  int height = (int) hk_as_number(args[2]);
  SetWindowMaxSize(width, height);
  hk_vm_push_nil(vm);
}

static void SetWindowSize_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int width = (int) hk_as_number(args[1]);
  int height = (int) hk_as_number(args[2]);
  SetWindowSize(width, height);
  hk_vm_push_nil(vm);
}

static void SetWindowOpacity_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  float opacity = (float) hk_as_number(args[1]);
  SetWindowOpacity(opacity);
  hk_vm_push_nil(vm);
}

static void SetWindowFocused_call(HkVM *vm, HkValue *args)
{
  (void) args;
  SetWindowFocused();
  hk_vm_push_nil(vm);
}

static void GetScreenWidth_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetScreenWidth();
  hk_vm_push_number(vm, result);
}

static void GetScreenHeight_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetScreenHeight();
  hk_vm_push_number(vm, result);
}

static void GetRenderWidth_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetRenderWidth();
  hk_vm_push_number(vm, result);
}

static void GetRenderHeight_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetRenderHeight();
  hk_vm_push_number(vm, result);
}

static void GetMonitorCount_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetMonitorCount();
  hk_vm_push_number(vm, result);
}

static void GetCurrentMonitor_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetCurrentMonitor();
  hk_vm_push_number(vm, result);
}

static void GetMonitorPosition_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  Vector2 vector = GetMonitorPosition(monitor);
  HkInstance *inst = instance_from_vector2(vector);
  hk_vm_push_instance(vm, inst);
}

static void GetMonitorWidth_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  int result = GetMonitorWidth(monitor);
  hk_vm_push_number(vm, result);
}

static void GetMonitorHeight_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  int result = GetMonitorHeight(monitor);
  hk_vm_push_number(vm, result);
}

static void GetMonitorPhysicalWidth_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  int result = GetMonitorPhysicalWidth(monitor);
  hk_vm_push_number(vm, result);
}

static void GetMonitorPhysicalHeight_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  int result = GetMonitorPhysicalHeight(monitor);
  hk_vm_push_number(vm, result);
}

static void GetMonitorRefreshRate_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  int result = GetMonitorRefreshRate(monitor);
  hk_vm_push_number(vm, result);
}

static void GetWindowPosition_call(HkVM *vm, HkValue *args)
{
  (void) args;
  Vector2 vector = GetWindowPosition();
  HkInstance *inst = instance_from_vector2(vector);
  hk_vm_push_instance(vm, inst);
}

static void GetWindowScaleDPI_call(HkVM *vm, HkValue *args)
{
  (void) args;
  Vector2 vector = GetWindowScaleDPI();
  HkInstance *inst = instance_from_vector2(vector);
  hk_vm_push_instance(vm, inst);
}

static void GetMonitorName_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int monitor = (int) hk_as_number(args[1]);
  const char *chars = GetMonitorName(monitor);
  hk_vm_push_string_from_chars(vm, -1, chars);
}

static void SetClipboardText_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  const char *text = (const char *) hk_as_string(args[1])->chars;
  SetClipboardText(text);
  hk_vm_push_nil(vm);
}

static void GetClipboardText_call(HkVM *vm, HkValue *args)
{
  (void) args;
  const char *text = GetClipboardText();
  hk_vm_push_string_from_chars(vm, -1, text);
}

static void EnableEventWaiting_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EnableEventWaiting();
  hk_vm_push_nil(vm);
}

static void DisableEventWaiting_call(HkVM *vm, HkValue *args)
{
  (void) args;
  DisableEventWaiting();
  hk_vm_push_nil(vm);
}

static void ShowCursor_call(HkVM *vm, HkValue *args)
{
  (void) args;
  ShowCursor();
  hk_vm_push_nil(vm);
}

static void HideCursor_call(HkVM *vm, HkValue *args)
{
  (void) args;
  HideCursor();
  hk_vm_push_nil(vm);
}

static void IsCursorHidden_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsCursorHidden();
  hk_vm_push_bool(vm, result);
}

static void EnableCursor_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EnableCursor();
  hk_vm_push_nil(vm);
}

static void DisableCursor_call(HkVM *vm, HkValue *args)
{
  (void) args;
  DisableCursor();
  hk_vm_push_nil(vm);
}

static void IsCursorOnScreen_call(HkVM *vm, HkValue *args)
{
  (void) args;
  bool result = IsCursorOnScreen();
  hk_vm_push_bool(vm, result);
}

static void ClearBackground_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Color color = color_from_instance(hk_as_instance(args[1]));
  ClearBackground(color);
  hk_vm_push_nil(vm);
}

static void BeginDrawing_call(HkVM *vm, HkValue *args)
{
  (void) args;
  BeginDrawing();
  hk_vm_push_nil(vm);
}

static void EndDrawing_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndDrawing();
  hk_vm_push_nil(vm);
}

static void BeginMode2D_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Camera2D camera = camera2d_from_instance(hk_as_instance(args[1]));
  BeginMode2D(camera);
  hk_vm_push_nil(vm);
}

static void EndMode2D_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndMode2D();
  hk_vm_push_nil(vm);
}

static void BeginMode3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Camera3D camera = camera3d_from_instance(hk_as_instance(args[1]));
  BeginMode3D(camera);
  hk_vm_push_nil(vm);
}

static void EndMode3D_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndMode3D();
  hk_vm_push_nil(vm);
}

static void BeginTextureMode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  RenderTexture target = render_texture_from_instance(hk_as_instance(args[1]));
  BeginTextureMode(target);
  hk_vm_push_nil(vm);
}

static void EndTextureMode_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndTextureMode();
  hk_vm_push_nil(vm);
}

static void BeginShaderMode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  BeginShaderMode(shader);
  hk_vm_push_nil(vm);
}

static void EndShaderMode_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndShaderMode();
  hk_vm_push_nil(vm);
}

static void BeginBlendMode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int mode = (int) hk_as_number(args[1]);
  BeginBlendMode(mode);
  hk_vm_push_nil(vm);
}

static void EndBlendMode_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndBlendMode();
  hk_vm_push_nil(vm);
}

static void BeginScissorMode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  int x = (int) hk_as_number(args[1]);
  int y = (int) hk_as_number(args[2]);
  int width = (int) hk_as_number(args[3]);
  int height = (int) hk_as_number(args[4]);
  BeginScissorMode(x, y, width, height);
  hk_vm_push_nil(vm);
}

static void EndScissorMode_call(HkVM *vm, HkValue *args)
{
  (void) args;
  EndScissorMode();
  hk_vm_push_nil(vm);
}

static void SetTargetFPS_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int fps = (int) hk_as_number(args[1]);
  SetTargetFPS(fps);
  hk_vm_push_nil(vm);
}

static void GetFrameTime_call(HkVM *vm, HkValue *args)
{
  (void) args;
  float result = GetFrameTime();
  hk_vm_push_number(vm, result);
}

static void GetTime_call(HkVM *vm, HkValue *args)
{
  (void) args;
  double result = GetTime();
  hk_vm_push_number(vm, result);
}

static void GetFPS_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetFPS();
  hk_vm_push_number(vm, result);
}

static void IsKeyPressed_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  bool result = IsKeyPressed(key);
  hk_vm_push_bool(vm, result);
}

static void IsKeyPressedRepeat_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  bool result = IsKeyPressedRepeat(key);  
  hk_vm_push_bool(vm, result);
}

static void IsKeyDown_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  bool result = IsKeyDown(key);
  hk_vm_push_bool(vm, result);
}

static void IsKeyReleased_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  bool result = IsKeyReleased(key);
  hk_vm_push_bool(vm, result);
}

static void IsKeyUp_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  bool result = IsKeyUp(key);
  hk_vm_push_bool(vm, result);
}

static void GetKeyPressed_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetKeyPressed();
  hk_vm_push_number(vm, result);
}

static void GetCharPressed_call(HkVM *vm, HkValue *args)
{
  (void) args;
  int result = GetCharPressed();
  hk_vm_push_number(vm, result);
}

static void SetExitKey_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int key = (int) hk_as_number(args[1]);
  SetExitKey(key);
  hk_vm_push_nil(vm);
}

static void DrawPixel_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawPixel(posX, posY, color);
  hk_vm_push_nil(vm);
}

static void DrawPixelV_call(HkVM *vm, HkValue*args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Vector2 position = vector2_from_instance(hk_as_instance(args[1]));
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawPixelV(position, color);
  hk_vm_push_nil(vm);
}

static void DrawLine_call(HkVM *vm, HkValue*args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int startPosX = (int) hk_as_number(args[1]);
  int startPosY = (int) hk_as_number(args[2]);
  int endPosX = (int) hk_as_number(args[3]);
  int endPosY = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawLine(startPosX, startPosY, endPosX, endPosY, color);
  hk_vm_push_nil(vm);
}

static void DrawLineV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector2 startPos = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 endPos = vector2_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawLineV(startPos, endPos, color);
  hk_vm_push_nil(vm);
}

static void DrawLineEx_call(HkVM *vm, HkValue*args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 startPos = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 endPos = vector2_from_instance(hk_as_instance(args[2]));
  float thick = (float) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawLineEx(startPos, endPos, thick, color);
  hk_vm_push_nil(vm);
}

static void DrawLineBezier_call(HkVM *vm, HkValue*args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 startPos = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 endPos = vector2_from_instance(hk_as_instance(args[2]));
  float thick = (float) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawLineBezier(startPos, endPos, thick, color);
  hk_vm_push_nil(vm);
}

static void DrawCircleV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawCircleV(center, radius, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  int width = (int) hk_as_number(args[3]);
  int height = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawRectangle(posX, posY, width, height, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector2 position = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 size = vector2_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawRectangleV(position, size, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleRec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawRectangleRec(rec, color);
  hk_vm_push_nil(vm);
}

static void DrawRectanglePro_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  Vector2 origin = vector2_from_instance(hk_as_instance(args[2]));
  float rotation = (float) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawRectanglePro(rec, origin, rotation, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  int width = (int) hk_as_number(args[3]);
  int height = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawRectangleLines(posX, posY, width, height, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleLinesEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  float lineThick = (float) hk_as_number(args[2]);
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawRectangleLinesEx(rec, lineThick, color);
  hk_vm_push_nil(vm);
}

static void DrawText_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  const char *text = (const char *) hk_as_string(args[1])->chars;
  int posX = (int) hk_as_number(args[2]);
  int posY = (int) hk_as_number(args[3]);
  int fontSize = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawText(text, posX, posY, fontSize, color);
  hk_vm_push_nil(vm);
}

HK_LOAD_MODULE_HANDLER(raylib)
{
  init();
  hk_vm_push_string_from_chars(vm, -1, "raylib");
  hk_return_if_not_ok(vm);
  load_vector2_struct(vm);
  load_vector3_struct(vm);
  load_vector4_struct(vm);
  load_matrix_struct(vm);
  load_color_struct(vm);
  load_rectangle_struct(vm);
  load_image_struct(vm);
  load_texture_struct(vm);
  load_render_texture_struct(vm);
  load_camera2D_struct(vm);
  load_camera3D_struct(vm);
  load_shader_struct(vm);
  load_colors(vm);
  load_functions(vm);
  hk_vm_construct(vm, 131);
}
