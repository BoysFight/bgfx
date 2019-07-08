$input v_texcoord0

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"
SAMPLER2D(s_texColor, 0);
void main()
{
	vec4 color = texture2D(s_texColor, v_texcoord0);
	gl_FragColor = vec4(vec3(color.r + color.g + color.b) / 3.0, color.a);
}
