#ifndef FLAP_SHADER_H_
#define FLAP_SHADER_H_

typedef struct flap_Shader flap_Shader;

flap_Shader *flap_shader_new(const char *vertex_shader_source,
                             const char *fragment_shader_source,
                             const char *geometry_shader_source);

void flap_shader_free(flap_Shader *shader);

void flap_shader_use(flap_Shader *shader);

#endif // FLAP_SHADER_H_
