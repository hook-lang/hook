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
#include <rlgl.h>

#define LIST_CAPACITY 256

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
static HkString *sourceStr;
static HkString *leftStr;
static HkString *topStr;
static HkString *rightStr;
static HkString *bottomStr;
static HkString *layoutStr;
static HkString *valueStr;
static HkString *offsetXStr;
static HkString *offsetYStr;
static HkString *advanceXStr;
static HkString *imageStr;
static HkString *baseSizeStr;
static HkString *glyphCountStr;
static HkString *glyphPaddingStr;
static HkString *recsStr;
static HkString *glyphsStr;
static HkString *colorStr;
static HkString *shaderStr;
static HkString *mapsStr;
static HkString *paramsStr;
static HkString *vertexCountStr;
static HkString *triangleCountStr;
static HkString *verticesStr;
static HkString *texcoordsStr;
static HkString *texcoords2Str;
static HkString *normalsStr;
static HkString *tangentsStr;
static HkString *colorsStr;
static HkString *indicesStr;
static HkString *animVerticesStr;
static HkString *animNormalsStr;
static HkString *boneIdsStr;
static HkString *boneWeightsStr;
static HkString *vaoIdStr;
static HkString *vboIdStr;
static HkString *transformStr;
static HkString *meshCountStr;
static HkString *materialCountStr;
static HkString *meshesStr;
static HkString *materialsStr;
static HkString *meshMaterialStr;
static HkString *boneCountStr;
static HkString *bonesStr;
static HkString *bindPoseStr;
static HkString *boneCountStr;
static HkString *frameCountStr;
static HkString *bonesStr;
static HkString *framePosesStr;
static HkString *nameStr;
static HkString *translationStr;
static HkString *rotationStr;
static HkString *scaleStr;
static HkString *nameStr;
static HkString *parentStr;
static HkString *positionStr;
static HkString *directionStr;
static HkString *hitStr;
static HkString *distanceStr;
static HkString *pointStr;
static HkString *normalStr;
static HkString *minStr;
static HkString *maxStr;

static HkStruct *vector2Struct;
static HkStruct *vector3Struct;
static HkStruct *vector4Struct;
static HkStruct *matrixStruct;
static HkStruct *colorStruct;
static HkStruct *rectangleStruct;
static HkStruct *imageStruct;
// TODO: Replace texture struct by userdata.
static HkStruct *textureStruct;
// TODO: Replace render texture struct by userdata.
static HkStruct *renderTextureStruct;
static HkStruct *nPatchInfoStruct;
static HkStruct *glyphInfoStruct;
static HkStruct *fontStruct;
static HkStruct *camera2DStruct;
static HkStruct *camera3DStruct;
// TODO: Replace shader struct by userdata.
static HkStruct *shaderStruct;
static HkStruct *materialMapStruct;
static HkStruct *materialStruct;
static HkStruct *meshStruct;
static HkStruct *modelStruct;
static HkStruct *modelAnimationStruct;
static HkStruct *transformStruct;
static HkStruct *boneInfoStruct;
static HkStruct *rayStruct;
static HkStruct *rayCollisionStruct;
static HkStruct *boundingBoxStruct;

