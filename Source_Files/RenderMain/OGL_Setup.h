#ifndef _OGL_SETUP_
#define _OGL_SETUP_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
	OpenGL Interface File,
	by Loren Petrich,
	March 12, 2000

	This contains functions intended for finding out OpenGL's presence
	in the host system, for setting parameters for OpenGL rendering,
	and for deciding whether to use OpenGL for rendering.
	
	May 27, 2000 (Loren Petrich)
	
	Added support for flat static effect

	Added XML support
	
	May 31, 2000 (Loren Petrich)
	
	Added support for texture resetting. This clears all the textures in memory
	and forces them to be reloaded. This may be good in cases of textures dropping out.
	
	June 11, 2000 (Loren Petrich)
	
	Added support for see-through liquids as an OpenGL parameter.
	Also added an opacity-value shift for making dark areas more visible

Sep 9, 2000:

	Added flag for AppleGL texturing fix

Dec 3. 2000 (Loren Petrich):
	Changed 16-bit internal representation of textures from 5551 to 4444

Dec 17, 2000 (Loren Petrich):
	Eliminated fog parameters from the preferences;
	there is still a "fog present" switch, which is used to indicate
	whether fog will not be suppressed.

Apr 27, 2001 (Loren Petrich):
	Modified the OpenGL fog support so as to enable below-liquid fogs

Aug 21, 2001 (Loren Petrich):
	Adding support for 3D-model inhabitant objects
*/


#include "XML_ElementParser.h"
#include "OGL_Subst_Texture_Def.h"
#include "OGL_Model_Def.h"

#include <string>


// Initializer; returns whether or not OpenGL is present
bool OGL_Initialize();

// Test for presence of OpenGL
bool OGL_IsPresent();

// Test for whether OpenGL is currently active
bool OGL_IsActive();

// Test whether an extension exists
bool OGL_CheckExtension(const std::string);

// Here are some OpenGL configuration options and how to access them
// (they are in the preferences data)

// There are separate texturing options for each kind of texture,
// as listed below; this is so that one can degrade texture quality independently,
// and have (say) high-quality walls and weapons in hand, medium-quality inhabitant sprites,
// and low-quality landscapes.
enum
{
	OGL_Txtr_Wall,
	OGL_Txtr_Landscape,
	OGL_Txtr_Inhabitant,
	OGL_Txtr_WeaponsInHand,
	OGL_NUMBER_OF_TEXTURE_TYPES
};

/*
	All enumeration starts from 0, in contrast to MacOS popup menus, for example,
	which start from one.
	
	The filters are OpenGL filter types
	GL_NEAREST		(Pixelated)
	GL_LINEAR		(Smoothed)
	GL_NEAREST_MIPMAP_NEAREST
	GL_LINEAR_MIPMAP_NEAREST
	GL_NEAREST_MIPMAP_LINEAR
	GL_LINEAR_MIPMAP_LINEAR
	
	Nearby textures have only the first two;
	distant textures have the additional four, which feature mipmapping.
	
	The resolutions are how much to shrink the textures before using them,
	in order to save VRAM space; they are, in order
	x1
	x1/2
	x1/4
	
	The color depths indicate what numbers of bits for each color channel:
	
	32-bit (8888)
	16-bit (4444)
	8-bit  (2222)
*/

struct OGL_Texture_Configure
{
	short NearFilter;
	short FarFilter;
	short Resolution;
	short ColorFormat;
};

// Here are some control flags
enum
{
	OGL_Flag_ZBuffer	= 0x0001,	// Whether to use a Z-buffer
	OGL_Flag_VoidColor	= 0x0002,	// Whether to color the void
	OGL_Flag_FlatLand	= 0x0004,	// Whether to use flat-textured landscapes
	OGL_Flag_Fog		= 0x0008,	// Whether to make fog
	OGL_Flag_3D_Models	= 0x0010,	// Whether to use 3D models
	OGL_Flag_2DGraphics	= 0x0020,	// Whether to pipe 2D graphics through OpenGL
	OGL_Flag_FlatStatic	= 0x0040,	// Whether to make the "static" effect look flat
	OGL_Flag_Fader		= 0x0080,	// Whether to do the fader effects in OpenGL
	OGL_Flag_LiqSeeThru	= 0x0100,	// Whether the liquids can be seen through
	OGL_Flag_Map		= 0x0200,	// Whether to do the overhead map with OpenGL
	OGL_Flag_TextureFix	= 0x0400,	// Whether to apply a texture fix for old Apple OpenGL
	OGL_Flag_HUD		= 0x0800,	// Whether to do the HUD with OpenGL
};

struct OGL_ConfigureData
{
	// Configure textures
	OGL_Texture_Configure TxtrConfigList[OGL_NUMBER_OF_TEXTURE_TYPES];

	// Overall rendering flags
	uint16 Flags;
	
	// Color of the Void
	RGBColor VoidColor;
	
	// Landscape Flat Colors
	// First index: which landscape
	// (day, night, moon, outer space)
	// Second index: ground, sky
	RGBColor LscpColors[4][2];
	
	// Anisotropy setting
	float AnisotropyLevel;
  int16 Multisamples;
};

OGL_ConfigureData& Get_OGL_ConfigureData();

// The OpenGL-configuration dialog box; returns whether its changes had been selected
bool OGL_ConfigureDialog(OGL_ConfigureData& Data);

