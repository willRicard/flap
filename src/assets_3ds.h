#pragma once

#include <3ds.h>
#include <citro3d.h>

void assets_3ds_create_shader(u32 *shader_shbin, u32 size, DVLB_s *vshader_dvlb,
                              shaderProgram_s *program);

void assets_3ds_create_texture(u32 *texture_t3x, u32 size, C3D_Tex *texture);
