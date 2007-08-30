// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
