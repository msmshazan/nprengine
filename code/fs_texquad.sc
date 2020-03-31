$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_texture,  0);

void main()
{
        vec2 uv = floor(v_texcoord0) + 0.5;
	uv += clamp((1.0 - fract(v_texcoord0)) , 0.0, 1.0);
	gl_FragColor =  texture2D( s_texture, v_texcoord0);
}
