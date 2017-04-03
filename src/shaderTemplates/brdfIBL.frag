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

uniform vec3 incidentVector;
uniform float incidentTheta;
uniform float incidentPhi;
uniform int passNumber;
uniform int stepSize;
uniform int totalSamples;

uniform float brightness;
uniform float gamma;
uniform float exposure;
uniform float useNDotL;
uniform float renderWithIBL;
uniform float useIBLImportance;
uniform float useBRDFImportance;
uniform float useMIS;
uniform samplerCube envCube;

uniform mat4  envRotMatrix;
uniform mat4  envRotMatrixInverse;

uniform sampler2D probTex;
uniform sampler2D marginalProbTex;
uniform vec2 texDims;

// probability textures:
// R component: PDF
// G component: CDF
// B component: inverse CDF

in vec3 eyeSpaceNormal;
in vec3 eyeSpaceTangent;
in vec3 eyeSpaceBitangent;
in vec4 eyeSpaceVert;

out vec4 fragColor;

::INSERT_UNIFORMS_HERE::

::INSERT_BRDF_FUNCTION_HERE::

::INSERT_IS_FUNCTION_HERE::


uint iSamples = 16u;
float fSamples = 16.0;
float fInvSamples = 1.0 / fSamples;


// globals
vec3 esNormal, esTangent, esBitangent, viewVec;
mat3 LocalToWorld, WorldToLocal;
vec3 tsSampleDir, tsViewVec;
mat3 envSampleRotMatrix;



vec3 uvToVector( vec2 uv )
{
    vec3 rv = vec3(0.0);

    // face order is px nx py ny pz nz
    float face = floor( uv.x*6.0 );
    uv.x = uv.x*6.0 - face;
    uv = uv * 2.0 - vec2(1.0);

    // x
    if( face < 1.5 )
    {
        // s = 1 (for +x) or -1 (for -x)
        float s = sign(0.5 - face);
        rv = vec3(s, uv.y, -s*uv.x);
    }

    // y
    else if( face < 3.5 )
    {
        float s = sign(2.5 - face);
        rv = vec3(uv.x, s, -s*uv.y);
    }

    // z
    else
    {
        float s = sign(4.5 - face);
        rv = vec3(s*uv.x, uv.y, s);
    }

    // note: vector is not normalized so that the length can be determined later
    // to compute projected solid angle at cube position
    return rv;
}




vec2 vectorToUV( vec3 rsReflVector )
{
    vec2 uv, mapFace;

    float face;

    // compare the components of the transformed reflection vector to
    // figure out which one is the largest
    vec3 a = abs(rsReflVector);

    // x vector?
    if( a.x >= a.y && a.x >= a.z)
    {
        uv = -rsReflVector.zy / rsReflVector.x;
        uv.y *= -sign(rsReflVector.x);
        face = 0.0 + step(rsReflVector.x,0);
    }

    // y vector?
    else if( a.y >= a.x && a.z >= a.x)
    {
        uv = -rsReflVector.xz / rsReflVector.y;
        uv.x *= -sign(rsReflVector.y);
        face = 2.0 + step(rsReflVector.y,0);
    }

    // z vector?
    else
    {
        uv = rsReflVector.xy / rsReflVector.z;
        uv.y *= sign(rsReflVector.z);
        face = 4.0 + step(rsReflVector.z,0);
    }

    // transform the sampling location into [0..1]
    uv = uv * 0.5 + vec2(0.5);

    // transform the coordinate so we're reading from the correct envRotMatrixface
    uv.x = (uv.x + face) * (1.0/6);
    return uv;
}


uint hash(uint x, uint y)
{
    const uint M = 1664525u, C = 1013904223u;
    uint seed = (x * M + y + C) * M;
    // tempering (from Matsumoto)
    seed ^= (seed >> 11u);
    seed ^= (seed << 7u) & 0x9d2c5680u;
    seed ^= (seed << 15u) & 0xefc60000u;
    seed ^= (seed >> 18u);
    return seed;
}


float hammersleySample(uint bits, uint seed)
{
    bits = ( bits << 16u) | ( bits >> 16u);
    bits = ((bits & 0x00ff00ffu) << 8u) | ((bits & 0xff00ff00u) >> 8u);
    bits = ((bits & 0x0f0f0f0fu) << 4u) | ((bits & 0xf0f0f0f0u) >> 4u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xccccccccu) >> 2u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xaaaaaaaau) >> 1u);
    bits ^= seed;
    return float(bits) * 2.3283064365386963e-10; // divide by 1<<32
}


float warpSample1D( sampler2D tex, float texDim, float u, float v, out float probInv )
{
    float invTexDim = 1/texDim;

    // evaluate approximate inverse cdf
    // Note: cvs are at pixel centers with implied end points at (0,0) and (1,1)
    // data[0] corresponds to u = 0.5/texDim
    float uN = u * texDim - 0.5;
    float ui = floor(uN);
    float frac = uN - ui;
    // segment spanning u is data[ui] and data[ui+1]
    // sample texture at texel centers (data[ui] = texture((ui+.5)/texDim))
    float t0 = (ui+.5)*invTexDim, t1 = (ui+1.5)*invTexDim;

    float cdf0 = t0 < 0 ? // data[-1] is -data[0]  (reflect around (0,0))
        -texture( tex, vec2(.5 * invTexDim, v) ).r :
        texture( tex, vec2(t0, v) ).r;
    float cdf1 = t1 > 1 ? // data[texDim] = 2-data[texDim-1]  (reflect around (1,1))
        2-texture( tex, vec2(1 - .5 * invTexDim, v) ).r :
        texture( tex, vec2(t1, v) ).r;

    // infer 1/pdf from slope of inverse cdf
    probInv = (texDim * (cdf1 - cdf0));

    // linear interp cdf values to get warped sample
    float uPrime =  cdf0 + frac * (cdf1 - cdf0);

    return uPrime;
}


