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

uniform float brightness;
uniform float gamma;
uniform float exposure;
uniform float useNDotL;

in vec4 worldSpaceVert;
in vec4 eyeSpaceVert;

out vec4 fragColor;

::INSERT_UNIFORMS_HERE::

::INSERT_BRDF_FUNCTION_HERE::


float lightDistanceFromCenter = 2.2;



vec3 computeWithDirectionalLight( vec3 surfPt, vec3 incidentVector, vec3 viewVec, vec3 normal, vec3 tangent, vec3 bitangent )
{
    // evaluate the BRDF
    vec3 b = max( BRDF( incidentVector, viewVec, normal, tangent, bitangent ), vec3(0.0) );

    // multiply in the cosine factor
    if (useNDotL != 0)
        b *= dot( normal, incidentVector );

    return b;
}


vec3 computeWithPointLight( vec3 surfPt, vec3 incidentVector, vec3 viewVec, vec3 normal, vec3 tangent, vec3 bitangent )
{
    // compute the point light vector
    vec3 toLight = (incidentVector * lightDistanceFromCenter) - surfPt;
    float pointToLightDist = length( toLight );
    toLight /= pointToLightDist;


    // evaluate the BRDF
    vec3 b = max( BRDF( toLight, viewVec, normal, tangent, bitangent ), vec3(0.0) );

    // multiply in the cosine factor
    if (useNDotL != 0)
        b *= dot( normal, toLight );

    // multiply in the falloff
    b *= (1.0 / (pointToLightDist*pointToLightDist));

    return b;
}



vec3 computeWithAreaLight( vec3 surfPt, vec3 incidentVector, vec3 viewVec, vec3 normal, vec3 tangent, vec3 bitangent )
{
    float lightRadius = 0.5;

    vec3 lightPoint = (incidentVector * lightDistanceFromCenter);

    // define the surface of the light source (we'll have it always face the sphere)
    vec3 toLight = normalize( (incidentVector * lightDistanceFromCenter) - surfPt );
    vec3 uVec = cross( toLight, vec3(1,0,0) );
    vec3 vVec = cross( uVec, toLight );

    vec3 result = vec3(0.0);

    float u = -1.0;
    for( int i = 0; i < 5; i++ )
    {
        float v = -1.0;
        for( int j = 0; j < 5; j++ )
        {
            vec3 vplPoint = lightPoint + (uVec*u + vVec*v)*lightRadius;

            // compute the point light vector
            vec3 toLight = vplPoint - surfPt;
            float pointToLightDist = length( toLight );
            toLight /= pointToLightDist;


            // evaluate the BRDF
            vec3 b = max( BRDF( toLight, viewVec, normal, tangent, bitangent ), vec3(0.0) );

            // multiply in the cosine factor
            if (useNDotL != 0)
                b *= dot( normal, toLight );

            // multiply in the falloff
            b *= (1.0 / (pointToLightDist*pointToLightDist));



            result += b;

            v += 0.4;
        }
        u += 0.4;
    }

    result /= 25.0;


    return result;
}


void main(void)
{
    // orthogonal vectors
    vec3 normal = normalize( worldSpaceVert.xyz );
    vec3 tangent = normalize( cross( vec3(0,1,0), normal ) );
    vec3 bitangent = normalize( cross( normal, tangent ) );


    // calculate the viewing vector
    //vec3 viewVec = -normalize(eyeSpaceVert.xyz);
    vec3 surfacePos = normalize( worldSpaceVert.xyz );
    vec3 viewVec = vec3(0,0,1); // ortho mode


    vec3 b = computeWithDirectionalLight( surfacePos, incidentVector, viewVec, normal, tangent, bitangent );
    //vec3 b = computeWithPointLight( surfacePos, incidentVector, viewVec, normal, tangent, bitangent );
    //vec3 b = computeWithAreaLight( surfacePos, incidentVector, viewVec, normal, tangent, bitangent );

    // brightness
    b *= brightness;

    // exposure
    b *= pow( 2.0, exposure );

    // gamma
    b = pow( b, vec3( 1.0 / gamma ) );

    fragColor = vec4( clamp( b, vec3(0.0), vec3(1.0) ), 1.0 );
}

