varying float alpha;
uniform float scale;
uniform float exponent;


void main(void) 
{
   // use normal to find pleasant alpha value
   vec3 vsNormal = gl_NormalMatrix * gl_Normal;
   alpha = 1.0 - abs(vsNormal.z);

   // Multiply alpha by itself or raise it by the given exponent.
   alpha = pow(alpha, exponent);

   // move vertex out along normal
   vec4 new_vert = gl_Vertex + vec4(scale * normalize(gl_Normal), 0.0);

   gl_Position = gl_ModelViewProjectionMatrix * new_vert;
}
