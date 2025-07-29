/**
 * Wii64 - GraphicsGX.h
 * Copyright (C) 2009 sepp256
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#ifndef GRAPHICSRSX_H
#define GRAPHICSRSX_H

#include "GuiTypes.h"
#ifdef __GX__
#include <gccore.h>
#endif //__GX__

#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

#include "combined_shader_vpo.h"
#include "combined_shader_fpo.h"

namespace menu {

class Graphics
{
public:
	Graphics(GXRModeObj *vmode);
	~Graphics();
	void init();
	void drawInit();
	void swapBuffers();
	void clearEFB(GXColor color, u32 zvalue);
	void newModelView();
	void translate(float x, float y, float z);
	void translateApply(float x, float y, float z);
	void rotate(float degrees);
	void loadModelView();
	void loadOrthographic();
	void setDepth(float newDepth);
	float getDepth();
	void setColor(GXColor color);
	void setColor(GXColor* color);
	void drawRect(int x, int y, int width, int height);
	void fillRect(int x, int y, int width, int height);
	void drawImage(int textureId, int x, int y, int width, int height, float s1, float s2, float t1, float t2);
	void drawLine(int x1, int y1, int x2, int y2);
	void drawCircle(int x, int y, int radius, int numSegments);
	void drawString(int x, int y, std::string str);
	void drawPoint(int x, int y, int radius);
	void setLineWidth(int width);
	void pushDepth(float d);
	void popDepth();
	void enableScissor(int x, int y, int width, int height);
	void disableScissor();
	void enableBlending(bool blend);
	void setTEV(int tev_op);
	void pushTransparency(float f);
	void popTransparency();
	void setTransparency(float f);
	float getTransparency();
	void testPrim();

	enum Shaders
	{
		SHADER_PASSTEX=1,
		SHADER_PASSCOLOR,
		SHADER_MODULATE
	};

private:
	void applyCurrentColor();
	float getCurrentTransparency(int index);
//	GXRModeObj *vmode;
//	GXRModeObj vmode_phys;
	int which_fb;
	bool first_frame;
//	void *xfb[2];
	float depth, transparency;
	float viewportWidth, viewportHeight;
	FloatStack depthStack, transparencyStack;
	GXColor currentColor[4], appliedColor[4];
//	Mtx currentModelViewMtx;
//	Mtx44 currentProjectionMtx;
//	MatrixStack modelViewMtxStack;

	u32 fpsize;
	u32 vpsize;
	u32 fp_offset;
	u32 *fp_buffer;

	rsxProgramConst *projMatrix_id;
	rsxProgramConst *modelViewMatrix_id;
	rsxProgramAttrib *vertexPosition_id;
	rsxProgramAttrib *vertexColor0_id;
	rsxProgramAttrib *vertexTexcoord_id;
	rsxProgramAttrib *textureUnit_id;
	rsxProgramConst *mode_id;
	f32 shader_mode;

	void *vp_ucode;
	rsxVertexProgram *vpo;
	void *fp_ucode;
	rsxFragmentProgram *fpo;

	Matrix4 projMatrix, modelViewMatrix;

};

} //namespace menu 

#endif