// Set defaults
void OGL_SetDefaults(OGL_ConfigureData& Data);


// for managing the model and image loading and unloading;
void OGL_LoadModelsImages(short Collection);
void OGL_UnloadModelsImages(short Collection);


// Does this for a set of several pixel values or color-table values;
// the pixels are assumed to be in OpenGL-friendly byte-by-byte RGBA format.
void SetPixelOpacities(OGL_TextureOptions& Options, int NumPixels, uint32 *Pixels);



// Reset the textures (walls, sprites, and model skins) (good if they start to crap out)
// Implemented in OGL_Textures.cpp
void OGL_ResetTextures();

#ifdef MOVED_OUT

#ifdef HAVE_OPENGL

// 3D-Model and Skin Support

// Model-skin options
struct OGL_SkinData: public OGL_TextureOptionsBase
{
	short CLUT;				// Which color table is this skin for? (-1 is all)
	
	OGL_SkinData(): CLUT(ALL_CLUTS) {}
};

// Manages skins, in case we decide to have separate static and animated models
struct OGL_SkinManager
{
	// List of skins that a model will "own"
	vector<OGL_SkinData> SkinData;
	
	// OpenGL skin ID's (one for each possible
	// Copied from TextureState in OGL_Textures	
	// Which member textures?
	enum
	{
		Normal,		// Used for all normally-shaded and shadeless textures
		Glowing,	// Used for self-luminous textures
		NUMBER_OF_TEXTURES
	};
	GLuint IDs[NUMBER_OF_OPENGL_BITMAP_SETS][NUMBER_OF_TEXTURES];		// Texture ID's
	bool IDsInUse[NUMBER_OF_OPENGL_BITMAP_SETS][NUMBER_OF_TEXTURES];	// Which ID's are being used?
		
	void Reset(bool Clear_OGL_Txtrs);		// Resets the skins so that they may be reloaded;
											// indicate whether to clear OpenGL textures
	
	OGL_SkinData *GetSkin(short CLUT);		// Gets a pointer to a skin-data object; NULL for no skin available
	bool Use(short CLUT, short Which);		// Uses a skin; returns whether to load one	
	
	// For convenience
	void Load();
	void Unload();
};


// Mode
enum
{
	OGL_MLight_Fast,			// Fast method -- only one miner's-light calculation
	OGL_MLight_Fast_NoFade,		// Like above, but miner's light doesn't fade toward sides
	OGL_MLight_Indiv,			// Miner's light calculated for each vertex
	OGL_MLight_Indiv_NoFade,	// Like above, but miner's light doesn't fade toward sides
	NUMBER_OF_MODEL_LIGHT_TYPES
};


// Static 3D-Model Data and Options
class OGL_ModelData: public OGL_SkinManager
{
public:
	// Name of the model file;
	// there are two extra names here for handling ggadwa's Dim3 multiple files
	vector<char> ModelFile, ModelFile1, ModelFile2;
	
	// Type of model-file data (guess the model-file type if empty)
	vector<char> ModelType;
	
	// Preprocessing: rotation scaling, shifting
	// Scaling and rotation are applied before shifting
	// Scaling can be negative, thus producing mirroring
	float Scale;					// From model units to engine internal units (not World Units)
	float XRot, YRot, ZRot;			// In degrees
	float XShift, YShift, ZShift;	// In internal units
	short Sidedness;				// Which side of the polygons is visible?
									// (+: clockwise, -: counterclockwise, 0: both)
	short NormalType;				// What type of normals?
	float NormalSplit;				// Threshold for splitting the vertex normals 
	short LightType;				// What type of lighting?
	short DepthType;				// What sort of depth reference to use?
									// (+: farthest point, -: nearest point, 0: center point)
	
	// Should a rotation rate be included, in order to get that Quake look?
	
	// The model itself (static, single-skin [only one skin at a time])
	Model3D Model;
	bool ModelPresent() {return !Model.VertIndices.empty();}
	
	// For convenience
	void Load();
	void Unload();
	
	OGL_ModelData():
		Scale(1), XRot(0), YRot(0), ZRot(0), XShift(0), YShift(0), ZShift(0), Sidedness(1),
			NormalType(1), NormalSplit(0.5), LightType(0), DepthType(0) {}
};


// Returns NULL if a collectiona and sequence do not have an associated model;
// also returns which model sequence was found (
OGL_ModelData *OGL_GetModelData(short Collection, short Sequence, short& ModelSequence);

// Resets all model skins; arg is whether to clear OpenGL textures
void OGL_ResetModelSkins(bool Clear_OGL_Txtrs);

#endif // def HAVE_OPENGL

#endif

#ifdef HAVE_OPENGL

// Resets all model skins; arg is whether to clear OpenGL textures
void OGL_ResetModelSkins(bool Clear_OGL_Txtrs);

#endif // def HAVE_OPENGL


// Fog data record
struct OGL_FogData
{
	rgb_color Color;
	float Depth;		// In World Units (1024 internal units)
	bool IsPresent;
	bool AffectsLandscapes;
};

// Fog types
enum
{
	OGL_Fog_AboveLiquid,
	OGL_Fog_BelowLiquid,
	OGL_NUMBER_OF_FOG_TYPES
};

OGL_FogData *OGL_GetFogData(int Type);


// XML support:
XML_ElementParser *OpenGL_GetParser();

#endif
