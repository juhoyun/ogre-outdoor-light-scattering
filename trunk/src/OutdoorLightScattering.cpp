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

#ifdef NOT_READY_YET
	// Create data source
	try
	{
		m_pElevDataSource.reset(new CElevationDataSource(m_strRawDEMDataFile.c_str()));
		m_pElevDataSource->SetOffsets(m_TerrainRenderParams.m_iColOffset, m_TerrainRenderParams.m_iRowOffset);
		m_fMinElevation = m_pElevDataSource->GetGlobalMinElevation() * m_TerrainRenderParams.m_TerrainAttribs.m_fElevationScale;
		m_fMaxElevation = m_pElevDataSource->GetGlobalMaxElevation() * m_TerrainRenderParams.m_TerrainAttribs.m_fElevationScale;
	}
	catch (const std::exception &)
	{
		LOG_ERROR(_T("Failed to create elevation data source"));
		return;
	}

	LPCTSTR strTileTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES], strNormalMapPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	for (int iTile = 0; iTile < _countof(strTileTexPaths); ++iTile)
	{
		strTileTexPaths[iTile] = m_strTileTexPaths[iTile].c_str();
		strNormalMapPaths[iTile] = m_strNormalMapTexPaths[iTile].c_str();
	}

	V(m_EarthHemisphere.OnD3D11CreateDevice(m_pElevDataSource.get(), m_TerrainRenderParams, mpD3dDevice, mpContext, m_strRawDEMDataFile.c_str(), m_strMtrlMaskFile.c_str(), strTileTexPaths, strNormalMapPaths));
#endif
}