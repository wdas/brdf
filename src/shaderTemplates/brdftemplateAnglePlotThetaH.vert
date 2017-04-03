/*
Copyright Disney Enterprises, Inc. All rights reserved.

This license governs use of the accompanying software. If you use the software, you
accept this license. If you do not accept the license, do not use the software.

1. Definitions
The terms "reproduce," "reproduction," "derivative works," and "distribution" have
the same meaning here as under U.S. copyright law. A "contribution" is the original
software, or any additions or changes to the software. A "contributor" is any person
that distributes its contribution under this license. "Licensed patents" are a
contributor's patent claims that read directly on its contribution.

2. Grant of Rights
(A) Copyright Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free copyright license to reproduce its contribution, prepare
derivative works of its contribution, and distribute its contribution or any derivative
works that you create.
(B) Patent Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free license under its licensed patents to make, have made,
use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the
software or derivative works of the contribution in the software.

3. Conditions and Limitations
(A) No Trademark License- This license does not grant you rights to use any
contributors' name, logo, or trademarks.
(B) If you bring a patent claim against any contributor over patents that you claim
are infringed by the software, your patent license from such contributor to the
software ends automatically.
(C) If you distribute any portion of the software, you must retain all copyright,
patent, trademark, and attribution notices that are present in the software.
(D) If you distribute any portion of the software in source code form, you may do
so only under this license by including a complete copy of this license with your
distribution. If you distribute any portion of the software in compiled or object code
form, you may only do so under a license that complies with this license.
(E) The software is licensed "as-is." You bear the risk of using it. The contributors
give no express warranties, guarantees or conditions. You may have additional
consumer rights under your local laws which this license cannot change.
To the extent permitted under your local laws, the contributors exclude the
implied warranties of merchantability, fitness for a particular purpose and non-
infringement.
*/


#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform vec3 incidentVector;
uniform float incidentTheta;
uniform float incidentPhi;
uniform float useLogPlot;
uniform float useNDotL;
uniform vec3 colorMask;
uniform float thetaD;

in vec3 vtx_position;

::INSERT_UNIFORMS_HERE::


::INSERT_BRDF_FUNCTION_HERE::


float modifyLog( float x )
{
    // log base 10
    return log(x + 1.0) * 0.434294482;
}

vec3 rotate( vec3 v, vec3 axis, float angle )
{
    vec3 n;
    axis = normalize( axis );
    n = axis * dot( axis, v );
    return n + cos(angle)*(v-n) + sin(angle)*cross(axis, v);
}

void main(void)
{
    float thetaH = vtx_position.x;

    vec3 N = vec3(0,0,1); // normal
    vec3 X = vec3(1,0,0); // tangent
    vec3 Y = vec3(0,1,0); // bitangent

    // synthesize L and V vectors
    vec3 L = rotate(rotate(N, X, thetaD), Y, thetaH);
    vec3 H = rotate(N, Y, thetaH);
    vec3 V = 2*dot(L,H)*H - L;

    // calculate the value of the BRDF at this output vector
    vec3 bRes = BRDF( L, V, N, X, Y );
    float b = dot( bRes, colorMask );
    b *= (useNDotL > 0.5 ? dot( N,L ) : 1.0);
    b = useLogPlot > 0.5 ? modifyLog( b ) : b;

    // plot curve, apply pan and zoom
    gl_Position = projectionMatrix * modelViewMatrix * vec4( thetaH, b, 0, 1 );
}

