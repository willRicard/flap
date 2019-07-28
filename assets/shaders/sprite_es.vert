precision mediump float;

attribute vec4 in_vertex;

varying vec2 frag_texcoord;

void main() {
  gl_Position = vec4(in_vertex.xy, 0.0, 1.0);
  gl_Position.y *= -1.0;
  frag_texcoord = in_vertex.zw;
}
