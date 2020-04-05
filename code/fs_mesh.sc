$input v_pos, v_view, v_normal

/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

uniform vec4 u_highlightcolor;
uniform vec4 u_surfacecolor;
uniform vec4 u_coolfactor;
uniform vec4 u_warmfactor;

void main()
{
        vec3 lightDir = vec3(0.0, -1.0, 0.0);
	vec3 normal = normalize(v_normal);
	vec3 view = normalize(v_view);
        float ndotl = dot(normal, lightDir);
	vec3 reflected = 2.0*ndotl*normal - lightDir;
	float rdotv = dot(reflected,view);
        float specular = clamp(100.0*rdotv - 97.0,0.0,1.0);
        vec3 highlightcolor = u_highlightcolor.rgb;
        vec3 surfacecolor = u_surfacecolor.rgb;
        vec3 warmcolor = u_warmfactor.rgb + u_warmfactor.a * surfacecolor;
        vec3 coolcolor = u_coolfactor.rgb + u_coolfactor.a * surfacecolor;
        float factor = 0.5;
	vec3 colorfactor =  mix(warmcolor,coolcolor,factor);
	vec3 outcolor = mix(highlightcolor,colorfactor ,specular);
        gl_FragColor.xyz = outcolor.xyz;
	gl_FragColor.w = 1.0;
}
