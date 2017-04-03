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
uniform float phiD;
uniform int nSamples;
uniform float sampleMultOn;
uniform vec3 colorMask;
uniform int samplingMode;

const float PI_ = 3.14159265358979323846264;
const vec3 RGB2L = vec3(0.3, 0.59, 0.11);

in vec3 vtx_position;

out vec4 v_albedoData;

::INSERT_UNIFORMS_HERE::


::INSERT_BRDF_FUNCTION_HERE::

float hammersleySample(uint bits)
{
    bits = ( bits << 16u) | ( bits >> 16u);
    bits = ((bits & 0x00ff00ffu) << 8u) | ((bits & 0xff00ff00u) >> 8u);
    bits = ((bits & 0x0f0f0f0fu) << 4u) | ((bits & 0xf0f0f0f0u) >> 4u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xccccccccu) >> 2u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xaaaaaaaau) >> 1u);
    return float(bits) * 2.3283064365386963e-10; // divide by 1<<32
}

float modifyLog( float x ) {
        // log base 10
        return log(x + 1.0) * 0.434294482;
}

float rand(vec2 n) {
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

//Sampling functions return costheta/pdf
//As passed in, view=vec3(cos(phi), sin(phi), 1.0)
float CosinePDF(in vec3 incident, in vec3 view) {
  if (view.z < 0.0) return 0.0;
  return view.z/PI_;
}

float CosineSample(in float x1, in vec3 incident, inout vec3 view) {
  float r = sqrt(x1);
  float costheta_v = sqrt(1-r*r);
  view = normalize( vec3( r*view.x, r*view.y, costheta_v));
  return PI_;
}

float UniformPDF(in vec3 incident, in vec3 view) {
  return 0.5/PI_;
}

float UniformSample(in float x1, in vec3 incident, inout vec3 view) {
  float costheta_v = x1;
  float sintheta_v = sqrt(1-costheta_v*costheta_v);
  view = normalize( vec3(costheta_v*view.x, costheta_v*view.y, sintheta_v) );
  return 2.0*PI_*costheta_v;
}

float PolarPDF(in vec3 incident, in vec3 view) {
  float costheta_v = view.z;
  float sintheta_v = sqrt(1.0-costheta_v*costheta_v);
  return 1.0/(PI_*PI_*sintheta_v);
}

float PolarSample(in float x1, in vec3 incident, inout vec3 view) {
  float costheta_v = cos(x1*0.5*PI_);
  float sintheta_v = sin(x1*0.5*PI_);
  view = normalize( vec3(sintheta_v*view.x, sintheta_v*view.y, costheta_v) );
  return PI_*PI_*costheta_v*sintheta_v;
}


float BlinnPDF(in float exponent, in vec3 incident, in vec3 view) {
  vec3 H = normalize(incident+view);
  float costhetah = H.z;
  float costhetad = dot(view, H);
  if (costhetad < 0) return 0.0;
  return (exponent+1.0)*pow(costhetah,exponent)/(PI_*8.0*costhetad);
}


float BlinnSample(in float exponent, in float x1, in vec3 incident, inout vec3 view) {
  float costhetah = pow(x1, 1.0/(exponent+1.0));
  float sinthetah = sqrt(max(0.0,1.0-costhetah*costhetah));
  vec3 halfvector = vec3(sinthetah*view.x, sinthetah*view.y, costhetah);
  if (halfvector.z*incident.z < 0) halfvector = -1.0*halfvector;
  view = -incident + 2.0*dot(incident, halfvector)*halfvector;
  float pdfinv = PI_*8.0*dot(incident, halfvector)/((exponent+1.0)*pow(costhetah,exponent));
  if (view.z < 0.0) return 0.0;
  return view.z*pdfinv;
}

float MISPowerHeuristic(int n_f, float pdf_f, int n_g, float pdf_g) {
  float f = n_f*pdf_f;
  float g = n_g*pdf_g;
  return f*f/(f*f+g*g);
}


void main(void)
{

	int i, j;

	// orthonormal vectors
	vec3 N = vec3(0,0,1); // normal
	vec3 X = vec3(1,0,0); // tangent
	vec3 Y = vec3(0,1,0); // bitangent

	// theta is encoded in Z; x and y might get stretched
	float yAngle = vtx_position.z;
	vec3 normalizedIncidentVector = normalize( vec3(  sin(yAngle) * cos(incidentPhi),
														 sin(yAngle) * sin(incidentPhi),
															cos(yAngle) ) );
  float bexponent = 2;
  //Estimate exponent, use brdf value at theta~=80 degrees as a 'minimum'
  //binary search (at normal incidence) for theta where brdf is 5% of it's peak
  if (samplingMode == 3 || samplingMode == 4) { //if Blinn-Phong sampling or MIS
    float cosphi = cos(incidentPhi);
    float sinphi = sin(incidentPhi);
    float theta = 0.25*PI_;
    float deltatheta = 0.5*theta;
    float maxbrdfval = dot(RGB2L, BRDF(N, N, N, X, Y));
    float minbrdfval = dot(RGB2L, BRDF(N, vec3(0.985*cosphi, 0.985*sinphi, 0.1697), N, X, Y));
    float target = 0.05*maxbrdfval+minbrdfval;
    float brdfval = dot(RGB2L, BRDF(N, vec3(sin(theta)*cosphi,sin(theta)*sinphi, cos(theta)), N, X, Y));

    while( deltatheta>0.01 && abs(target-brdfval)>0.01*target && theta<0.5*PI_ ) {
      if( brdfval < target) theta-= deltatheta;
      else theta += deltatheta;
      deltatheta = deltatheta*0.5;
      float costhetah = cos(theta);
      float sinthetah = sin(theta);
      brdfval= dot(RGB2L, BRDF(N, normalize(vec3(sinthetah*cosphi, sinthetah*sinphi, costhetah)), N, X, Y ));
    }
    bexponent = max(2.0,min(1000.0,log(brdfval)/log(cos(theta/2.0))));
    if (theta<0.2) bexponent = 1000.0;
    if ((maxbrdfval-minbrdfval)/minbrdfval<1.5) bexponent = 2.0;
    bexponent *= 2; //This might help?
  }

  //Hemisphere sampling
  int ns = nSamples;
  if (sampleMultOn > 0.5) ns *= 10;
  float ns_inv = 1.0/float(ns);
  vec3 radianceFull = vec3(0,0,0);
  vec3 radiance23 = vec3(0,0,0);
  for(i=0; i<ns; i++) {
      float phi_v = 2.0*PI_*float(i)*ns_inv;
      float x1 = hammersleySample(uint(i));
      vec3 viewingVector = vec3(cos(phi_v), sin(phi_v), 1.0);
      float costhetaoverpdf;
      if (samplingMode == 0 || samplingMode == 4) costhetaoverpdf = CosineSample(x1, normalizedIncidentVector, viewingVector);
      else if (samplingMode == 1) costhetaoverpdf = UniformSample(x1, normalizedIncidentVector, viewingVector);
      else if (samplingMode == 2) costhetaoverpdf = PolarSample(x1, normalizedIncidentVector, viewingVector);
      else if (samplingMode == 3) costhetaoverpdf = BlinnSample(bexponent, x1, normalizedIncidentVector, viewingVector);
      vec3 radiance = BRDF(normalizedIncidentVector, viewingVector, N, X, Y )*costhetaoverpdf;
      //Multiple Importance Sampling (2x as many samples)
      if (samplingMode == 4) {
        radiance *= MISPowerHeuristic(ns, CosinePDF(normalizedIncidentVector, viewingVector),
                                      ns, BlinnPDF(bexponent, normalizedIncidentVector, viewingVector));
        //Blinn-Phong Sample
        viewingVector = vec3(cos(phi_v), sin(phi_v), 1.0);
        float costhetaoverpdfG = BlinnSample(bexponent, x1, normalizedIncidentVector, viewingVector);
        vec3 radianceG = BRDF(normalizedIncidentVector, viewingVector, N, X, Y )*costhetaoverpdfG;
        radianceG *= MISPowerHeuristic( ns,
                                          viewingVector.z/costhetaoverpdfG,
                                          ns,
                                          CosinePDF(normalizedIncidentVector, viewingVector) );
        if (costhetaoverpdfG>0) {
          radianceFull += radianceG;
          if(i%3!=0) radiance23 += radianceG;
        }
      }
      if (costhetaoverpdf>0) {
        radianceFull += radiance;
        if(i%3!=0) radiance23 += radiance;
      }
  }
  vec3 bRes = ns_inv*radianceFull;
  vec3 bRes23 = ns_inv*radiance23*(3.0/2.0);

  float bResL = dot(bRes, RGB2L);
  float bRes23L = dot(bRes23, RGB2L);

  //calculate whether or not the sample is likely to be converged
  float error = abs(bResL-bRes23L)/bResL;
  float alpha = 1;
  if (error > 0.05) alpha = 0.05;
  if (error > 0.02) alpha = 0.15;
  int degrees = int(round(180*yAngle/PI_));
  if ( degrees % 15 == 0 ) alpha = 1.0; //ensure some parts of the line are solid for contrast
  v_albedoData = vec4( bRes.xyz, alpha);

	float b = dot( bRes, colorMask );
	float radius = useLogPlot > 0.5 ? modifyLog( b ) : b;
	
	// now displace the vertex by that much
	vec4 inPos = vec4( vtx_position.xy, 0, 1 );
	inPos.y = radius;

	
	// do the necessary transformations
  // send the eye-space vert to the fragment shader, to fake some normals with
	vec4 eyeSpaceVert = modelViewMatrix * inPos;
	gl_Position = projectionMatrix * eyeSpaceVert;

}

