#include "OutdoorLightScattering.h"
#include "LightSctrPostProcess.h"
#include "prerequisites.h"

#include <string>

COutdoorLightScatteringSample::COutdoorLightScatteringSample(Ogre::SceneManager* scnMgr)
{
	m_pLightSctrPP = new CLightSctrPostProcess(scnMgr);
}

COutdoorLightScatteringSample::~COutdoorLightScatteringSample()
{
	Destroy();

	SAFE_DELETE(m_pLightSctrPP);
}

void COutdoorLightScatteringSample::Destroy()
{
	m_pLightSctrPP->OnDestroyDevice();
}

void COutdoorLightScatteringSample::Create()
{
#ifdef NOT_READY_YET
	//std::string ConfigPath = "Default_Config.txt";
	ParseConfigurationFile( ConfigPath );

	// Create shadow map before other assets!!!
	HRESULT hResult = CreateShadowMap(mpD3dDevice);
#endif /* NOT_READY_YET */

	m_pLightSctrPP->OnCreateDevice();
}