static inline void init(void);
static inline void int_list_from_array(HkArray *arr, int *list);
static inline HkArray *array_from_int_list(int *list, int length);
static inline Vector2 vector2_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_vector2(Vector2 vector);
static inline void vector2_list_from_array(HkArray *arr, Vector2 *list);
static inline Vector3 vector3_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_vector3(Vector3 vector);
static inline void vector3_list_from_array(HkArray *arr, Vector3 *list);
static inline Matrix matrix_from_instance(HkInstance *inst);
static inline Color color_from_instance(HkInstance *inst);
static inline Rectangle rectangle_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_rectangle(Rectangle rect);
static inline void rectangle_list_from_array(HkArray *arr, Rectangle *list);
static inline Image image_from_instance(HkInstance *inst);
static inline void image_list_from_array(HkArray *arr, Image *list);
static inline Texture texture_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_texture(Texture texture);
static inline RenderTexture render_texture_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_render_texture(RenderTexture renderTexture);
static inline GlyphInfo glyph_info_from_instance(HkInstance *inst);
static inline void glyph_info_list_from_array(HkArray *arr, GlyphInfo *list);
static inline Font font_from_instance(HkInstance *inst);
static inline Camera2D camera2d_from_instance(HkInstance *inst);
static inline Camera3D camera3d_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_camera3d(Camera3D camera);
static inline Shader shader_from_instance(HkInstance *inst);
static inline HkInstance *instance_from_shader(Shader shader);
static inline Ray ray_from_instance(HkInstance *inst);
static inline void load_vector2_struct(HkVM *vm);
static inline void load_vector3_struct(HkVM *vm);
static inline void load_vector4_struct(HkVM *vm);
static inline void load_matrix_struct(HkVM *vm);
static inline void load_color_struct(HkVM *vm);
static inline void load_rectangle_struct(HkVM *vm);
static inline void load_image_struct(HkVM *vm);
static inline void load_texture_struct(HkVM *vm);
static inline void load_render_texture_struct(HkVM *vm);
static inline void load_npatch_info_struct(HkVM *vm);
static inline void load_glyph_info_struct(HkVM *vm);
static inline void load_font_struct(HkVM *vm);
static inline void load_camera2D_struct(HkVM *vm);
static inline void load_camera3D_struct(HkVM *vm);
static inline void load_shader_struct(HkVM *vm);
static inline void load_material_map_struct(HkVM *vm);
static inline void load_material_struct(HkVM *vm);
static inline void load_mesh_struct(HkVM *vm);
static inline void load_model_struct(HkVM *vm);
static inline void load_model_animation_struct(HkVM *vm);
static inline void load_transform_struct(HkVM *vm);
static inline void load_bone_info_struct(HkVM *vm);
static inline void load_ray_struct(HkVM *vm);
static inline void load_ray_collision_struct(HkVM *vm);
static inline void load_bounding_box_struct(HkVM *vm);
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
// Shader management functions
static void LoadShader_call(HkVM *vm, HkValue *args);
static void LoadShaderFromMemory_call(HkVM *vm, HkValue *args);
static void GetShaderLocation_call(HkVM *vm, HkValue *args);
static void GetShaderLocationAttrib_call(HkVM *vm, HkValue *args);
static void SetShaderValue_call(HkVM *vm, HkValue *args);
static void SetShaderValueV_call(HkVM *vm, HkValue *args);
static void SetShaderValueMatrix_call(HkVM *vm, HkValue *args);
static void SetShaderValueTexture_call(HkVM *vm, HkValue *args);
// TODO: Call UnloadShader automatically when the shader is no longer needed.
static void UnloadShader_call(HkVM *vm, HkValue *args);
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
// Camera System Functions
static void UpdateCamera_call(HkVM *vm, HkValue *args);
static void UpdateCameraPro_call(HkVM *vm, HkValue *args);
// Basic shapes drawing functions
static void DrawPixel_call(HkVM *vm, HkValue *args);
static void DrawPixelV_call(HkVM *vm, HkValue *args);
static void DrawLine_call(HkVM *vm, HkValue *args);
static void DrawLineV_call(HkVM *vm, HkValue *args);
static void DrawLineEx_call(HkVM *vm, HkValue *args);
static void DrawLineStrip_call(HkVM *vm, HkValue *args);
static void DrawLineBezier_call(HkVM *vm, HkValue *args);
static void DrawCircle_call(HkVM *vm, HkValue *args);
static void DrawCircleSector_call(HkVM *vm, HkValue *args);
static void DrawCircleSectorLines_call(HkVM *vm, HkValue *args);
static void DrawCircleGradient_call(HkVM *vm, HkValue *args);
static void DrawCircleV_call(HkVM *vm, HkValue *args);
static void DrawCircleLines_call(HkVM *vm, HkValue *args);
static void DrawCircleLinesV_call(HkVM *vm, HkValue *args);
static void DrawEllipse_call(HkVM *vm, HkValue *args);
static void DrawEllipseLines_call(HkVM *vm, HkValue *args);
static void DrawRing_call(HkVM *vm, HkValue *args);
static void DrawRingLines_call(HkVM *vm, HkValue *args);
static void DrawRectangle_call(HkVM *vm, HkValue *args);
static void DrawRectangleV_call(HkVM *vm, HkValue *args);
static void DrawRectangleRec_call(HkVM *vm, HkValue *args);
static void DrawRectanglePro_call(HkVM *vm, HkValue *args);
static void DrawRectangleGradientV_call(HkVM *vm, HkValue *args);
static void DrawRectangleGradientH_call(HkVM *vm, HkValue *args);
static void DrawRectangleGradientEx_call(HkVM *vm, HkValue *args);
static void DrawRectangleLines_call(HkVM *vm, HkValue *args);
static void DrawRectangleLinesEx_call(HkVM *vm, HkValue *args);
static void DrawRectangleRounded_call(HkVM *vm, HkValue *args);
static void DrawRectangleRoundedLines_call(HkVM *vm, HkValue *args);
static void DrawRectangleRoundedLinesEx_call(HkVM *vm, HkValue *args);
static void DrawTriangle_call(HkVM *vm, HkValue *args);
static void DrawTriangleLines_call(HkVM *vm, HkValue *args);
static void DrawTriangleFan_call(HkVM *vm, HkValue *args);
static void DrawTriangleStrip_call(HkVM *vm, HkValue *args);
static void DrawPoly_call(HkVM *vm, HkValue *args);
static void DrawPolyLines_call(HkVM *vm, HkValue *args);
static void DrawPolyLinesEx_call(HkVM *vm, HkValue *args);
// Basic shapes collision detection functions
static void CheckCollisionRecs_call(HkVM *vm, HkValue *args);
static void CheckCollisionCircles_call(HkVM *vm, HkValue *args);
static void CheckCollisionCircleRec_call(HkVM *vm, HkValue *args);
static void CheckCollisionCircleLine_call(HkVM *vm, HkValue *args);
static void CheckCollisionPointRec_call(HkVM *vm, HkValue *args);
static void CheckCollisionPointCircle_call(HkVM *vm, HkValue *args);
static void CheckCollisionPointTriangle_call(HkVM *vm, HkValue *args);
static void CheckCollisionPointLine_call(HkVM *vm, HkValue *args);
static void CheckCollisionPointPoly_call(HkVM *vm, HkValue *args);
static void CheckCollisionLines_call(HkVM *vm, HkValue *args);
static void GetCollisionRec_call(HkVM *vm, HkValue *args);
// Texture loading functions
static void LoadTexture_call(HkVM *vm, HkValue *args);
static void LoadTextureFromImage_call(HkVM *vm, HkValue *args);
static void LoadTextureCubemap_call(HkVM *vm, HkValue *args);
static void LoadRenderTexture_call(HkVM *vm, HkValue *args);
// TODO: Call UnloadTexture automatically when the texture is no longer needed.
static void UnloadTexture_call(HkVM *vm, HkValue *args);
// TODO: Call UnloadRenderTexture automatically when the render texture is no longer needed.
static void UnloadRenderTexture_call(HkVM *vm, HkValue *args);
static void UpdateTexture_call(HkVM *vm, HkValue *args);
static void UpdateTextureRec_call(HkVM *vm, HkValue *args);
// Text drawing functions
static void DrawFPS_call(HkVM *vm, HkValue *args);
static void DrawText_call(HkVM *vm, HkValue *args);
static void DrawTextEx_call(HkVM *vm, HkValue *args);
static void DrawTextPro_call(HkVM *vm, HkValue *args);
static void DrawTextCodepoint_call(HkVM *vm, HkValue *args);
static void DrawTextCodepoints_call(HkVM *vm, HkValue *args);
// Basic geometric 3D shapes drawing functions
static void DrawLine3D_call(HkVM *vm, HkValue *args);
static void DrawPoint3D_call(HkVM *vm, HkValue *args);
static void DrawCircle3D_call(HkVM *vm, HkValue *args);
static void DrawTriangle3D_call(HkVM *vm, HkValue *args);
static void DrawTriangleStrip3D_call(HkVM *vm, HkValue *args);
static void DrawCube_call(HkVM *vm, HkValue *args);
static void DrawCubeV_call(HkVM *vm, HkValue *args);
static void DrawCubeWires_call(HkVM *vm, HkValue *args);
static void DrawCubeWiresV_call(HkVM *vm, HkValue *args);
static void DrawSphere_call(HkVM *vm, HkValue *args);
static void DrawSphereEx_call(HkVM *vm, HkValue *args);
static void DrawSphereWires_call(HkVM *vm, HkValue *args);
static void DrawCylinder_call(HkVM *vm, HkValue *args);
static void DrawCylinderEx_call(HkVM *vm, HkValue *args);
static void DrawCylinderWires_call(HkVM *vm, HkValue *args);
static void DrawCylinderWiresEx_call(HkVM *vm, HkValue *args);
static void DrawCapsule_call(HkVM *vm, HkValue *args);
static void DrawCapsuleWires_call(HkVM *vm, HkValue *args);
static void DrawPlane_call(HkVM *vm, HkValue *args);
static void DrawRay_call(HkVM *vm, HkValue *args);
static void DrawGrid_call(HkVM *vm, HkValue *args);

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
  sourceStr = hk_string_from_chars(-1, "source");
  leftStr = hk_string_from_chars(-1, "left");
  topStr = hk_string_from_chars(-1, "top");
  rightStr = hk_string_from_chars(-1, "right");
  bottomStr = hk_string_from_chars(-1, "bottom");
  layoutStr = hk_string_from_chars(-1, "layout");
  valueStr = hk_string_from_chars(-1, "value");
  offsetXStr = hk_string_from_chars(-1, "offsetX");
  offsetYStr = hk_string_from_chars(-1, "offsetY");
  advanceXStr = hk_string_from_chars(-1, "advanceX");
  imageStr = hk_string_from_chars(-1, "image");
  baseSizeStr = hk_string_from_chars(-1, "baseSize");
  glyphCountStr = hk_string_from_chars(-1, "glyphCount");
  glyphPaddingStr = hk_string_from_chars(-1, "glyphPadding");
  recsStr = hk_string_from_chars(-1, "recs");
  glyphsStr = hk_string_from_chars(-1, "glyphs");
  colorStr = hk_string_from_chars(-1, "color");
  shaderStr = hk_string_from_chars(-1, "shader");
  mapsStr = hk_string_from_chars(-1, "maps");
  paramsStr = hk_string_from_chars(-1, "params");
  vertexCountStr = hk_string_from_chars(-1, "vertexCount");
  triangleCountStr = hk_string_from_chars(-1, "triangleCount");
  verticesStr = hk_string_from_chars(-1, "vertices");
  texcoordsStr = hk_string_from_chars(-1, "texcoords");
  texcoords2Str = hk_string_from_chars(-1, "texcoords2");
  normalsStr = hk_string_from_chars(-1, "normals");
  tangentsStr = hk_string_from_chars(-1, "tangents");
  colorsStr = hk_string_from_chars(-1, "colors");
  indicesStr = hk_string_from_chars(-1, "indices");
  animVerticesStr = hk_string_from_chars(-1, "animVertices");
  animNormalsStr = hk_string_from_chars(-1, "animNormals");
  boneIdsStr = hk_string_from_chars(-1, "boneIds");
  boneWeightsStr = hk_string_from_chars(-1, "boneWeights");
  vaoIdStr = hk_string_from_chars(-1, "vaoId");
  vboIdStr = hk_string_from_chars(-1, "vboId");
  transformStr = hk_string_from_chars(-1, "transform");
  meshCountStr = hk_string_from_chars(-1, "meshCount");
  materialCountStr = hk_string_from_chars(-1, "materialCount");
  meshesStr = hk_string_from_chars(-1, "meshes");
  materialsStr = hk_string_from_chars(-1, "materials");
  meshMaterialStr = hk_string_from_chars(-1, "meshMaterial");
  boneCountStr = hk_string_from_chars(-1, "boneCount");
  bonesStr = hk_string_from_chars(-1, "bones");
  bindPoseStr = hk_string_from_chars(-1, "bindPose");
  frameCountStr = hk_string_from_chars(-1, "frameCount");
  framePosesStr = hk_string_from_chars(-1, "framePoses");
  nameStr = hk_string_from_chars(-1, "name");
  translationStr = hk_string_from_chars(-1, "translation");
  scaleStr = hk_string_from_chars(-1, "scale");
  parentStr = hk_string_from_chars(-1, "parent");
  directionStr = hk_string_from_chars(-1, "direction");
  hitStr = hk_string_from_chars(-1, "hit");
  distanceStr = hk_string_from_chars(-1, "distance");
  pointStr = hk_string_from_chars(-1, "point");
  normalStr = hk_string_from_chars(-1, "normal");
  minStr = hk_string_from_chars(-1, "min");
  maxStr = hk_string_from_chars(-1, "max");
}