vec2 warpSample( vec2 uv, out float probInv )
{
    float uProbInv, vProbInv;
    float vPrime = warpSample1D(marginalProbTex, texDims.y, uv.y, 0.5, vProbInv);
    float uPrime = warpSample1D(probTex, texDims.x, uv.x, vPrime, uProbInv);

    probInv = uProbInv * vProbInv;
    return vec2(uPrime, vPrime);
}


vec4 envMapSample( float u, float v )
{
    float probInv = 1;
    vec2 uv = vec2(u,v);

    if (useIBLImportance > .5)
        uv = warpSample( uv, probInv ); // will overwrite prob with actual pdf value

    vec3 esSampleDir = uvToVector( uv );
    // TODO - precompute LocalToWorld * envRotMatrixInverse
    vec3 tsSampleDir = normalize(LocalToWorld * mat3(envRotMatrixInverse) * esSampleDir);

    // cosine weight
    float cosine = max(0,dot( tsSampleDir, vec3(0,0,1)));
    if (cosine <= 0) return vec4(vec3(0), 1.0);

    // since we're working in tangent space, the basis vectors can be nice and easy and hardcoded
    vec3 brdf = max( BRDF( tsSampleDir, tsViewVec, vec3(0,0,1), vec3(1,0,0), vec3(0,1,0) ), vec3(0.0) );

    // sample env map
    vec3 envSample = textureLod( envCube, esSampleDir, 0 ).rgb;

    // dA (area of cube) = (6*2*2)/N  (Note: divide by N happens later)
    // dw = dA / r^3 = 24 * pow(x*x + y*y + z*z, -1.5) (see pbrt v2 p 947).
    float dw = 24 * pow(esSampleDir.x*esSampleDir.x +
                        esSampleDir.y*esSampleDir.y +
                        esSampleDir.z*esSampleDir.z, -1.5);

    vec3 result = envSample*brdf*(probInv*cosine*dw);

    // hack - clamp outliers to eliminate hot spot samples
    if (useIBLImportance > .5)
        result = min(result, 50);

    return vec4(result, 1.0 );
}


vec4 computeIBL()
{
    vec4 result = vec4(0.0);
    uint seed1 = hash(uint(gl_FragCoord.x), uint(gl_FragCoord.y));
    uint seed2 = hash(seed1, 1000u);

    float inv = 1.0 / float(totalSamples);
    float bigInv = inv * float(stepSize);

    float u = float(seed1) * 2.3283064365386963e-10;

    // importance sample the BRDF
    if( useBRDFImportance != 0.0 )
    {
        for( uint i = uint(passNumber); i < uint(totalSamples); i += uint(stepSize) )
        {
            float uu = fract( u + i * inv);
            float vv = fract( hammersleySample(i, seed2) );

            // choose a sample from the environment map
            //result += envMapSample( uu, vv );
        }

        result = vec4(0,1,0,1);
    }

    // multiple importance sampling
    else if( useMIS != 0.0 )
    {
        for( uint i = uint(passNumber); i < uint(totalSamples); i += uint(stepSize) )
        {
            float uu = fract( u + i * inv);
            float vv = fract( hammersleySample(i, seed2) );

            // choose a sample from the environment map
            //result += envMapSample( uu, vv );
        }

        result = vec4(1,0,0,1);
    }

    // importance sample the IBL, or don't importance sample at all
    else
    {
        for( uint i = uint(passNumber); i < uint(totalSamples); i += uint(stepSize) )
        {
            float uu = fract( u + i * inv);
            float vv = fract( hammersleySample(i, seed2) );

            // choose a sample from the environment map
            result += envMapSample( uu, vv );
        }
    }

    return result;
}




void main(void)
{
    esNormal = normalize( eyeSpaceNormal );
    esTangent = normalize( eyeSpaceTangent );
    esBitangent = normalize( eyeSpaceBitangent );
    //viewVec = -normalize( eyeSpaceVert );
    viewVec = vec3(0,0,1);

    WorldToLocal = mat3( esTangent, esBitangent, esNormal );
    LocalToWorld = transpose(WorldToLocal);

    envSampleRotMatrix = mat3(envRotMatrix) * WorldToLocal;

    tsSampleDir = normalize( incidentVector );
    tsViewVec = LocalToWorld * viewVec;


    vec4 result = vec4(0.0);

    // render with just a single directional light?
    if( renderWithIBL < 0.5 )
    {
        vec3 b = max( BRDF( tsSampleDir, viewVec, esNormal, esTangent, esBitangent ), vec3(0.0) );
        b *= dot( tsSampleDir, esNormal );
        result = vec4( b, 1.0 );
    }

    // or render samples from an IBL?
    else
    {
        result = computeIBL();
    }

    fragColor = result;
}
