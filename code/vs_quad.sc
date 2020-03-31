$input a_position, a_color0
$output v_color0

#include "common.sh"

void main()
{
        a_position.xy *= u_viewTexel.xy;
	a_position.xy *= 2;
	a_position.xy -= vec2(1,1);
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_color0 = a_color0;
}