static inline void int_list_from_array(HkArray *arr, int *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    list[i] = hk_is_number(val) ? (int) hk_as_number(val) : 0;
  }
}

static inline HkArray *array_from_int_list(int *list, int length)
{
  HkArray *arr = hk_array_new_with_capacity(length);
  for (int i = 0; i < length; ++i)
    hk_array_inplace_set_element(arr, i, hk_number_value(list[i]));
  return arr;
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

static inline void vector2_list_from_array(HkArray *arr, Vector2 *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    Vector2 vector = { 0, 0 };
    if (hk_is_instance(val))
      vector = vector2_from_instance(hk_as_instance(val));
    list[i] = vector;
  }
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

static inline HkInstance *instance_from_vector3(Vector3 vector)
{
  HkInstance *inst = hk_instance_new(vector3Struct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(vector.x));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(vector.y));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(vector.z));
  return inst;
}

static inline void vector3_list_from_array(HkArray *arr, Vector3 *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    Vector3 vector = { 0, 0, 0 };
    if (hk_is_instance(val))
      vector = vector3_from_instance(hk_as_instance(val));
    list[i] = vector;
  }
}

static inline Matrix matrix_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int m0i = hk_struct_index_of(ztruct, m0Str);
  int m1i = hk_struct_index_of(ztruct, m1Str);
  int m2i = hk_struct_index_of(ztruct, m2Str);
  int m3i = hk_struct_index_of(ztruct, m3Str);
  int m4i = hk_struct_index_of(ztruct, m4Str);
  int m5i = hk_struct_index_of(ztruct, m5Str);
  int m6i = hk_struct_index_of(ztruct, m6Str);
  int m7i = hk_struct_index_of(ztruct, m7Str);
  int m8i = hk_struct_index_of(ztruct, m8Str);
  int m9i = hk_struct_index_of(ztruct, m9Str);
  int m10i = hk_struct_index_of(ztruct, m10Str);
  int m11i = hk_struct_index_of(ztruct, m11Str);
  int m12i = hk_struct_index_of(ztruct, m12Str);
  int m13i = hk_struct_index_of(ztruct, m13Str);
  int m14i = hk_struct_index_of(ztruct, m14Str);
  int m15i = hk_struct_index_of(ztruct, m15Str);
  float m0 = (m0i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m0i));
  float m1 = (m1i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m1i));
  float m2 = (m2i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m2i));
  float m3 = (m3i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m3i));
  float m4 = (m4i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m4i));
  float m5 = (m5i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m5i));
  float m6 = (m6i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m6i));
  float m7 = (m7i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m7i));
  float m8 = (m8i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m8i));
  float m9 = (m9i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m9i));
  float m10 = (m10i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m10i));
  float m11 = (m11i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m11i));
  float m12 = (m12i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m12i));
  float m13 = (m13i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m13i));
  float m14 = (m14i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m14i));
  float m15 = (m15i == -1) ? 0 : (float) hk_as_number(hk_instance_get_field(inst, m15i));
  return (Matrix) { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15 };
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

static inline HkInstance *instance_from_rectangle(Rectangle rect)
{
  HkInstance *inst = hk_instance_new(rectangleStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(rect.x));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(rect.y));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(rect.width));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(rect.height));
  return inst;
}

static inline void rectangle_list_from_array(HkArray *arr, Rectangle *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    Rectangle rec = { 0, 0, 0, 0 };
    if (hk_is_instance(val))
      rec = rectangle_from_instance(hk_as_instance(val));
    list[i] = rec;
  }
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

static inline void image_list_from_array(HkArray *arr, Image *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    Image image = { NULL, 0, 0, 0, 0 };
    if (hk_is_instance(val))
      image = image_from_instance(hk_as_instance(val));
    list[i] = image;
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

static inline HkInstance *instance_from_texture(Texture texture)
{
  HkInstance *inst = hk_instance_new(textureStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(texture.id));
  hk_instance_inplace_set_field(inst, 1, hk_number_value(texture.width));
  hk_instance_inplace_set_field(inst, 2, hk_number_value(texture.height));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(texture.mipmaps));
  hk_instance_inplace_set_field(inst, 4, hk_number_value(texture.format));
  return inst;
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

static inline HkInstance *instance_from_render_texture(RenderTexture renderTexture)
{
  HkInstance *inst = hk_instance_new(renderTextureStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(renderTexture.id));
  hk_instance_inplace_set_field(inst, 1, hk_instance_value(instance_from_texture(renderTexture.texture)));
  hk_instance_inplace_set_field(inst, 2, hk_instance_value(instance_from_texture(renderTexture.depth)));
  return inst;
}

static inline GlyphInfo glyph_info_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int valuei = hk_struct_index_of(ztruct, valueStr);
  int offsetXi = hk_struct_index_of(ztruct, offsetXStr);
  int offsetYi = hk_struct_index_of(ztruct, offsetYStr);
  int advanceXi = hk_struct_index_of(ztruct, advanceXStr);
  int imagei = hk_struct_index_of(ztruct, imageStr);
  int value = (valuei == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, valuei));
  int offsetX = (offsetXi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, offsetXi));
  int offsetY = (offsetYi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, offsetYi));
  int advanceX = (advanceXi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, advanceXi));
  Image image = { NULL, 0, 0, 0, 0 };
  if (imagei != -1)
  {
    HkValue val = hk_instance_get_field(inst, imagei);
    if (hk_is_instance(val))
      image = image_from_instance(hk_as_instance(val));
  }
  return (GlyphInfo) { value, offsetX, offsetY, advanceX, image };
}

static inline void glyph_info_list_from_array(HkArray *arr, GlyphInfo *list)
{
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue val = hk_array_get_element(arr, i);
    GlyphInfo glyphInfo = { 0, 0, 0, 0, { NULL, 0, 0, 0, 0 } };
    if (hk_is_instance(val))
      glyphInfo = glyph_info_from_instance(hk_as_instance(val));
    list[i] = glyphInfo;
  }
}

static inline Font font_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int baseSizei = hk_struct_index_of(ztruct, baseSizeStr);
  int glyphCounti = hk_struct_index_of(ztruct, glyphCountStr);
  int glyphPaddingi = hk_struct_index_of(ztruct, glyphPaddingStr);
  int texturei = hk_struct_index_of(ztruct, textureStr);
  int recsi = hk_struct_index_of(ztruct, recsStr);
  int glyphsi = hk_struct_index_of(ztruct, glyphsStr);
  int baseSize = (baseSizei == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, baseSizei));
  int glyphCount = (glyphCounti == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, glyphCounti));
  int glyphPadding = (glyphPaddingi == -1) ? 0 : (int) hk_as_number(hk_instance_get_field(inst, glyphPaddingi));
  Texture texture = { 0, 0, 0, 0, 0 };
  if (texturei != -1)
  {
    HkValue val = hk_instance_get_field(inst, texturei);
    if (hk_is_instance(val))
      texture = texture_from_instance(hk_as_instance(val));
  }
  // TODO: Change to Userdata.
  Rectangle recs[LIST_CAPACITY] = { 0 };
  if (recsi != -1)
  {
    HkValue val = hk_instance_get_field(inst, recsi);
    if (hk_is_array(val))
      rectangle_list_from_array(hk_as_array(val), recs);
  }
  // TODO: Change to Userdata.
  GlyphInfo glyphs[LIST_CAPACITY] = { 0 };
  if (glyphsi != -1)
  {
    HkValue val = hk_instance_get_field(inst, glyphsi);
    if (hk_is_array(val))
      glyph_info_list_from_array(hk_as_array(val), glyphs);
  }
  return (Font) { baseSize, glyphCount, glyphPadding, texture, recs, glyphs };
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

