#include "ProjectApplication.h"

//--------------------------------------------------------------------------
ProjectApplication::ProjectApplication(void)
	: mPlayerNode(0),
	mMainNode(0),
	mCameraNode(0),
	mPlayerEntity(0),
	mPlayerSpd(250),
	mPlayerAnimation(0)
{
}

//--------------------------------------------------------------------------
ProjectApplication::~ProjectApplication(void)
{
}

//--------------------------------------------------------------------------
void ProjectApplication::createScene(void)
{
	mMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MainNode");

	//Create a camera
	mCamera->setPosition(200, 500, 400);
	mCamera->lookAt(mMainNode->getPosition());
	mCamera->setNearClipDistance(.1);

	mCameraNode = mMainNode->createChildSceneNode();
	mCameraNode->attachObject(mCamera);

	//Create a viewport
	Ogre::Viewport* vp = mWindow->getViewport(0);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

	mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

	//Adding the player
	mPlayerEntity = mSceneMgr->createEntity("ninja.mesh");
	mPlayerEntity->setCastShadows(true);

	mPlayerNode = mMainNode->createChildSceneNode("PlayerNode");
	mPlayerNode->attachObject(mPlayerEntity);

	//Building the ground
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane(
		"ground",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane,
		1500, 1500, 20, 20,
		true,
		1, 5, 5,
		Ogre::Vector3::UNIT_Z);

	Ogre::Entity* groundEntity = mSceneMgr->createEntity("ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
	groundEntity->setCastShadows(false);
	groundEntity->setMaterialName("Examples/Rockwall");

	//Setting the light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));

	//Setup the animation for the ninja
	mPlayerAnimation = mPlayerEntity->getAnimationState("Idle3");
	mPlayerAnimation->setLoop(true);
	mPlayerAnimation->setEnabled(true);
}

//--------------------------------------------------------------------------
bool ProjectApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
	//bool ret = BaseApplication::frameRenderingQueued(fe);

	if (!processUnbufferedInput(fe))
		return false;

	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	mKeyboard->capture();
	mMouse->capture();

	mTrayMgr->frameRenderingQueued(fe);

	if (isPlayerMoving)
	{
		mPlayerAnimation = mPlayerEntity->getAnimationState("Walk");
		mPlayerAnimation->setLoop(true);
		mPlayerAnimation->setEnabled(true);
	}
	else {
		mPlayerAnimation = mPlayerEntity->getAnimationState("Idle3");
		mPlayerAnimation->setLoop(true);
		mPlayerAnimation->setEnabled(true);
	}

	mPlayerAnimation->addTime(fe.timeSinceLastFrame);

	return true;
}

//--------------------------------------------------------------------------
bool ProjectApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
	isPlayerMoving = false;
	//Adding move functionality to player
	Ogre::Vector3 dirVec = Ogre::Vector3::ZERO;

	if (mKeyboard->isKeyDown(OIS::KC_W))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(0), Ogre::Vector3::UNIT_Y));
		dirVec.z -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Y));
		dirVec.z += mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y));
		dirVec.x -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(270), Ogre::Vector3::UNIT_Y));
		dirVec.x += mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_W) && mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(45), Ogre::Vector3::UNIT_Y));
		dirVec.z += .5*mPlayerSpd;
		dirVec.x += .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_W) && mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(315), Ogre::Vector3::UNIT_Y));
		dirVec.z += .5*mPlayerSpd;
		dirVec.x -= .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(135), Ogre::Vector3::UNIT_Y));
		dirVec.z -= .5*mPlayerSpd;
		dirVec.x += .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("PlayerNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(225), Ogre::Vector3::UNIT_Y));
		dirVec.z -= .5*mPlayerSpd;
		dirVec.x -= .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	mSceneMgr->getSceneNode("MainNode")->translate(
		dirVec * fe.timeSinceLastFrame, Ogre::Node::TS_LOCAL);

	return true;
}
//---------------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		// Create application object
		ProjectApplication app;

		try {
			app.go();
		}
		catch (Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occurred: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif