#ifndef __ProjectApplication_h_
#define __ProjectApplication_h_

#include "BaseApplication.h"
#include "OgreHead.h"

class MyMotionState : public btMotionState {
		public:			       
			MyMotionState(const btTransform &initialpos, Ogre::SceneNode *node) 
			{				                
				mVisibleobj = node;
				mPos1 = initialpos;		
			}
			        
			virtual ~MyMotionState() {    }
			        
			void setNode(Ogre::SceneNode *node) 
			{				                
				mVisibleobj = node;	
			}
			        
			virtual void getWorldTransform(btTransform &worldTrans) const 
			{				                
				worldTrans = mPos1;					
			}
			        
			virtual void setWorldTransform(const btTransform &worldTrans) 
			{
				                
				if (NULL == mVisibleobj) return; // silently return before we set a node				                
				btQuaternion rot = worldTrans.getRotation();				                
				mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());				                
				btVector3 pos = worldTrans.getOrigin();				                
				mVisibleobj->setPosition(pos.x(), pos.y() + 5, pos.z() - 5);
	
			}
			
protected:
				        
	Ogre::SceneNode *mVisibleobj;			        
	btTransform mPos1;
			
};

class ProjectApplication : public BaseApplication
{
public:
	ProjectApplication(void);
	virtual ~ProjectApplication(void);

protected:
	void loadNinjaAndCamera(const btVector3 &Position, std::string name);
	void NewWave();
	void NewGame();
	void Die();
	void TakeDamage();
	virtual void createScene(void);
	virtual void createFrameListener();
	//virtual bool frameEnded(const Ogre::FrameEvent &evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
	virtual bool keyPressed(const OIS::KeyEvent& ke);
	virtual bool keyReleased(const OIS::KeyEvent& ke);
	void CreateOgre(const btVector3 &Position);
	bool frameStarted(const Ogre::FrameEvent &evt);
	CEGUI::OgreRenderer* mRenderer;
private:
	void createBulletSim(void);
	bool processUnbufferedInput(const Ogre::FrameEvent& fe);
	bool isPlayerMoving;
	bool attacking;
	bool spawning;
	bool gameOver;
	bool isTakingDamage;
	bool newWaveAvailable;
	bool playing;
	bool dead;
	double timer,attackTimer,attackTime,cooldownTime, damageCooldownTime, damageCooldownTimerNinja, respawnTime;
	int numOgres; int maxOgres;
	int roundOgreCount;
	int ninjaHealth = 10;
	int score = 0;
	int waveNum = 0;
	int enemiesLeft = 0;
	Ogre::AxisAlignedBox ninBox;

	Ogre::SceneNode* mMainNode;
	Ogre::SceneNode* mPlayerNode;
	Ogre::SceneNode* mCameraNode;
	Ogre::Entity* mPlayerEntity;
	Ogre::AnimationState* mPlayerAnimation;

	Ogre::Real mPlayerSpd;
	Ogre::Real ogreMove;
	Ogre::Real mRotate;
	Ogre::Real mMove;
	Ogre::Vector3 mDirection;
	//timers and variables to control how many enemies there are
	

	OgreBites::Label* mInfoLabel;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btCollisionShape* groundShape;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	std::vector<OgreHeadStruct> ogreHeads;

	CEGUI::Window *healthWindow;
	CEGUI::Window *scoreWindow;
	CEGUI::Window *enemiesWindow;

	CEGUI::Window *waveWindow;
	CEGUI::Window* startQuit;

	//struct OgreHeadStruct;
	struct ogreObject 
	{
		
		Ogre::Entity * entityObject;
		Ogre::SceneNode * sceneNodeObject;
		MyMotionState * myMotionStateObject;
		btCollisionShape * btCollisionShapeObject;
		btRigidBody * btRigidBodyObject;
	    btCollisionObject * btCollisionObjectObject;
		std::string objectType;
	    Ogre::Real timer = 0;
	    bool objectDelete = false;
	    OgreHeadStruct headStruct;
		int health = 1;
		std::vector<ogreObject *> objectCollisions;
};

	ogreObject* ptrToNinja;

	ogreObject * ptrToOgreObject; // pointer to an ogre Object struct
	std::vector<ogreObject *> ptrToOgreObjects; // vector of pointers to actual ogreObject structs
	ogreObject* ninjaObject;
	

};
#endif