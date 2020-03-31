$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_texture,  0);
SAMPLER2D(l_texture,  1);

uniform vec4 tile_size;
uniform vec4 tex_sizeinv;

void main()
{
        vec2 uv = floor(tile_size.xy*v_texcoord0) + 0.5;
	uv  = uv / tile_size.xy;
        vec2 lookup = texture2D(l_texture,uv).xy*255.0;
	vec2 samplebase = lookup*tile_size.zw;
        vec2 samplefrac = ( v_texcoord0*tile_size.xy*tile_size.zw ) % tile_size.zw;
        vec2 samplepos = (samplebase + samplefrac)*(tex_sizeinv.xy);
        vec4 pixel = texture2D(s_texture,samplepos);
	gl_FragColor =  pixel;
}
