#include "ExampleApplication.h"
#include "ExampleFrameListener.h"
#include "OgrePlatform.h"
#include "windows.h"
#include "Ogre.h"

#include "OutdoorLightScattering.h"
#include "LightSctrPostProcess.h"

using namespace Ogre;

class TestFrameListener : public ExampleFrameListener
{
	SceneManager* mSceneMgr;
public:
	TestFrameListener(SceneManager* scnMgr, RenderWindow* win, Camera* cam)
		: ExampleFrameListener(win, cam), mSceneMgr(scnMgr)
	{
	}

	virtual bool frameStarted(const FrameEvent& evt)
	{
		return true;
	}

	virtual bool processUnbufferedKeyInput(const FrameEvent& evt)
	{
		if (!ExampleFrameListener::processUnbufferedKeyInput(evt))
			return false;

		return true;
	}
};

class TestApplication : public ExampleApplication
{
public:
	TestApplication() : ExampleApplication()
	{
	}

protected:

	COutdoorLightScatteringSample* mSample;

	virtual void createFrameListener(void)
	{
		mFrameListener= new TestFrameListener(mSceneMgr, mWindow, mCamera);
		mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
	}

	virtual void createScene(void)
	{
#if 1
		mCamera->setPosition(0, 0, 10);
		mCamera->lookAt(0, 0, -1);
		mCamera->setNearClipDistance(1);
		mCamera->setFarClipDistance(1000);
#else
		mCamera->setProjectionType(ProjectionType::PT_ORTHOGRAPHIC);
		mCamera->setNearClipDistance(1);
		mCamera->setFarClipDistance(3);
		mCamera->setPosition(0, 0, 0);
		mCamera->setDirection(0, 0, -1);
		mCamera->setOrthoWindow(2, 2);
#endif
		mSample = new COutdoorLightScatteringSample(mSceneMgr);
		mSample->Create();
#if 0
		//MaterialPtr quadTestMat = MaterialManager::getSingleton().getByName("QuadTestMaterial");
		//quadTestMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("NetDensityToAtmTopTex");

		TexturePtr rtTex = TextureManager::getSingleton().createManual("testRT",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			TEX_TYPE_2D, 16, 16,
			0, PF_R8G8B8, TU_RENDERTARGET);

		RenderTexture* rt = rtTex->getBuffer()->getRenderTarget();
		Viewport* viewport = rt->addViewport(mCamera);
		rt->update();
		rt->setAutoUpdated(false);
#endif
		Plane quadPlane;
		quadPlane.normal = Vector3::UNIT_Z;
		quadPlane.d = 0;
		Ogre::MeshManager::getSingleton().createPlane("QuadPlane2",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, quadPlane, 2, 2);
		Entity* mQuadEntity = mSceneMgr->createEntity("QuadEntity2", "QuadPlane2");
		mQuadEntity->setMaterialName("QuadTestMaterial");
		SceneNode* mQuadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mQuadNode->attachObject(mQuadEntity);
		mQuadNode->setPosition(2, 0, -2);

	}
};

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, INT)
{
	int argc = __argc;
	char** argv = __argv;
	try
	{
		TestApplication testApp;
		testApp.go();
	}
	catch (Ogre::Exception& e)
	{
		MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_ICONERROR | MB_TASKMODAL);
	}
	return 0;
}