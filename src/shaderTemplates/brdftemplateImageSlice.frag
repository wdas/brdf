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

uniform float useNDotL;
uniform float incidentPhi;
uniform float phiD;
uniform float brightness;
uniform float gamma;
uniform float exposure;
uniform float showChroma;
uniform float useThetaHSquared;

in vec2 texCoord;

out vec4 fragColor;

::INSERT_UNIFORMS_HERE::

::INSERT_BRDF_FUNCTION_HERE::



float Sqr( float x )
{
    return x * x;
}

vec3 rotate_vector( vec3 v, vec3 axis, float angle )
{
    vec3 n;
    axis = normalize( axis );
    n = axis * dot( axis, v );
    return n + cos(angle)*(v-n) + sin(angle)*cross(axis, v);
}

void main()
{
    // orthonormal vectors
    vec3 normal = vec3(0,0,1);
    vec3 tangent = vec3(1,0,0);
    vec3 bitangent = vec3(0,1,0);

    // thetaH and thetaD vary from [0 - pi/2]
    const float M_PI = 3.1415926535897932384626433832795;

    float thetaH = texCoord.r;
    if (useThetaHSquared != 0) thetaH = Sqr(thetaH) / (M_PI * 0.5);

    float thetaD = texCoord.g;

    // compute H from thetaH,phiH where (phiH = incidentPhi)
    float phiH = incidentPhi;
    float sinThetaH = sin(thetaH), cosThetaH = cos(thetaH);
    float sinPhiH = sin(phiH), cosPhiH = cos(phiH);
    vec3 H = vec3(sinThetaH*cosPhiH, sinThetaH*sinPhiH, cosThetaH );

    // compute D from thetaD,phiD
    float sinThetaD = sin(thetaD), cosThetaD = cos(thetaD);
    float sinPhiD = sin(phiD), cosPhiD = cos(phiD);
    vec3 D = vec3(sinThetaD*cosPhiD, sinThetaD*sinPhiD, cosThetaD );

    // compute L by rotating D into incident frame
    vec3 L = rotate_vector( rotate_vector( D, bitangent, thetaH ),
                            normal, phiH );

    // compute V by reflecting L across H
    vec3 V = 2*dot(H,L)*H - L;

    vec3 b = BRDF( L, V, normal, tangent, bitangent );

    // apply N . L
    if (useNDotL != 0) b *= clamp(L[2],0,1);

    if (showChroma != 0) {
        float norm = max(b[0],max(b[1],b[2]));
        if (norm > 0) b /= norm;
    }

    // brightness
    b *= brightness;

    // exposure
    b *= pow( 2.0, exposure );

    // gamma
    b = pow( b, vec3( 1.0 / gamma ) );

    fragColor = vec4( clamp( b, vec3(0.0), vec3(1.0) ), 1.0 );
}
