#include "LightSctrPostProcess.h"

#include "Ogre.h"

using namespace Ogre;

CLightSctrPostProcess::CLightSctrPostProcess(SceneManager* scnMgr) :
	m_fTurbidity(1.02f),
	mSceneMgr(scnMgr)
{
	ComputeScatteringCoefficients();
}

CLightSctrPostProcess :: ~CLightSctrPostProcess()
{
}

void CLightSctrPostProcess::OnCreateDevice()
{
	//mRtCam = new Camera("rtCam", mSceneMgr);
	mRtCam = mSceneMgr->createCamera("RtCam");
	mRtCam->setProjectionType(ProjectionType::PT_ORTHOGRAPHIC);
	mRtCam->setNearClipDistance(1);
	mRtCam->setFarClipDistance(3000);
	mRtCam->setPosition(0, 0, 0);
	mRtCam->setDirection(0, 0, -1);
	mRtCam->setOrthoWindow(2, 2);

	Plane quadPlane;
	quadPlane.normal = Vector3::UNIT_Z;
	quadPlane.d = 0;
	Ogre::MeshManager::getSingleton().createPlane("QuadPlane",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, quadPlane, 2, 2);
	mQuadEntity = mSceneMgr->createEntity("QuadEntity", "QuadPlane");
	mQuadEntity->setCastShadows(false);

	mQuadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mQuadNode->attachObject(mQuadEntity);
	mQuadNode->setPosition(0, 0, -2);

	CreatePrecomputedOpticalDepthTexture();

	// disable till the next render quad
	mQuadNode->setVisible(false);
}

void CLightSctrPostProcess::OnDestroyDevice()
{
	delete mRtCam;
	mRtCam = 0;
}

void CLightSctrPostProcess::RenderQuad(const char* matName, RenderTexture* rt)
{
	mQuadEntity->setMaterialName(matName);

	Viewport* viewport = rt->addViewport(mRtCam);
	viewport->setClearEveryFrame(false);
	viewport->setOverlaysEnabled(false);
	// viewport auto update must be turned on for RenderTarget::update()
	rt->update();
	rt->setAutoUpdated(false);
#if 0
	// save to a file for debug
	size_t imgSize = 4 * 4 * PixelUtil::getNumElemBytes(PF_FLOAT32_GR);
	uchar *data = new uchar[imgSize];
	PixelBox pb(4, 4, 1, PF_FLOAT32_GR, data);
	rt->copyContentsToMemory(pb, RenderTarget::FB_AUTO);
	FILE *fp = fopen("NetDensityToAtmTop.txt", "w");
	if (fp)
	{
		//fwrite(data, imgSize, 1, fp);
		float *p = (float *)data;
		for (size_t i = 0; i < imgSize / 8; i++, p += 2)
			fprintf(fp, "[%d,%d] %f %f\n", i / 4, i % 4, p[0], p[1]);
		fclose(fp);
	}
	delete[] data;
#endif
}

bool CLightSctrPostProcess::CreatePrecomputedOpticalDepthTexture()
{
	TexturePtr rtTex = TextureManager::getSingleton().createManual("NetDensityToAtmTopTex",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
		sm_iNumPrecomputedHeights, sm_iNumPrecomputedAngles,
		//4, 4,
		0, PF_FLOAT32_GR, TU_RENDERTARGET);

	RenderQuad("Material/NetDensityToAtmTop", rtTex->getBuffer()->getRenderTarget());

	return true;
}

