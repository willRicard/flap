#ifndef FLAP_PIPELINE_H_
#define FLAP_PIPELINE_H_

/** 
 * A pipeline holds a GPU program used for drawing.
 */
typedef struct Pipeline Pipeline;

Pipeline pipeline_create(const char *vertex_shader_source,
                         const char *fragment_shader_source);

void pipeline_destroy(Pipeline pipeline);

#endif // FLAP_PIPELINE_H_
