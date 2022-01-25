# BRDF Explorer
The Disney BRDF Explorer is an application that allows the development and analysis
of bidirectional reflectance distribution functions (BRDFs). It can load and plot 
analytic BRDF functions (coded as snippets of OpenGL GLSL programs), measured
material data from the MERL database, and anisotropic measured material data from
MIT CSAIL.

Most of the application should be (hopefully) self explanatory, so the goal here is to
document the less obvious bits of usage, as well as providing some useful info.



### Which BRDF(s) you are seeing
-----------------------------------
In the BRDF Parameters window, each BRDF has a colored background. The plotted values
(for the 3D, polar, and cartesian plots) are drawn in the corresponding colors.

In the Image Slice, Lit Object, and Lit Sphere windows, you're seeing the first
(topmost in the parameters window) enabled BRDF.


### Navigating polar/cartesian Plots
------------------------------------
Use the left mouse button to pan around in the plot. Right dragging will zoom in/out
around the centered location. Double-clicking anywhere resets the view to the default
zoom and location. For cartesian plots, Control+left drag stretches ONLY the x-axis,
while Control+right drag stretches only the y-axis.


### Navigating 3D Plots
------------------------------------
Left-drag spins around the origin (there's no current way to change this). Right drag
zooms in and out. Double clicking resets to the default view.


### Parameter Sliders
------------------------------------
Parameters sliders (a slider plus a text entry box) are all over the application. To
reset one to its default value, Control+Click in the text box.


### Soloing BRDFs
------------------------------------
Pressing the "solo" (circle) button for a given BRDF makes ONLY that BRDF visible,
hiding all the others. The "Solo this BRDF's color channels" button also hides all
the other channels, but shows a separate plot for the red, green, and blue channels
(in their respective colors). Click the solo button again to exit solo mode.


### Lit Sphere View
------------------------------------
You can drag the left mouse button on the surface of the sphere to change the
incident angle. If "Double Theta" is enabled, the specular highlight will track
the position of the mouse.


### Lit Object View
------------------------------------
The lit object view allows an arbitrary object to be viewed under with a directional
light from the incident direction (in "No IBL" mode) or under illumination from an
arbitrary environment map. Left dragging rotates the object; right dragging zooms
the object; Control+left dragging rotates the environment probe.

The combo box lets you choose "No IBL", "IBL: No IS" (quasirandom sampling from
the environment map with no importance sampling) and "IBL: IBL IS" (importance
sampling from the IBL). Multiple importance sampling is planned but not implemented.
The "Keep Sampling" mode, when enabled, progressively refines the image until
4096 samples have been applied.

The buttons allow changing out the object (any OBJ should work) and the environment
probe (which must be in ptex format).


### Albedo View
------------------------------------
Albedo computation by brute force sampling proved too expensive to do interactively,
so the application can use several different sampling strategies to compute the albedo.
Use the combo box on the right to choose between these sampling strategies. The
Resample x10 button adds more samples to that view of the graph.


### Image Slice View
------------------------------------
An "image slice" is an alternative way of looking at BRDF data that we have found
helpful at Disney. Along the x-axis, the half angle is varied from zero to 90
degrees; along the y-axis, the difference angle is varied from 0 to 90 degrees.


### BRDF files
------------------------------------
.brdf files consist of a set of parameters and a BRDF function written in GLSL.
At runtime, the application creates UI elements for each parameter, and creates
shaders for different views that incorporate the GLSL function.

The BRDF function looks like this (this example is for a lambertian BRDF):
```glsl
::begin shader
vec3 BRDF( vec3 L, vec3 V, vec3 N, vec3 X, vec3 Y )
{
    return vec3(reflectance / 3.14159265);
}
::end shader
```
Anything valid in GLSL can go between "::begin shader" and "::end shader" as
long as it's valid GLSL.

The application allows float, color, and boolean parameters. The float parameters
have the form
```glsl
float [name] [min val] [max val] [default val]
```
for example:
```glsl
float reflectance 0.0 1.0 1.0
```

Boolean parameters have the form
```glsl
bool [name] [default], where default is 0 or 1 (keywords such as true and
false aren't recognized)
```
for example:
```glsl
bool hasDiffuse 0
```

Color parameters have the form
```glsl
color [name] [defaultR] [defaultG] [defaultB], where defaultR/G/B are in [0..1]
```
for example:
```glsl
color diffuseColor 0.5 0.1 1.0
```

Parameters are passed into the resulting GLSL shaders as uniforms of the same
name (so parameter names must be valid GLSL variable names, although the
application doesn't enforce this). Float parameters come in as uniform floats,
color parameters as vec3s, and boolean parameters as bools. The application
declares them when constructing shaders, so your GLSL BRDF functions can 
refer to them knowing that they'll exist and have the proper values at runtime.


### Light Probe Attribution
------------------------------------
HDR Light Probe Images Copyright 1998 courtesy of Paul Debevec,
www.debevec.org, used with permission.

Please see:
Paul Debevec.  Rendering Synthetic Objects Into Real Scenes: Bridging Traditional and 
Image-Based Graphics With Global Illumination and High Dynamic Range Photography.  
Proceedings of SIGGRAPH 98, Computer Graphics Proceedings, Annual Conference Series, 
July 1998, pp. 189-198.


### Where to get measured BRDF data
------------------------------------
MERL data can be requested here:
http://www.merl.com/brdf/

MIT CSAIL data is here:
http://people.csail.mit.edu/addy/research/brdf/