void CLightSctrPostProcess::ComputeScatteringCoefficients()
{
	// For details, see "A practical Analytic Model for Daylight" by Preetham & Hoffman, p.23

	// Wave lengths
	// [BN08] follows [REK04] and gives the following values for Rayleigh scattering coefficients:
	// RayleighBetha(lambda = (680nm, 550nm, 440nm) ) = (5.8, 13.5, 33.1)e-6
	static const double dWaveLengths[] =
	{
		680e-9,     // red
		550e-9,     // green
		440e-9      // blue
	};

	// Calculate angular and total scattering coefficients for Rayleigh scattering:
	{
		Vector4 &f4AngularRayleighSctrCoeff = m_MediaParams.f4AngularRayleighSctrCoeff;
		Vector4 &f4TotalRayleighSctrCoeff = m_MediaParams.f4TotalRayleighSctrCoeff;
		Vector4 &f4RayleighExtinctionCoeff = m_MediaParams.f4RayleighExtinctionCoeff;

		double n = 1.0003;    // - Refractive index of air in the visible spectrum
		double N = 2.545e+25; // - Number of molecules per unit volume
		double Pn = 0.035;    // - Depolarization factor for air which exoresses corrections 
		//   due to anisotropy of air molecules

		double dRayleighConst = 8.0 * Math::PI * Math::PI * Math::PI * (n*n - 1.0) * (n*n - 1.0) / (3.0 * N) * (6.0 + 3.0*Pn) / (6.0 - 7.0*Pn);
		for (int WaveNum = 0; WaveNum < 3; WaveNum++)
		{
			double dSctrCoeff;
			if (m_PostProcessingAttribs.m_bUseCustomSctrCoeffs)
				dSctrCoeff = f4TotalRayleighSctrCoeff[WaveNum] = m_PostProcessingAttribs.m_f4CustomRlghBeta[WaveNum];
			else
			{
				double Lambda2 = dWaveLengths[WaveNum] * dWaveLengths[WaveNum];
				double Lambda4 = Lambda2 * Lambda2;
				dSctrCoeff = dRayleighConst / Lambda4;
				// Total Rayleigh scattering coefficient is the integral of angular scattering coefficient in all directions
				f4TotalRayleighSctrCoeff[WaveNum] = static_cast<float>(dSctrCoeff);
			}
			// Angular scattering coefficient is essentially volumetric scattering coefficient multiplied by the
			// normalized phase function
			// p(Theta) = 3/(16*Pi) * (1 + cos^2(Theta))
			// f4AngularRayleighSctrCoeff contains all the terms exepting 1 + cos^2(Theta):
			f4AngularRayleighSctrCoeff[WaveNum] = static_cast<float>(3.0 / (16.0 * Math::PI) * dSctrCoeff);
			// f4AngularRayleighSctrCoeff[WaveNum] = f4TotalRayleighSctrCoeff[WaveNum] * p(Theta)
		}
		// Air molecules do not absorb light, so extinction coefficient is only caused by out-scattering
		f4RayleighExtinctionCoeff = f4TotalRayleighSctrCoeff;
	}

	// Calculate angular and total scattering coefficients for Mie scattering:
	{
		Vector4 &f4AngularMieSctrCoeff = m_MediaParams.f4AngularMieSctrCoeff;
		Vector4 &f4TotalMieSctrCoeff = m_MediaParams.f4TotalMieSctrCoeff;
		Vector4 &f4MieExtinctionCoeff = m_MediaParams.f4MieExtinctionCoeff;

		if (m_PostProcessingAttribs.m_bUseCustomSctrCoeffs)
		{
			f4TotalMieSctrCoeff = m_PostProcessingAttribs.m_f4CustomMieBeta * m_PostProcessingAttribs.m_fAerosolDensityScale;
		}
		else
		{
			const bool bUsePreethamMethod = false;
			if (bUsePreethamMethod)
			{
				// Values for K came from the table 2 in the "A practical Analytic Model 
				// for Daylight" by Preetham & Hoffman, p.28
				double K[] =
				{
					0.68455,                //  K[650nm]
					0.678781,               //  K[570nm]
					(0.668532 + 0.669765) / 2.0 // (K[470nm]+K[480nm])/2
				};

				assert(m_MediaParams.fTurbidity >= 1.f);

				// Beta is an Angstrom's turbidity coefficient and is approximated by:
				//float beta = 0.04608365822050f * m_fTurbidity - 0.04586025928522f; ???????

				double c = (0.6544*m_MediaParams.fTurbidity - 0.6510)*1E-16; // concentration factor
				const double v = 4; // Junge's exponent

				double dTotalMieBetaTerm = 0.434 * c * Math::PI * pow(2.0 * Math::PI, v - 2);

				for (int WaveNum = 0; WaveNum < 3; WaveNum++)
				{
					double Lambdav_minus_2 = pow(dWaveLengths[WaveNum], v - 2);
					double dTotalMieSctrCoeff = dTotalMieBetaTerm * K[WaveNum] / Lambdav_minus_2;
					f4TotalMieSctrCoeff[WaveNum] = static_cast<float>(dTotalMieSctrCoeff);
				}

				//AtmScatteringAttribs.f4AngularMieSctrCoeff *= 0.02f;
				//AtmScatteringAttribs.f4TotalMieSctrCoeff *= 0.02f;
			}
			else
			{
				// [BN08] uses the following value (independent of wavelength) for Mie scattering coefficient: 2e-5
				// For g=0.76 and MieBetha=2e-5 [BN08] was able to reproduce the same luminance as given by the 
				// reference CIE sky light model 
				const float fMieBethaBN08 = 2e-5f * m_PostProcessingAttribs.m_fAerosolDensityScale;
				m_MediaParams.f4TotalMieSctrCoeff = Vector4(fMieBethaBN08, fMieBethaBN08, fMieBethaBN08, 0);
			}
		}

		for (int WaveNum = 0; WaveNum < 3; WaveNum++)
		{
			// Normalized to unity Cornette-Shanks phase function has the following form:
			// F(theta) = 1/(4*PI) * 3*(1-g^2) / (2*(2+g^2)) * (1+cos^2(theta)) / (1 + g^2 - 2g*cos(theta))^(3/2)
			// The angular scattering coefficient is the volumetric scattering coefficient multiplied by the phase 
			// function. 1/(4*PI) is baked into the f4AngularMieSctrCoeff, the other terms are baked into f4CS_g
			f4AngularMieSctrCoeff[WaveNum] = f4TotalMieSctrCoeff[WaveNum] / static_cast<float>(4.0 * Math::PI);
			// [BN08] also uses slight absorption factor which is 10% of scattering
			f4MieExtinctionCoeff[WaveNum] = f4TotalMieSctrCoeff[WaveNum] * (1.f + m_PostProcessingAttribs.m_fAerosolAbsorbtionScale);
		}
	}

	{
		// For g=0.76 and MieBetha=2e-5 [BN08] was able to reproduce the same luminance as is given by the 
		// reference CIE sky light model 
		// Cornette phase function (see Nishita et al. 93):
		// F(theta) = 1/(4*PI) * 3*(1-g^2) / (2*(2+g^2)) * (1+cos^2(theta)) / (1 + g^2 - 2g*cos(theta))^(3/2)
		// 1/(4*PI) is baked into the f4AngularMieSctrCoeff
		Vector4 &f4CS_g = m_MediaParams.f4CS_g;
		float f_g = m_MediaParams.m_fAerosolPhaseFuncG;
		f4CS_g.x = 3 * (1.f - f_g*f_g) / (2 * (2.f + f_g*f_g));
		f4CS_g.y = 1.f + f_g*f_g;
		f4CS_g.z = -2.f*f_g;
		f4CS_g.w = 1.f;
	}

	m_MediaParams.f4TotalExtinctionCoeff = m_MediaParams.f4RayleighExtinctionCoeff + m_MediaParams.f4MieExtinctionCoeff;

	/*
	if (pDeviceCtx && m_pcbMediaAttribs)
	{
		pDeviceCtx->UpdateSubresource(m_pcbMediaAttribs, 0, NULL, &m_MediaParams, 0, 0);
	}
	*/
}