static inline HkInstance *instance_from_camera3d(Camera3D camera)
{
  HkInstance *inst = hk_instance_new(camera3DStruct);
  hk_instance_inplace_set_field(inst, 0, hk_instance_value(instance_from_vector3(camera.position)));
  hk_instance_inplace_set_field(inst, 1, hk_instance_value(instance_from_vector3(camera.target)));
  hk_instance_inplace_set_field(inst, 2, hk_instance_value(instance_from_vector3(camera.up)));
  hk_instance_inplace_set_field(inst, 3, hk_number_value(camera.fovy));
  hk_instance_inplace_set_field(inst, 4, hk_number_value(camera.projection));
  return inst;
}

static inline Shader shader_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int idi = hk_struct_index_of(ztruct, idStr);
  int locsi = hk_struct_index_of(ztruct, locsStr);
  unsigned int id = (idi == -1) ? 0 : (unsigned int) hk_as_number(hk_instance_get_field(inst, idi));
  // TODO: Change to Userdata.
  int locs[RL_MAX_SHADER_LOCATIONS] = { 0 };
  if (locsi != -1)
  {
    HkValue val = hk_instance_get_field(inst, locsi);
    if (hk_is_array(val))
      int_list_from_array(hk_as_array(val), locs);
  }
  return (Shader) { id, locs };
}

static inline HkInstance *instance_from_shader(Shader shader)
{
  HkInstance *inst = hk_instance_new(shaderStruct);
  hk_instance_inplace_set_field(inst, 0, hk_number_value(shader.id));
  HkArray *arr = array_from_int_list(shader.locs, RL_MAX_SHADER_LOCATIONS);
  hk_instance_inplace_set_field(inst, 1, hk_array_value(arr));
  return inst;
}

