#pragma once

namespace Ogre
{
	class SceneManager;
}
class CLightSctrPostProcess;

class COutdoorLightScatteringSample
{
public:
	COutdoorLightScatteringSample(Ogre::SceneManager* scnMgr);
	virtual ~COutdoorLightScatteringSample();

	virtual void Create();
private:
	void Destroy();

	class CLightSctrPostProcess *m_pLightSctrPP;
};