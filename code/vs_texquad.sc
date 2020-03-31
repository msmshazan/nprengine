$input a_position, a_texcoord0
$output v_texcoord0

#include "common.sh"

uniform vec4 tex_sizeinv;

void main()
{
        a_position.xy *= u_viewTexel.xy;
	a_position.xy *= 2;
	a_position.xy -= vec2(1,1);
	a_texcoord0 = a_texcoord0 * tex_sizeinv.xy;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_texcoord0 = a_texcoord0;
}
