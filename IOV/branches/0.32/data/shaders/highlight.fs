varying float alpha;
uniform vec3 color;


void main(void)
{
    gl_FragColor = vec4(color, alpha);
}
