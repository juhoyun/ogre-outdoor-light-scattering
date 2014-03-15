#pragma once

class CLightSctrPostProcess;

class COutdoorLightScatteringSample
{
public:
	COutdoorLightScatteringSample();
	virtual ~COutdoorLightScatteringSample();

	virtual void Create();
private:
	void Destroy();

	class CLightSctrPostProcess *m_pLightSctrPP;
};