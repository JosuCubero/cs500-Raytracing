/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_josu.cubero_final
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 03/18/2020
----------------------------------------------------------------------------------------------------------*/

This project contains the logic for a basic multhreaded ray tracer.

To program can be compiled and run using Visual Studio.
The executable will be generated in the bin folder

Program input parameters:
All of the configuration options are now in the config file for convenience.

Config file:
- Input scene file path
- Output scene file path
- Depth:		Maximum number of recursion depth
- Resolution:		Screen resolution
- AntialiasingSamples:	Total samples for antialiasing
- AdaptiveAntialiasing:	Flag for adaptive sampling
- ShadowSamples:	Total samples for shadows
- DoF:			Flag for depth of field
- DoFSamples:		Total samples for depth of field
- ReflectionSamples:	Total samples for reflection roughness
- Window		Flag for window preview
- Epsilon: 		Epsilon value for the bouncing ray offset

Important files:
- raytracer.h/cpp	-> Refraction, Antialiasing
- shapes.h/cpp		-> Intersection algorithms / Mesh

Problems / Bugs:
- The size of the image (pixels) is limited to the amount of elements an std::vector can hold.
