#include "assets_3ds.h"

#include <tex3ds.h>

#include "window.h"

void assets_3ds_create_shader(u32 *shader_shbin, u32 size, DVLB_s *vshader_dvlb,
                              shaderProgram_s *program) {
  vshader_dvlb = DVLB_ParseFile((u32 *)shader_shbin, size);
  if (vshader_dvlb == NULL) {
    window_fail_with_error("Assets: Could not parse shader binary");
  }
  shaderProgramInit(program);
  shaderProgramSetVsh(program, &vshader_dvlb->DVLE[0]);
}

void assets_3ds_create_texture(u32 *texture_t3x, u32 size, C3D_Tex *texture) {
  Tex3DS_Texture t3x =
      Tex3DS_TextureImport(texture_t3x, size, texture, NULL, false);
  if (!t3x) {
    window_fail_with_error("Assets: Could not load texture from memory");
  }
  Tex3DS_TextureFree(t3x);
}
