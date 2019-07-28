precision mediump float;

varying vec2 frag_texcoord;

uniform sampler2D texture_sampler;

void main() {
  gl_FragColor = texture2D(texture_sampler, frag_texcoord);
}
