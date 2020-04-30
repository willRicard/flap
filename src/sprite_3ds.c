#include "sprite_impl.h"

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>

#include "assets_3ds.h"
#include "atlas_t3x.h"
#include "sprite_shbin.h"

static DVLB_s *vshader_dvlb = NULL;
static shaderProgram_s program;

static C3D_Tex texture;

static u8 *vbo_data = NULL;

void sprite_update(void) {
  memcpy(vbo_data, vertices, sizeof(vertices));
  C3D_DrawElements(GPU_TRIANGLES, kIndicesPerSprite * kNumSprites,
                   C3D_UNSIGNED_SHORT, indices);
  printf("[ok] draw\n");
}

void sprite_init() {
  assets_3ds_create_shader((u32 *)sprite_shbin, sprite_shbin_size, vshader_dvlb,
                           &program);
  C3D_BindProgram(&program);
  printf("[ok] shader\n");

  C3D_AttrInfo *attr_info = C3D_GetAttrInfo();
  AttrInfo_Init(attr_info);
  AttrInfo_AddLoader(attr_info, 0, GPU_FLOAT, 2); // v0=position
  AttrInfo_AddLoader(attr_info, 1, GPU_FLOAT, 2); // v1=texcoord

  assets_3ds_create_texture((u32 *)atlas_t3x, atlas_t3x_size, &texture);
  C3D_TexSetFilter(&texture, GPU_NEAREST, GPU_NEAREST);
  C3D_TexBind(0, &texture);

  C3D_TexEnv *env = C3D_GetTexEnv(0);
  C3D_TexEnvInit(env);
  C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
  C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
  printf("[ok] texture\n");

  vbo_data = linearAlloc(sizeof(vertices));
  memcpy(vbo_data, vertices, sizeof(vertices));

  C3D_BufInfo *buf_info = C3D_GetBufInfo();
  BufInfo_Init(buf_info);
  BufInfo_Add(buf_info, vbo_data, sizeof(vertices), 3, 0x210);
  printf("[ok] vbo\n");
}

void sprite_quit() {
  DVLB_Free(vshader_dvlb);
  shaderProgramFree(&program);
}