static inline Ray ray_from_instance(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int positioni = hk_struct_index_of(ztruct, positionStr);
  int directioni = hk_struct_index_of(ztruct, directionStr);
  Vector3 position = (Vector3) { 0, 0, 0 };
  if (positioni != -1)
  {
    HkValue val = hk_instance_get_field(inst, positioni);
    if (hk_is_instance(val))
      position = vector3_from_instance(hk_as_instance(val));
  }
  Vector3 direction = (Vector3) { 0, 0, 0 };
  if (directioni != -1)
  {
    HkValue val = hk_instance_get_field(inst, directioni);
    if (hk_is_instance(val))
      direction = vector3_from_instance(hk_as_instance(val));
  }
  return (Ray) { position, direction };
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

static inline void load_npatch_info_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "NPatchInfo");
  nPatchInfoStruct = hk_struct_new(name);
  hk_struct_define_field(nPatchInfoStruct, sourceStr);
  hk_struct_define_field(nPatchInfoStruct, leftStr);
  hk_struct_define_field(nPatchInfoStruct, topStr);
  hk_struct_define_field(nPatchInfoStruct, rightStr);
  hk_struct_define_field(nPatchInfoStruct, bottomStr);
  hk_struct_define_field(nPatchInfoStruct, layoutStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, nPatchInfoStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_glyph_info_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "GlyphInfo");
  glyphInfoStruct = hk_struct_new(name);
  hk_struct_define_field(glyphInfoStruct, valueStr);
  hk_struct_define_field(glyphInfoStruct, offsetXStr);
  hk_struct_define_field(glyphInfoStruct, offsetYStr);
  hk_struct_define_field(glyphInfoStruct, advanceXStr);
  hk_struct_define_field(glyphInfoStruct, imageStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, glyphInfoStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_font_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Font");
  fontStruct = hk_struct_new(name);
  hk_struct_define_field(fontStruct, baseSizeStr);
  hk_struct_define_field(fontStruct, glyphCountStr);
  hk_struct_define_field(fontStruct, glyphPaddingStr);
  hk_struct_define_field(fontStruct, textureStr);
  hk_struct_define_field(fontStruct, recsStr);
  hk_struct_define_field(fontStruct, glyphsStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, fontStruct);
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

static inline void load_material_map_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "MaterialMap");
  materialMapStruct = hk_struct_new(name);
  hk_struct_define_field(materialMapStruct, textureStr);
  hk_struct_define_field(materialMapStruct, colorStr);
  hk_struct_define_field(materialMapStruct, valueStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, materialMapStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_material_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Material");
  materialStruct = hk_struct_new(name);
  hk_struct_define_field(materialStruct, shaderStr);
  hk_struct_define_field(materialStruct, mapsStr);
  hk_struct_define_field(materialStruct, paramsStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, materialStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_mesh_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Mesh");
  meshStruct = hk_struct_new(name);
  hk_struct_define_field(meshStruct, vertexCountStr);
  hk_struct_define_field(meshStruct, triangleCountStr);
  hk_struct_define_field(meshStruct, verticesStr);
  hk_struct_define_field(meshStruct, texcoordsStr);
  hk_struct_define_field(meshStruct, texcoords2Str);
  hk_struct_define_field(meshStruct, normalsStr);
  hk_struct_define_field(meshStruct, tangentsStr);
  hk_struct_define_field(meshStruct, colorsStr);
  hk_struct_define_field(meshStruct, indicesStr);
  hk_struct_define_field(meshStruct, animVerticesStr);
  hk_struct_define_field(meshStruct, animNormalsStr);
  hk_struct_define_field(meshStruct, boneIdsStr);
  hk_struct_define_field(meshStruct, boneWeightsStr);
  hk_struct_define_field(meshStruct, vaoIdStr);
  hk_struct_define_field(meshStruct, vboIdStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, meshStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_model_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Model");
  modelStruct = hk_struct_new(name);
  hk_struct_define_field(modelStruct, transformStr);
  hk_struct_define_field(modelStruct, meshCountStr);
  hk_struct_define_field(modelStruct, materialCountStr);
  hk_struct_define_field(modelStruct, meshesStr);
  hk_struct_define_field(modelStruct, materialsStr);
  hk_struct_define_field(modelStruct, meshMaterialStr);
  hk_struct_define_field(modelStruct, boneCountStr);
  hk_struct_define_field(modelStruct, bonesStr);
  hk_struct_define_field(modelStruct, bindPoseStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, modelStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_model_animation_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "ModelAnimation");
  modelAnimationStruct = hk_struct_new(name);
  hk_struct_define_field(modelAnimationStruct, boneCountStr);
  hk_struct_define_field(modelAnimationStruct, frameCountStr);
  hk_struct_define_field(modelAnimationStruct, bonesStr);
  hk_struct_define_field(modelAnimationStruct, framePosesStr);
  hk_struct_define_field(modelAnimationStruct, nameStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, modelAnimationStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_transform_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Transform");
  transformStruct = hk_struct_new(name);
  hk_struct_define_field(transformStruct, translationStr);
  hk_struct_define_field(transformStruct, rotationStr);
  hk_struct_define_field(transformStruct, scaleStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, transformStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_bone_info_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "BoneInfo");
  boneInfoStruct = hk_struct_new(name);
  hk_struct_define_field(boneInfoStruct, nameStr);
  hk_struct_define_field(boneInfoStruct, parentStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, boneInfoStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_ray_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "Ray");
  rayStruct = hk_struct_new(name);
  hk_struct_define_field(rayStruct, positionStr);
  hk_struct_define_field(rayStruct, directionStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, rayStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_ray_collision_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "RayCollision");
  rayCollisionStruct = hk_struct_new(name);
  hk_struct_define_field(rayCollisionStruct, hitStr);
  hk_struct_define_field(rayCollisionStruct, distanceStr);
  hk_struct_define_field(rayCollisionStruct, pointStr);
  hk_struct_define_field(rayCollisionStruct, normalStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, rayCollisionStruct);
  hk_return_if_not_ok(vm);
}

static inline void load_bounding_box_struct(HkVM *vm)
{
  HkString *name = hk_string_from_chars(-1, "BoundingBox");
  boundingBoxStruct = hk_struct_new(name);
  hk_struct_define_field(boundingBoxStruct, minStr);
  hk_struct_define_field(boundingBoxStruct, maxStr);
  hk_vm_push_string(vm, name);
  hk_return_if_not_ok(vm);
  hk_vm_push_struct(vm, boundingBoxStruct);
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
  // Shader management functions
  hk_vm_push_string_from_chars(vm, -1, "LoadShader");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadShader", 2, LoadShader_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LoadShaderFromMemory");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadShaderFromMemory", 2, LoadShaderFromMemory_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetShaderLocation");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetShaderLocation", 2, GetShaderLocation_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetShaderLocationAttrib");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetShaderLocationAttrib", 2, GetShaderLocationAttrib_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetShaderValue");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetShaderValue", 4, SetShaderValue_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetShaderValueV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetShaderValueV", 5, SetShaderValueV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetShaderValueMatrix");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetShaderValueMatrix", 3, SetShaderValueMatrix_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SetShaderValueTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "SetShaderValueTexture", 3, SetShaderValueTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UnloadShader");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UnloadShader", 1, UnloadShader_call);
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
  // Camera System Functions
  hk_vm_push_string_from_chars(vm, -1, "UpdateCamera");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UpdateCamera", 2, UpdateCamera_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UpdateCameraPro");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UpdateCameraPro", 4, UpdateCameraPro_call);
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
  hk_vm_push_string_from_chars(vm, -1, "DrawLineStrip");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLineStrip", 2, DrawLineStrip_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLineBezier");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLineBezier", 4, DrawLineBezier_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircle", 4, DrawCircle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleSector");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleSector", 6, DrawCircleSector_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleSectorLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleSectorLines", 6, DrawCircleSectorLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleGradient");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleGradient", 5, DrawCircleGradient_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleV", 3, DrawCircleV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleLines", 4, DrawCircleLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircleLinesV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircleLinesV", 3, DrawCircleLinesV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawEllipse");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawEllipse", 5, DrawEllipse_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawEllipseLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawEllipseLines", 5, DrawEllipseLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRing");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRing", 7, DrawRing_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRingLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRingLines", 7, DrawRingLines_call);
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
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleGradientV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleGradientV", 6, DrawRectangleGradientV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleGradientH");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleGradientH", 6, DrawRectangleGradientH_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleGradientEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleGradientEx", 5, DrawRectangleGradientEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleLines", 4, DrawRectangleLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleLinesEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleLinesEx", 3, DrawRectangleLinesEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleRounded");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleRounded", 4, DrawRectangleRounded_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleRoundedLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleRoundedLines", 4, DrawRectangleRoundedLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRectangleRoundedLinesEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRectangleRoundedLinesEx", 5, DrawRectangleRoundedLinesEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangle", 4, DrawTriangle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangleLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangleLines", 4, DrawTriangleLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangleFan");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangleFan", 2, DrawTriangleFan_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangleStrip");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangleStrip", 2, DrawTriangleStrip_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPoly");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPoly", 5, DrawPoly_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPolyLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPolyLines", 5, DrawPolyLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPolyLinesEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPolyLinesEx", 6, DrawPolyLinesEx_call);
  hk_return_if_not_ok(vm);
  // Basic shapes collision detection functions
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionRecs");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionRecs", 2, CheckCollisionRecs_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionCircles");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionCircles", 4, CheckCollisionCircles_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionCircleRec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionCircleRec", 3, CheckCollisionCircleRec_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionCircleLine");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionCircleLine", 4, CheckCollisionCircleLine_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionPointRec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionPointRec", 2, CheckCollisionPointRec_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionPointCircle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionPointCircle", 3, CheckCollisionPointCircle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionPointTriangle");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionPointTriangle", 4, CheckCollisionPointTriangle_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionPointLine");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionPointLine", 4, CheckCollisionPointLine_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionPointPoly");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionPointPoly", 2, CheckCollisionPointPoly_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CheckCollisionLines");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "CheckCollisionLines", 5, CheckCollisionLines_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "GetCollisionRec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "GetCollisionRec", 2, GetCollisionRec_call);
  hk_return_if_not_ok(vm);
  // Texture loading functions
  hk_vm_push_string_from_chars(vm, -1, "LoadTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadTexture", 1, LoadTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LoadTextureFromImage");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadTextureFromImage", 1, LoadTextureFromImage_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LoadTextureCubemap");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadTextureCubemap", 2, LoadTextureCubemap_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LoadRenderTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "LoadRenderTexture", 2, LoadRenderTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UnloadTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UnloadTexture", 1, UnloadTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UnloadRenderTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UnloadRenderTexture", 1, UnloadRenderTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UpdateTexture");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UpdateTexture", 2, UpdateTexture_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "UpdateTextureRec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "UpdateTextureRec", 3, UpdateTextureRec_call);
  hk_return_if_not_ok(vm);
  // Text drawing functions
  hk_vm_push_string_from_chars(vm, -1, "DrawFPS");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawFPS", 2, DrawFPS_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawText");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawText", 5, DrawText_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTextEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTextEx", 6, DrawTextEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTextPro");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTextPro", 8, DrawTextPro_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTextCodepoint");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTextCodepoint", 5, DrawTextCodepoint_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTextCodepoints");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTextCodepoints", 6, DrawTextCodepoints_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawLine3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawLine3D", 3, DrawLine3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPoint3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPoint3D", 2, DrawPoint3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCircle3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCircle3D", 5, DrawCircle3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangle3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangle3D", 4, DrawTriangle3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawTriangleStrip3D");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawTriangleStrip3D", 2, DrawTriangleStrip3D_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCube");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCube", 5, DrawCube_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCubeV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCubeV", 3, DrawCubeV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCubeWires");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCubeWires", 5, DrawCubeWires_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCubeWiresV");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCubeWiresV", 3, DrawCubeWiresV_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawSphere");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawSphere", 3, DrawSphere_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawSphereEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawSphereEx", 5, DrawSphereEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawSphereWires");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawSphereWires", 5, DrawSphereWires_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCylinder");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCylinder", 6, DrawCylinder_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCylinderEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCylinderEx", 6, DrawCylinderEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCylinderWires");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCylinderWires", 6, DrawCylinderWires_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCylinderWiresEx");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCylinderWiresEx", 6, DrawCylinderWiresEx_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCapsule");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCapsule", 6, DrawCapsule_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawCapsuleWires");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawCapsuleWires", 6, DrawCapsuleWires_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawPlane");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawPlane", 3, DrawPlane_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawRay");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawRay", 2, DrawRay_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "DrawGrid");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "DrawGrid", 2, DrawGrid_call);
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
  Image images[LIST_CAPACITY];
  HkArray *arr = hk_as_array(args[1]);
  image_list_from_array(arr, images);
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

static void LoadShader_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  const char *vsFileName = (const char *) hk_as_string(args[1])->chars;
  const char *fsFileName = (const char *) hk_as_string(args[2])->chars;
  Shader shader = LoadShader(vsFileName, fsFileName);
  HkInstance *inst = instance_from_shader(shader);
  hk_vm_push_instance(vm, inst);
}

static void LoadShaderFromMemory_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  const char *vsCode = (const char *) hk_as_string(args[1])->chars;
  const char *fsCode = (const char *) hk_as_string(args[2])->chars;
  Shader shader = LoadShaderFromMemory(vsCode, fsCode);
  HkInstance *inst = instance_from_shader(shader);
  hk_vm_push_instance(vm, inst);
}

static void GetShaderLocation_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  const char *uniformName = (const char *) hk_as_string(args[2])->chars;
  int result = GetShaderLocation(shader, uniformName);
  hk_vm_push_number(vm, result);
}

static void GetShaderLocationAttrib_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  const char *attribName = (const char *) hk_as_string(args[2])->chars;
  int result = GetShaderLocationAttrib(shader, attribName);
  hk_vm_push_number(vm, result);
}

