#include "ExampleApplication.h"
#include "ExampleFrameListener.h"
#include "OgrePlatform.h"
#include "windows.h"
#include "Ogre.h"

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
	virtual void createFrameListener(void)
	{
		mFrameListener= new TestFrameListener(mSceneMgr, mWindow, mCamera);
		mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
	}

	virtual void createScene(void)
	{
		mCamera->setPosition(0, 0, 0);
		mCamera->lookAt(0, 0, -1);
		mCamera->setNearClipDistance(1);
		mCamera->setFarClipDistance(1000);
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