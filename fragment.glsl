/****************************
 Filename: fragment.glsl
 This is a pass-through fragment shader.
*****************************/

varying float pos_y;

void main()
{
    gl_FragColor = gl_Color;

	if(pos_y < 0.1)
		discard;
}