static void SetShaderValue_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  int locIndex = (int) hk_as_number(args[2]);
  const void *value = (const void *) hk_as_string(args[3])->chars;
  int uniformType = (int) hk_as_number(args[4]);
  SetShaderValue(shader, locIndex, value, uniformType);
  hk_vm_push_nil(vm);
}

static void SetShaderValueV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  int locIndex = (int) hk_as_number(args[2]);
  const void *value = (const void *) hk_as_string(args[3])->chars;
  int uniformType = (int) hk_as_number(args[4]);
  int count = (int) hk_as_number(args[5]);
  SetShaderValueV(shader, locIndex, value, uniformType, count);
  hk_vm_push_nil(vm);
}

static void SetShaderValueMatrix_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  int locIndex = (int) hk_as_number(args[2]);
  Matrix mat = matrix_from_instance(hk_as_instance(args[3]));
  SetShaderValueMatrix(shader, locIndex, mat);
  hk_vm_push_nil(vm);
}

static void SetShaderValueTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  int locIndex = (int) hk_as_number(args[2]);
  Texture texture = texture_from_instance(hk_as_instance(args[3]));
  SetShaderValueTexture(shader, locIndex, texture);
  hk_vm_push_nil(vm);
}

static void UnloadShader_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Shader shader = shader_from_instance(hk_as_instance(args[1]));
  UnloadShader(shader);
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

static void UpdateCamera_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  Camera3D camera = camera3d_from_instance(hk_as_instance(args[1]));
  int mode = (int) hk_as_number(args[2]);
  UpdateCamera(&camera, mode);
  HkInstance *inst = instance_from_camera3d(camera);
  hk_vm_push_instance(vm, inst);
}

static void UpdateCameraPro_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  Camera3D camera = camera3d_from_instance(hk_as_instance(args[1]));
  Vector3 movement = vector3_from_instance(hk_as_instance(args[2]));
  Vector3 rotation = vector3_from_instance(hk_as_instance(args[3]));
  float zoom = (float) hk_as_number(args[4]);
  UpdateCameraPro(&camera, movement, rotation, zoom);
  HkInstance *inst = instance_from_camera3d(camera);
  hk_vm_push_instance(vm, inst);
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

static void DrawPixelV_call(HkVM *vm, HkValue *args)
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

static void DrawLine_call(HkVM *vm, HkValue *args)
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

static void DrawLineEx_call(HkVM *vm, HkValue *args)
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

static void DrawLineStrip_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  // TODO: Make array size dynamic.
  Vector2 points[LIST_CAPACITY];
  vector2_list_from_array(arr, points);
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawLineStrip(points, arr->length, color);
  hk_vm_push_nil(vm);
}

static void DrawLineBezier_call(HkVM *vm, HkValue *args)
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

static void DrawCircle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  int centerX = (int) hk_as_number(args[1]);
  int centerY = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawCircle(centerX, centerY, radius, color);
  hk_vm_push_nil(vm);
}

static void DrawCircleSector_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  float startAngle = (float) hk_as_number(args[3]);
  float endAngle = (float) hk_as_number(args[4]);
  int segments = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
  hk_vm_push_nil(vm);
}

static void DrawCircleSectorLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  float startAngle = (float) hk_as_number(args[3]);
  float endAngle = (float) hk_as_number(args[4]);
  int segments = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCircleSectorLines(center, radius, startAngle, endAngle, segments, color);
  hk_vm_push_nil(vm);
}

static void DrawCircleGradient_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int centerX = (int) hk_as_number(args[1]);
  int centerY = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  Color inner = color_from_instance(hk_as_instance(args[4]));
  Color outer = color_from_instance(hk_as_instance(args[5]));
  DrawCircleGradient(centerX, centerY, radius, inner, outer);
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

static void DrawCircleLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  int centerX = (int) hk_as_number(args[1]);
  int centerY = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawCircleLines(centerX, centerY, radius, color);
  hk_vm_push_nil(vm);
}

static void DrawCircleLinesV_call(HkVM *vm, HkValue *args)
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
  DrawCircleLinesV(center, radius, color);
  hk_vm_push_nil(vm);
}

static void DrawEllipse_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int centerX = (int) hk_as_number(args[1]);
  int centerY = (int) hk_as_number(args[2]);
  float radiusH = (float) hk_as_number(args[3]);
  float radiusV = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawEllipse(centerX, centerY, radiusH, radiusV, color);
  hk_vm_push_nil(vm);
}

static void DrawEllipseLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  int centerX = (int) hk_as_number(args[1]);
  int centerY = (int) hk_as_number(args[2]);
  float radiusH = (float) hk_as_number(args[3]);
  float radiusV = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawEllipseLines(centerX, centerY, radiusH, radiusV, color);
  hk_vm_push_nil(vm);
}

static void DrawRing_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 6);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 7);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float innerRadius = (float) hk_as_number(args[2]);
  float outerRadius = (float) hk_as_number(args[3]);
  float startAngle = (float) hk_as_number(args[4]);
  float endAngle = (float) hk_as_number(args[5]);
  int segments = (int) hk_as_number(args[6]);
  Color color = color_from_instance(hk_as_instance(args[7]));
  DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
  hk_vm_push_nil(vm);
}

static void DrawRingLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 6);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 7);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float innerRadius = (float) hk_as_number(args[2]);
  float outerRadius = (float) hk_as_number(args[3]);
  float startAngle = (float) hk_as_number(args[4]);
  float endAngle = (float) hk_as_number(args[5]);
  int segments = (int) hk_as_number(args[6]);
  Color color = color_from_instance(hk_as_instance(args[7]));
  DrawRingLines(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
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

static void DrawRectangleGradientV_call(HkVM *vm, HkValue *args)
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
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  int width = (int) hk_as_number(args[3]);
  int height = (int) hk_as_number(args[4]);
  Color top = color_from_instance(hk_as_instance(args[5]));
  Color bottom = color_from_instance(hk_as_instance(args[6]));
  DrawRectangleGradientV(posX, posY, width, height, top, bottom);
  hk_vm_push_nil(vm);
}

static void DrawRectangleGradientH_call(HkVM *vm, HkValue *args)
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
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  int width = (int) hk_as_number(args[3]);
  int height = (int) hk_as_number(args[4]);
  Color left = color_from_instance(hk_as_instance(args[5]));
  Color right = color_from_instance(hk_as_instance(args[6]));
  DrawRectangleGradientH(posX, posY, width, height, left, right);
  hk_vm_push_nil(vm);
}

static void DrawRectangleGradientEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  Color topLeft = color_from_instance(hk_as_instance(args[2]));
  Color bottomLeft = color_from_instance(hk_as_instance(args[3]));
  Color topRight = color_from_instance(hk_as_instance(args[4]));
  Color bottomRight = color_from_instance(hk_as_instance(args[5]));
  DrawRectangleGradientEx(rec, topLeft, bottomLeft, topRight, bottomRight);
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

static void DrawRectangleRounded_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  float roundness = (float) hk_as_number(args[2]);
  int segments = (int) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawRectangleRounded(rec, roundness, segments, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleRoundedLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  float roundness = (float) hk_as_number(args[2]);
  int segments = (int) hk_as_number(args[3]);
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawRectangleRoundedLines(rec, roundness, segments, color);
  hk_vm_push_nil(vm);
}

static void DrawRectangleRoundedLinesEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[1]));
  float roundness = (float) hk_as_number(args[2]);
  int segments = (int) hk_as_number(args[3]);
  float lineThick = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color);
  hk_vm_push_nil(vm);
}

static void DrawTriangle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 v1 = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 v2 = vector2_from_instance(hk_as_instance(args[2]));
  Vector2 v3 = vector2_from_instance(hk_as_instance(args[3]));
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawTriangle(v1, v2, v3, color);
  hk_vm_push_nil(vm);
}

static void DrawTriangleLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 v1 = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 v2 = vector2_from_instance(hk_as_instance(args[2]));
  Vector2 v3 = vector2_from_instance(hk_as_instance(args[3]));
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawTriangleLines(v1, v2, v3, color);
  hk_vm_push_nil(vm);  
}

