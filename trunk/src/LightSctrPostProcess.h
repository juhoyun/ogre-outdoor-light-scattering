#pragma once

#include "structures.h"

namespace Ogre
{
	class SceneManager;
	class Camera;
	class SceneNode;
	class RenderTexture;
	class Entity;
};

class CLightSctrPostProcess
{
public:
	CLightSctrPostProcess(SceneManager* scnMgr);
	~CLightSctrPostProcess();

	void OnCreateDevice();
	void OnDestroyDevice();
private:
	bool CreatePrecomputedOpticalDepthTexture();
	SPostProcessingAttribs m_PostProcessingAttribs;

	static const int sm_iNumPrecomputedHeights = 1024;
	static const int sm_iNumPrecomputedAngles = 1024;

	void ComputeScatteringCoefficients();

	const float m_fTurbidity;
	SAirScatteringAttribs m_MediaParams;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mRtCam;
	Ogre::SceneNode* mQuadNode;
	Ogre::Entity* mQuadEntity;

	void RenderQuad(const char* matName, RenderTexture* rt);
};