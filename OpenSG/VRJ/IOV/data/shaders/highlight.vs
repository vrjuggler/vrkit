varying float alpha;


void main(void) 
{
   // use normal to find pleasant alpha value
   const vec3 vsNormal = gl_NormalMatrix * gl_Normal;
   alpha = 1.0 - abs(vsNormal.z);

   // move vertex out along normal
   const vec4 new_vert = gl_Vertex + vec4(gl_Normal, 1.0);

   gl_Position = gl_ModelViewProjectionMatrix * new_vert;
}