static void DrawTriangleFan_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  // TODO: Make array size dynamic.
  Vector2 points[LIST_CAPACITY];
  vector2_list_from_array(arr, points);
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawTriangleFan(points, arr->length, color);
  hk_vm_push_nil(vm);
}

static void DrawTriangleStrip_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  // TODO: Make array size dynamic.
  Vector2 points[LIST_CAPACITY];
  vector2_list_from_array(arr, points);
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawTriangleStrip(points, arr->length, color);
  hk_vm_push_nil(vm);
}

static void DrawPoly_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  int sides = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  float rotation = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawPoly(center, sides, radius, rotation, color);
  hk_vm_push_nil(vm);
}

static void DrawPolyLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  int sides = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  float rotation = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawPolyLines(center, sides, radius, rotation, color);
  hk_vm_push_nil(vm);
}

static void DrawPolyLinesEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  int sides = (int) hk_as_number(args[2]);
  float radius = (float) hk_as_number(args[3]);
  float rotation = (float) hk_as_number(args[4]);
  float lineThick = (float) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawPolyLinesEx(center, sides, radius, rotation, lineThick, color);
  hk_vm_push_nil(vm);
}

static void CheckCollisionRecs_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Rectangle rec1 = rectangle_from_instance(hk_as_instance(args[1]));
  Rectangle rec2 = rectangle_from_instance(hk_as_instance(args[2]));
  bool result = CheckCollisionRecs(rec1, rec2);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionCircles_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 center1 = vector2_from_instance(hk_as_instance(args[1]));
  float radius1 = (float) hk_as_number(args[2]);
  Vector2 center2 = vector2_from_instance(hk_as_instance(args[3]));
  float radius2 = (float) hk_as_number(args[4]);
  bool result = CheckCollisionCircles(center1, radius1, center2, radius2);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionCircleRec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1); 
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[3]));
  bool result = CheckCollisionCircleRec(center, radius, rec);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionCircleLine_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 center = vector2_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  Vector2 p1 = vector2_from_instance(hk_as_instance(args[3]));
  Vector2 p2 = vector2_from_instance(hk_as_instance(args[4]));
  bool result = CheckCollisionCircleLine(center, radius, p1, p2);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionPointRec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Vector2 point = vector2_from_instance(hk_as_instance(args[1]));
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[2]));
  bool result = CheckCollisionPointRec(point, rec);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionPointCircle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector2 point = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 center = vector2_from_instance(hk_as_instance(args[2]));
  float radius = (float) hk_as_number(args[3]);
  bool result = CheckCollisionPointCircle(point, center, radius);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionPointTriangle_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 point = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 p1 = vector2_from_instance(hk_as_instance(args[2]));
  Vector2 p2 = vector2_from_instance(hk_as_instance(args[3]));
  Vector2 p3 = vector2_from_instance(hk_as_instance(args[4]));
  bool result = CheckCollisionPointTriangle(point, p1, p2, p3);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionPointLine_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector2 point = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 p1 = vector2_from_instance(hk_as_instance(args[2]));
  Vector2 p2 = vector2_from_instance(hk_as_instance(args[3]));
  int threshold = (int) hk_as_number(args[4]);
  bool result = CheckCollisionPointLine(point, p1, p2, threshold);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionPointPoly_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_array(vm, args, 2);
  hk_return_if_not_ok(vm);
  Vector2 point = vector2_from_instance(hk_as_instance(args[1]));
  HkArray *arr = hk_as_array(args[2]);
  // TODO: Make array size dynamic.
  Vector2 points[LIST_CAPACITY];
  vector2_list_from_array(arr, points);
  bool result = CheckCollisionPointPoly(point, points, arr->length);
  hk_vm_push_bool(vm, result);
}

static void CheckCollisionLines_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector2 startPos1 = vector2_from_instance(hk_as_instance(args[1]));
  Vector2 endPos1 = vector2_from_instance(hk_as_instance(args[2]));
  Vector2 startPos2 = vector2_from_instance(hk_as_instance(args[3]));
  Vector2 endPos2 = vector2_from_instance(hk_as_instance(args[4]));
  Vector2 collisionPoint = vector2_from_instance(hk_as_instance(args[5]));
  bool result = CheckCollisionLines(startPos1, endPos1, startPos2, endPos2, &collisionPoint);
  HkArray *arr = hk_array_new_with_capacity(2);
  hk_array_inplace_append_element(arr, hk_bool_value(result));
  hk_array_inplace_append_element(arr, hk_instance_value(instance_from_vector2(collisionPoint)));
  hk_vm_push_array(vm, arr);
}

static void GetCollisionRec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Rectangle rec1 = rectangle_from_instance(hk_as_instance(args[1]));
  Rectangle rec2 = rectangle_from_instance(hk_as_instance(args[2]));
  Rectangle result = GetCollisionRec(rec1, rec2);
  hk_vm_push_instance(vm, instance_from_rectangle(result));
}

static void LoadTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  const char *fileName = (const char *) hk_as_string(args[1])->chars;
  Texture texture = LoadTexture(fileName);
  hk_vm_push_instance(vm, instance_from_texture(texture));
}

static void LoadTextureFromImage_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Image image = image_from_instance(hk_as_instance(args[1]));
  Texture texture = LoadTextureFromImage(image);
  hk_vm_push_instance(vm, instance_from_texture(texture));
}

static void LoadTextureCubemap_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  Image image = image_from_instance(hk_as_instance(args[1]));
  int layout = (int) hk_as_number(args[2]);
  Texture texture = LoadTextureCubemap(image, layout);
  hk_vm_push_instance(vm, instance_from_texture(texture));
}

static void LoadRenderTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int width = (int) hk_as_number(args[1]);
  int height = (int) hk_as_number(args[2]);
  RenderTexture target = LoadRenderTexture(width, height);
  hk_vm_push_instance(vm, instance_from_render_texture(target));
}

static void UnloadTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  Texture texture = texture_from_instance(hk_as_instance(args[1]));
  UnloadTexture(texture);
  hk_vm_push_nil(vm);
}

static void UnloadRenderTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  RenderTexture target = render_texture_from_instance(hk_as_instance(args[1]));
  UnloadRenderTexture(target);
  hk_vm_push_nil(vm);
}

static void UpdateTexture_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  Texture texture = texture_from_instance(hk_as_instance(args[1]));
  const void *pixels = (const void *) hk_as_string(args[2])->chars;
  UpdateTexture(texture, pixels);
  hk_vm_push_nil(vm);
}

static void UpdateTextureRec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  Texture texture = texture_from_instance(hk_as_instance(args[1]));
  Rectangle rec = rectangle_from_instance(hk_as_instance(args[2]));
  const void *pixels = (const void *) hk_as_string(args[3])->chars;
  UpdateTextureRec(texture, rec, pixels);
  hk_vm_push_nil(vm);
}

static void DrawFPS_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  int posX = (int) hk_as_number(args[1]);
  int posY = (int) hk_as_number(args[2]);
  DrawFPS(posX, posY);
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

static void DrawTextEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Font font = font_from_instance(hk_as_instance(args[1]));
  const char *text = (const char *) hk_as_string(args[2])->chars;
  Vector2 position = vector2_from_instance(hk_as_instance(args[3]));
  float fontSize = (float) hk_as_number(args[4]);
  float spacing = (float) hk_as_number(args[5]);
  Color tint = color_from_instance(hk_as_instance(args[6]));
  DrawTextEx(font, text, position, fontSize, spacing, tint);
  hk_vm_push_nil(vm);
}

static void DrawTextPro_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 6);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 7);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 8);
  hk_return_if_not_ok(vm);
  Font font = font_from_instance(hk_as_instance(args[1]));
  const char *text = (const char *) hk_as_string(args[2])->chars;
  Vector2 position = vector2_from_instance(hk_as_instance(args[3]));
  Vector2 origin = vector2_from_instance(hk_as_instance(args[4]));
  float rotation = (float) hk_as_number(args[5]);
  float fontSize = (float) hk_as_number(args[6]);
  float spacing = (float) hk_as_number(args[7]);
  Color tint = color_from_instance(hk_as_instance(args[8]));
  DrawTextPro(font, text, position, origin, rotation, fontSize, spacing, tint);
  hk_vm_push_nil(vm);
}

static void DrawTextCodepoint_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Font font = font_from_instance(hk_as_instance(args[1]));
  int codepoint = (int) hk_as_number(args[2]);
  Vector2 position = vector2_from_instance(hk_as_instance(args[3]));
  float fontSize = (float) hk_as_number(args[4]);
  Color tint = color_from_instance(hk_as_instance(args[5]));
  DrawTextCodepoint(font, codepoint, position, fontSize, tint);
  hk_vm_push_nil(vm);
}

static void DrawTextCodepoints_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_array(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Font font = font_from_instance(hk_as_instance(args[1]));
  // TODO: Make array size dynamic.
  int codepoints[LIST_CAPACITY];
  HkArray *arr = hk_as_array(args[2]);
  int_list_from_array(arr, codepoints);
  Vector2 position = vector2_from_instance(hk_as_instance(args[3]));
  float fontSize = (float) hk_as_number(args[4]);
  float spacing = (float) hk_as_number(args[5]);
  Color tint = color_from_instance(hk_as_instance(args[6]));
  DrawTextCodepoints(font, codepoints, arr->length, position, fontSize, spacing, tint);
  hk_vm_push_nil(vm);
}

static void DrawLine3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector3 startPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 endPos = vector3_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawLine3D(startPos, endPos, color);
  hk_vm_push_nil(vm);
}

static void DrawPoint3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawPoint3D(position, color);
  hk_vm_push_nil(vm);
}

static void DrawCircle3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector3 center = vector3_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  Vector3 rotationAxis = vector3_from_instance(hk_as_instance(args[3]));
  float rotationAngle = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawCircle3D(center, radius, rotationAxis, rotationAngle, color);
  hk_vm_push_nil(vm);
}

static void DrawTriangle3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 4);
  hk_return_if_not_ok(vm);
  Vector3 v1 = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 v2 = vector3_from_instance(hk_as_instance(args[2]));
  Vector3 v3 = vector3_from_instance(hk_as_instance(args[3]));
  Color color = color_from_instance(hk_as_instance(args[4]));
  DrawTriangle3D(v1, v2, v3, color);
  hk_vm_push_nil(vm);
}

static void DrawTriangleStrip3D_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  // TODO: Make array size dynamic.
  Vector3 points[LIST_CAPACITY];
  vector3_list_from_array(arr, points);
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawTriangleStrip3D(points, arr->length, color);
  hk_vm_push_nil(vm);
}

static void DrawCube_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  float width = (float) hk_as_number(args[2]);
  float height = (float) hk_as_number(args[3]);
  float length = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawCube(position, width, height, length, color);
  hk_vm_push_nil(vm);
}

static void DrawCubeV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 size = vector3_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawCubeV(position, size, color);
  hk_vm_push_nil(vm);
}

static void DrawCubeWires_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  float width = (float) hk_as_number(args[2]);
  float height = (float) hk_as_number(args[3]);
  float length = (float) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawCubeWires(position, width, height, length, color);
  hk_vm_push_nil(vm);
}

static void DrawCubeWiresV_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 size = vector3_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawCubeWiresV(position, size, color);
  hk_vm_push_nil(vm);
}

static void DrawSphere_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector3 centerPos = vector3_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawSphere(centerPos, radius, color);
  hk_vm_push_nil(vm);
}

static void DrawSphereEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector3 centerPos = vector3_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  int rings = (int) hk_as_number(args[3]);
  int slices = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawSphereEx(centerPos, radius, rings, slices, color);
  hk_vm_push_nil(vm);
}

static void DrawSphereWires_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 5);
  hk_return_if_not_ok(vm);
  Vector3 centerPos = vector3_from_instance(hk_as_instance(args[1]));
  float radius = (float) hk_as_number(args[2]);
  int rings = (int) hk_as_number(args[3]);
  int slices = (int) hk_as_number(args[4]);
  Color color = color_from_instance(hk_as_instance(args[5]));
  DrawSphereWires(centerPos, radius, rings, slices, color);
  hk_vm_push_nil(vm);
}

static void DrawCylinder_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  float radiusTop = (float) hk_as_number(args[2]);
  float radiusBottom = (float) hk_as_number(args[3]);
  float height = (float) hk_as_number(args[4]);
  int slices = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCylinder(position, radiusTop, radiusBottom, height, slices, color);
  hk_vm_push_nil(vm);
}

static void DrawCylinderEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 startPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 endPos = vector3_from_instance(hk_as_instance(args[2]));
  float startRadius = (float) hk_as_number(args[3]);
  float endRadius = (float) hk_as_number(args[4]);
  int sides = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCylinderEx(startPos, endPos, startRadius, endRadius, sides, color);
  hk_vm_push_nil(vm);
}

static void DrawCylinderWires_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 position = vector3_from_instance(hk_as_instance(args[1]));
  float radiusTop = (float) hk_as_number(args[2]);
  float radiusBottom = (float) hk_as_number(args[3]);
  float height = (float) hk_as_number(args[4]);
  int slices = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCylinderWires(position, radiusTop, radiusBottom, height, slices, color);
  hk_vm_push_nil(vm);
}

static void DrawCylinderWiresEx_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 startPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 endPos = vector3_from_instance(hk_as_instance(args[2]));
  float startRadius = (float) hk_as_number(args[3]);
  float endRadius = (float) hk_as_number(args[4]);
  int sides = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCylinderWiresEx(startPos, endPos, startRadius, endRadius, sides, color);
  hk_vm_push_nil(vm);
}

static void DrawCapsule_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 startPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 endPos = vector3_from_instance(hk_as_instance(args[2]));
  float radius = (float) hk_as_number(args[3]);
  int slices = (int) hk_as_number(args[4]);
  int rings = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCapsule(startPos, endPos, radius, slices, rings, color);
  hk_vm_push_nil(vm);
}

static void DrawCapsuleWires_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 5);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 6);
  hk_return_if_not_ok(vm);
  Vector3 startPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector3 endPos = vector3_from_instance(hk_as_instance(args[2]));
  float radius = (float) hk_as_number(args[3]);
  int slices = (int) hk_as_number(args[4]);
  int rings = (int) hk_as_number(args[5]);
  Color color = color_from_instance(hk_as_instance(args[6]));
  DrawCapsuleWires(startPos, endPos, radius, slices, rings, color);
  hk_vm_push_nil(vm);
}

static void DrawPlane_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 3);
  hk_return_if_not_ok(vm);
  Vector3 centerPos = vector3_from_instance(hk_as_instance(args[1]));
  Vector2 size = vector2_from_instance(hk_as_instance(args[2]));
  Color color = color_from_instance(hk_as_instance(args[3]));
  DrawPlane(centerPos, size, color);
  hk_vm_push_nil(vm);
}

static void DrawRay_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_instance(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_instance(vm, args, 2);
  hk_return_if_not_ok(vm);
  Ray ray = ray_from_instance(hk_as_instance(args[1]));
  Color color = color_from_instance(hk_as_instance(args[2]));
  DrawRay(ray, color);
  hk_vm_push_nil(vm);
}

static void DrawGrid_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  int slices = (int) hk_as_number(args[1]);
  float spacing = (float) hk_as_number(args[2]);
  DrawGrid(slices, spacing);
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
  load_npatch_info_struct(vm);
  load_glyph_info_struct(vm);
  load_font_struct(vm);
  load_camera2D_struct(vm);
  load_camera3D_struct(vm);
  load_shader_struct(vm);
  load_material_map_struct(vm);
  load_material_struct(vm);
  load_mesh_struct(vm);
  load_model_struct(vm);
  load_model_animation_struct(vm);
  load_transform_struct(vm);
  load_bone_info_struct(vm);
  load_ray_struct(vm);
  load_ray_collision_struct(vm);
  load_bounding_box_struct(vm);
  load_colors(vm);
  load_functions(vm);
  hk_vm_construct(vm, 225);
}
