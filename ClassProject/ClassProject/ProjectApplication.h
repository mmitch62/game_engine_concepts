#ifndef __ProjectApplication_h_
#define __ProjectApplication_h_

#include "BaseApplication.h"
#include "OgreHead.h"

class MyMotionState : public btMotionState {
		public:
			        MyMotionState(const btTransform &initialpos, Ogre::SceneNode *node) {
				                mVisibleobj = node;
				                mPos1 = initialpos;
			
		}
			        virtual ~MyMotionState() {    }
			        void setNode(Ogre::SceneNode *node) {
				                mVisibleobj = node;
			
		}
			        virtual void getWorldTransform(btTransform &worldTrans) const {
				                worldTrans = mPos1;
			
		}
			        virtual void setWorldTransform(const btTransform &worldTrans) {
				                if (NULL == mVisibleobj) return; // silently return before we set a node
				                btQuaternion rot = worldTrans.getRotation();
				                mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
				                btVector3 pos = worldTrans.getOrigin();
				                // TODO **** XXX need to fix this up such that it renders properly since this doesnt know the scale of the node
					                // also the getCube function returns a cube that isnt centered on Z
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
	virtual void createScene(void);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
	void CreateBall(const btVector3 &Position, btScalar Mass, const btVector3 &scale, std::string name, Ogre::Vector3 direction);
	void CreateOgre(const btVector3 &Position);
	bool frameStarted(const Ogre::FrameEvent &evt);
	//Ogre::ManualObject *createCubeMesh(Ogre::String name, Ogre::String matName);
	CEGUI::OgreRenderer* mRenderer;
	bool quitGame(const CEGUI::EventArgs &e);
private:
	void createBulletSim(void);
	bool processUnbufferedInput(const Ogre::FrameEvent& fe);


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
	double timer, respawnTime;
	int numOgres; int maxOgres;

	bool isPlayerMoving;
	double sphereVelocity; int sphereIt;
	std::vector<OgreHeadStruct> ogreHeads;

	OgreBites::Label* mInfoLabel;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btCollisionShape* groundShape;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;


	int ninjaHealth = 10;

	CEGUI::Window *healthWindow;

struct ogreObject {
	Ogre::Entity * entityObject;
	 Ogre::SceneNode * sceneNodeObject;
	 MyMotionState * myMotionStateObject;
	btCollisionShape * btCollisionShapeObject;
	                btRigidBody * btRigidBodyObject;
	                btCollisionObject * btCollisionObjectObject;
		                std::string objectType;
	               Ogre::Real timer = 0;
	               bool canDelete = false;
	      OgreHeadStruct headStruct;
	 int health = 3;

};
ogreObject* getOgreObject(const btCollisionObject * obj);

	        ogreObject * ptrToOgreObject; // pointer to an ogre Object struct
	        std::vector<ogreObject *> ptrToOgreObjects; // vector of pointers to actual ogreObject structs

	        ogreObject* ninjaObject;

	        void checkCollisions();
	        void checkDeletions();
	        void removeDynamicOgreObject(ogreObject * ptrToOgreObject, std::vector<ogreObject *> &ptrToOgreObjects);
	       void eraseObject(ogreObject * object);
	       // void removeOgreHeadObject(OgreHeadStruct headStruct, std::vector<OgreHeadStruct> &ogreHeads);
};
#endif