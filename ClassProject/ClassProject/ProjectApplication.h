#ifndef __ProjectApplication_h_
#define __ProjectApplication_h_

#include "BaseApplication.h"

class ProjectApplication : public BaseApplication
{
public:
	ProjectApplication(void);
	virtual ~ProjectApplication(void);

protected:
	virtual void createScene(void);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);

private:
	bool processUnbufferedInput(const Ogre::FrameEvent& fe);

	Ogre::SceneNode* mMainNode;
	Ogre::SceneNode* mPlayerNode;
	Ogre::SceneNode* mCameraNode;
	Ogre::Entity* mPlayerEntity;

	Ogre::AnimationState* mPlayerAnimation;

	Ogre::Real mPlayerSpd;

	bool isPlayerMoving;
};

#endif