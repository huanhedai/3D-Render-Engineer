#pragma once
#include "material.h"
#include "../texture.h"

class ScreenMaterial : public Material {
public:
	ScreenMaterial();
	~ScreenMaterial();

public:
	Texture* mScreenTexture{ nullptr };	// 屏幕纹理			
	Texture* mColorWeightTexture{ nullptr };		// OIT的color×weight纹理					
	Texture* mWeightSumTexture{ nullptr };			// OIT的weight总和纹理
};