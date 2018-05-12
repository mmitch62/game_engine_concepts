#include "ProjectApplication.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       
bool deleting;
//--------------------------------------------------------------------------
ProjectApplication::ProjectApplication(void)
	: mPlayerNode(0),
	mMainNode(0),
	mCameraNode(0),
	mPlayerEntity(0),
	mPlayerSpd(250),
	mPlayerAnimation(0),
	mDirection(Ogre::Vector3::ZERO),
	mMove(250),
	mRotate(.8),
	mRenderer(0),
	timer(4.5f),
	attackTimer(0.0f),
	attackTime(1.0f),
	cooldownTime(2.0f),
	damageCooldownTime(0.5f),
	damageCooldownTimerNinja(0.0f),
	respawnTime(5),
	numOgres(0),
	ogreMove(0.5f),
	roundOgreCount(0),
	maxOgres(1),
	newWaveAvailable(true),
	ogreHeads()
{	
}

//--------------------------------------------------------------------------
ProjectApplication::~ProjectApplication(void)
{
}
void ProjectApplication::createBulletSim(void) {
	
	//collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();
	
	//use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new   btCollisionDispatcher(collisionConfiguration);
	
	//btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();
	
	//the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;
	
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, 0, 0)); //gravity not needed in this game
}

//--------------------------------------------------------------------------
void ProjectApplication::createScene(void)
{
	srand(time(NULL));
	

	//CEGUI setup
	mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

	CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");
	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

	//set up button
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
	CEGUI::Window *sheet = wmgr.createWindow("DefaultWindow", "CEGUIDemo/Sheet");
	
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(sheet);

	healthWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	scoreWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	enemiesWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	waveWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");

	sheet->addChild(healthWindow);
	sheet->addChild(scoreWindow);
	sheet->addChild(enemiesWindow);
	sheet->addChild(waveWindow);

	healthWindow->setText("Health: " + std::to_string(ninjaHealth));
	healthWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.88f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	healthWindow->setSize(CEGUI::USize(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));

	scoreWindow->setText("Score: " + std::to_string(score));
	scoreWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	scoreWindow->setSize(CEGUI::USize(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));

	enemiesWindow->setText("Enemies Remaining: " + std::to_string(enemiesLeft));
	enemiesWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.45f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	enemiesWindow->setSize(CEGUI::USize(CEGUI::UDim(0.2f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));

	waveWindow->setText("Wave: " + std::to_string(waveNum));
	waveWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	waveWindow->setSize(CEGUI::USize(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));

	createBulletSim();

	loadNinjaAndCamera(btVector3(0, 0, 0), "NinjaEntity");

	

	//Create a viewport
	Ogre::Viewport* vp = mWindow->getViewport(0);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

	mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

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
	
}
	
	void ProjectApplication::CreateOgre(const btVector3 &Position)
	{
		bool availableOgre = false;

		
		
		Ogre::Vector3 tempPoint = Ogre::Vector3(mSceneMgr->getSceneNode("MoveNode")->getPosition());
		Ogre::Vector3 ninjaPoint = Ogre::Vector3(tempPoint.x, tempPoint.y + 200, tempPoint.z);
		// empty ogre vectors for the cubes size and position
		Ogre::Vector3 size = Ogre::Vector3::ZERO;
		Ogre::Vector3 pos = Ogre::Vector3::ZERO;
		        // Convert the bullet physics vector to the ogre vector
		        pos.x = Position.getX();
		        pos.y = Position.getY();
		        pos.z = Position.getZ();
				
				
		for (std::vector<OgreHeadStruct>::iterator iterator = ogreHeads.begin(); iterator != ogreHeads.end(); ++iterator)
			{
				if (!iterator->isActive)
				{
					availableOgre = true;
					iterator->ogreNode->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
					++numOgres;
					iterator->isActive = true;
					break;
				}
			}

		if (!availableOgre)
		{
			ptrToOgreObject = new ogreObject;
			ptrToOgreObject->entityObject = mSceneMgr->createEntity("ogrehead.mesh");
			ptrToOgreObject->entityObject->setCastShadows(false);
			ptrToOgreObject->sceneNodeObject = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			ptrToOgreObject->sceneNodeObject->attachObject(ptrToOgreObject->entityObject);

			ptrToOgreObject->sceneNodeObject->setPosition(pos);
			//the scale will always be the same
			ptrToOgreObject->sceneNodeObject->scale(1, 1, 1);
			Ogre::AxisAlignedBox boundingB = ptrToOgreObject->entityObject->getWorldBoundingBox(true);
			boundingB.scale(Ogre::Vector3(1, 1, 1));



			size = boundingB.getSize()*0.95f;
			boundingB.transformAffine(ptrToOgreObject->sceneNodeObject->_getFullTransform());
			btTransform Transform;
			Transform.setIdentity();
			Transform.setOrigin(Position);

			ptrToOgreObject->myMotionStateObject = new MyMotionState(Transform, ptrToOgreObject->sceneNodeObject);
			btVector3 HalfExtents(size.x*0.7f, size.y*0.7f, size.z*0.7f);
			ptrToOgreObject->btCollisionShapeObject = new btBoxShape(HalfExtents);
			btVector3 LocalInertia;
			//the zero is mass in the next two functions
			ptrToOgreObject->btCollisionShapeObject->calculateLocalInertia(0, LocalInertia);
			ptrToOgreObject->btRigidBodyObject = new btRigidBody(0, ptrToOgreObject->myMotionStateObject, ptrToOgreObject->btCollisionShapeObject, LocalInertia);
			//set this so it's rigidbody doesn't turn
			ptrToOgreObject->btRigidBodyObject->setLinearFactor(btVector3(1, 1, 1));


			ptrToOgreObject->objectType = "Ogre";

			ptrToOgreObject->btRigidBodyObject->setActivationState(DISABLE_DEACTIVATION);

			// Add it to the physics world
			dynamicsWorld->addRigidBody(ptrToOgreObject->btRigidBodyObject);
			collisionShapes.push_back(ptrToOgreObject->btCollisionShapeObject);

			//add ogreHead to the list

			OgreHeadStruct newHead = OgreHeadStruct(ptrToOgreObject->sceneNodeObject, ptrToOgreObject->entityObject, ptrToOgreObject->btRigidBodyObject);
			newHead.destination = ninjaPoint;
			newHead.direction = ninjaPoint;
			ogreHeads.emplace_back(newHead);
			ptrToOgreObject->headStruct = newHead;
			ptrToOgreObjects.push_back(ptrToOgreObject);

			ptrToOgreObject->btRigidBodyObject->setUserPointer(ptrToOgreObject);

			++numOgres;
		}
}
	void ProjectApplication::NewWave()
	{
		if (newWaveAvailable)
		{
			++maxOgres;
			spawning = true;
			roundOgreCount = 0;
			++waveNum;

			newWaveAvailable = false;
		}
	}
//--------------------------------------------------------------------------
bool ProjectApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
	if (numOgres == 0)
	{
		NewWave();
	}

	bool ret = BaseApplication::frameRenderingQueued(fe);
	//Increment timer 
	
	timer += fe.timeSinceLastFrame;
	damageCooldownTimerNinja += fe.timeSinceLastFrame;
	Ogre::Vector3 tempPoint = Ogre::Vector3(ptrToNinja->entityObject->getWorldBoundingBox().getCenter().x,0, ptrToNinja->entityObject->getWorldBoundingBox().getCenter().z);
	Ogre::Vector3 ninjaPoint = Ogre::Vector3(tempPoint.x, tempPoint.y + 50, tempPoint.z);

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
		mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Walk");
		mPlayerAnimation->setLoop(true);
		mPlayerAnimation->setEnabled(true);
	}
	else {
		mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Idle3");
		mPlayerAnimation->setLoop(true);
		mPlayerAnimation->setEnabled(true);
	}

	if (mKeyboard->isKeyDown(OIS::KC_SPACE) && !attacking && attackTimer == 0)
	{
		attacking = true;
	}

	if (attacking)
	{
		attackTimer += fe.timeSinceLastFrame;
		mPlayerAnimation->setLoop(false);
		mPlayerAnimation->setEnabled(false);
		mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Attack1");
		mPlayerAnimation->setLoop(false);
		mPlayerAnimation->setEnabled(true);

	}
	
	if (attackTimer >= attackTime)
	{
		attacking = false;
		attackTimer += fe.timeSinceLastFrame;
		if (attackTimer >= cooldownTime)
		{
			attackTimer = 0;
		}

	}

	mPlayerAnimation->addTime(fe.timeSinceLastFrame);
	if (timer >= respawnTime && numOgres < maxOgres && roundOgreCount < maxOgres) 
	{
		
		++roundOgreCount;
		int corner = rand() % 4 + 1;
		switch (corner) 
		{
			
		case 1:
			CreateOgre(btVector3(-650, 200, -650)); //should start in top left corner		
			break;

		case 2:
			CreateOgre(btVector3(650, 200, -650)); //should start in top right corner		
			break;

		case 3:
			CreateOgre(btVector3(-650, 200, 650)); //should start in bottom left corner		
			break;
		case 4:
			CreateOgre(btVector3(650, 200, 650)); //should start in bottom right corner		
			break;

		}
		timer = 0; //reset timer to zero	
	}


		for (std::vector<OgreHeadStruct>::iterator iterator = ogreHeads.begin(); iterator != ogreHeads.end(); ++iterator)
		{

			Ogre::Real move = ogreMove * fe.timeSinceLastFrame;
			Ogre::SceneNode* currentNode = iterator->ogreNode;
			iterator->destination = ninjaPoint;
			iterator->direction = iterator->destination - iterator->ogreNode->getPosition();
			iterator->distance = iterator->direction.normalise();
			iterator->ogreNode->lookAt(ninjaPoint, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Z);
			
			if (iterator->isActive)
			{
				iterator->ogreNode->translate(ogreMove * iterator->direction);
			}
			//adjust the ogre head's rigidbody every frame as well
			Ogre::SceneNode* locationNode = iterator->ogreNode;
			btTransform rigidTrans;
			iterator->ogreBody->getMotionState()->getWorldTransform(rigidTrans);
			//reset the rigidbody's place every frame
			btVector3 rigidLoc = btVector3(locationNode->getPosition().x, 200, locationNode->getPosition().z);
			rigidTrans.setOrigin(rigidLoc);
			iterator->ogreBody->setWorldTransform(rigidTrans);
			if (ptrToNinja->entityObject->getWorldBoundingBox().intersects(iterator->ogreEntity->getWorldBoundingBox()) && (!attacking)) {
				TakeDamage();
				

				//if(!deleting)

			}

			if (ptrToNinja->entityObject->getWorldBoundingBox().intersects(iterator->ogreEntity->getWorldBoundingBox()) && (attacking)) {
				iterator->ogreNode->setPosition(Ogre::Vector3(10000, 0, 0));
				iterator->isActive = false;
				++score;
					--numOgres;
					if (numOgres == 1)
					{
						newWaveAvailable = true;
					}

				
			}

		}
	
	if (mPlayerAnimation->hasEnded()) {
		mPlayerAnimation->setTimePosition(0);
		mPlayerAnimation->setLoop(false);
		mPlayerAnimation->setEnabled(false);
		mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Idle1");
		mPlayerAnimation->setLoop(true);
	//	mPlayerAnimation->setEnabled(true);
	}
	
	
	//update the healthWindow if the Ninja's health changes
	healthWindow->setText("Health: " + std::to_string(ninjaHealth));
	scoreWindow->setText("Score: " + std::to_string(score));
	enemiesWindow->setText("Enemies Remaining: " + std::to_string(numOgres));
	waveWindow->setText("Wave: " + std::to_string(waveNum));

	return ret;
	}	

	bool ProjectApplication::isNinja(std::string name) {
	return name == "Ninja";
		
	}
	
	bool ProjectApplication::isOgre(std::string name) {
		 return name == "Ogre";
		
	}


	void ProjectApplication::TakeDamage()
	{
		if (damageCooldownTimerNinja >= damageCooldownTime)
		{

			--ninjaHealth;
			damageCooldownTimerNinja = 0;
		}
	}

	
//--------------------------------------------------------------------------
bool ProjectApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
	isPlayerMoving = false;
	//Adding move functionality to player
	Ogre::Vector3 dirVec = Ogre::Vector3::ZERO;

	if (mKeyboard->isKeyDown(OIS::KC_W))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(0), Ogre::Vector3::UNIT_Y));
		dirVec.z -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Y));
		dirVec.z -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y));
		dirVec.z -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(270), Ogre::Vector3::UNIT_Y));
		dirVec.z -= mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_W) && mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(45), Ogre::Vector3::UNIT_Y));
		dirVec.z += .5*mPlayerSpd;
		dirVec.x += .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_W) && mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(315), Ogre::Vector3::UNIT_Y));
		dirVec.z += .5*mPlayerSpd;
		dirVec.x -= .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_A))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(135), Ogre::Vector3::UNIT_Y));
		dirVec.z -= .5*mPlayerSpd;
		dirVec.x += .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_D))
	{
		mSceneMgr->getSceneNode("MoveNode")->setOrientation(
			Ogre::Quaternion(Ogre::Degree(225), Ogre::Vector3::UNIT_Y));
		dirVec.z -= .5*mPlayerSpd;
		dirVec.x -= .5*mPlayerSpd;

		isPlayerMoving = true;
	}

	mSceneMgr->getSceneNode("MoveNode")->translate(
		dirVec * fe.timeSinceLastFrame, Ogre::Node::TS_LOCAL);


	//to update the ninja's rigidbody's location to align with the ninja
	Ogre::SceneNode* locationNode = mSceneMgr->getSceneNode("MoveNode");
	btTransform rigidTrans;
	ptrToNinja->btRigidBodyObject->getMotionState()->getWorldTransform(rigidTrans);
	//you basically reset the transform's position every frame for it to follow
	btVector3 rigidLoc = btVector3(locationNode->getPosition().x, locationNode->getPosition().y, locationNode->getPosition().z);
	rigidTrans.setOrigin(rigidLoc);
	ptrToNinja->btRigidBodyObject->setWorldTransform(rigidTrans);
	
	//retrieves the ninja so it can be animates
	//Ogre::Entity *ninja = static_cast<Ogre::Entity*>(mSceneMgr->getSceneNode("MoveNode")->getAttachedObject("NinjaEntity"));
	
	//animates the ninja depending on whether it's moving or if it's idle


	return true;
}



	// Game over
	//if (playing == false) {

	//	// Record high score
	//	if (gameTimer > highScore)
	//		highScore = gameTimer;

	//	// Update windows
	//	bestTime->setText("Best Time : " + std::to_string(highScore));
	//	startQuit->setText("Your Final " + currentTime->getText() + "\nBest Time : "
	//		+ std::to_string(highScore) + "\n\nPlay: (Enter)\nQuit Game: (Esc)");
	//	startQuit->show();

	//	// Reset game timers
	//	gameTimer = 0;
	//	collisionTimer = 0;

	//	// Put player back into orginal position
	//	btTransform Transform;
	//	Transform.setIdentity();
	//	Transform.setOrigin(btVector3(0, 120, 0));
	//	player->body->setWorldTransform(Transform);
	//}


void ProjectApplication::loadNinjaAndCamera(const btVector3 &Position, std::string name) {
	//first set up the moveNode so the NinjaNode can be attached to it
		//the moveNode is what controls the Ninja's Movement
		 Ogre::SceneNode* moveNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MoveNode");
	 //ninja and it's physics setup
		 // empty ogre vectors for the cubes size and position
	Ogre::Vector3 size = Ogre::Vector3::ZERO;
	Ogre::Vector3 pos = Ogre::Vector3::ZERO;
	Ogre::Entity *ninja = mSceneMgr->createEntity("ninja.mesh");
	
	 // Convert the bullet physics vector to the ogre vector
	pos.x = Position.getX();
	pos.y = Position.getY();
	pos.z = Position.getZ();
	
	ptrToOgreObject = new ogreObject;
	ptrToOgreObject->entityObject = ninja;
	ptrToOgreObject->entityObject->setCastShadows(true);
	
	ptrToOgreObject->sceneNodeObject = moveNode;
	ptrToOgreObject->sceneNodeObject->attachObject(ptrToOgreObject->entityObject);
	Ogre::AxisAlignedBox boundingB = ptrToOgreObject->entityObject->getBoundingBox();


	boundingB.scale(Ogre::Vector3(0.5f, 0.25f, 0.5f));
	size = boundingB.getSize()*0.5f;
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(Position);

	ptrToOgreObject->myMotionStateObject = new MyMotionState(Transform, ptrToOgreObject->sceneNodeObject);
	//Give the rigid body half the size
	btVector3 HalfExtents(size.x, size.y, size.z);
	ptrToOgreObject->btCollisionShapeObject = new btBoxShape(HalfExtents);
	btVector3 LocalInertia = btVector3(0, 0, 0);
	ptrToOgreObject->btCollisionShapeObject->calculateLocalInertia(0, LocalInertia);
	ptrToOgreObject->btRigidBodyObject = new btRigidBody(0, ptrToOgreObject->myMotionStateObject, ptrToOgreObject->btCollisionShapeObject, LocalInertia);
	ptrToOgreObject->btRigidBodyObject->setAngularFactor(btVector3(0, 0, 0));
	ptrToOgreObject->objectType = "Ninja";

	ptrToOgreObject->btRigidBodyObject->setLinearFactor(btVector3(0, 0, 0));
	// Store a pointer to the Ogre Node so we can update it later
	ptrToOgreObject->btRigidBodyObject->setUserPointer((void *)(ptrToOgreObject));

	ptrToOgreObject->btCollisionObjectObject = ptrToOgreObject->btRigidBodyObject;
	ptrToOgreObject->objectDelete = false;
	ptrToOgreObjects.push_back(ptrToOgreObject);

	
	//ptrToNinja->sceneNodeObject->attachObject(ptrToNinja->entityObject);
	////the scale will always be the same
	//ptrToNinja->sceneNodeObject->scale(1, 1, 1);
	//
	////the zeros in these two are so then wthe ninja has no mass and is easy to control
	//ptrToNinja->btRigidBodyObject->setActivationState(DISABLE_DEACTIVATION);
	// Add it to the physics world
	dynamicsWorld->addRigidBody(ptrToOgreObject->btRigidBodyObject);
	collisionShapes.push_back(ptrToOgreObject->btCollisionShapeObject);
	
	ptrToNinja = ptrToOgreObject;
	
	mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Idle3");
	mPlayerAnimation->setLoop(true);
	mPlayerAnimation->setEnabled(true);
	
	//camera setup
		{
		//camera will be child of ninja node, so it follows it's movement around the map
			Ogre::SceneNode* cameraNode = ptrToNinja->sceneNodeObject->createChildSceneNode("CameraNode");
			cameraNode->attachObject(mCamera);
			cameraNode->setPosition(ptrToNinja->sceneNodeObject->getPosition().x, ptrToNinja->sceneNodeObject->getPosition().y + 1000, ptrToNinja->sceneNodeObject->getPosition().z); //directly above the ninjaNode
			mCamera->setNearClipDistance(.1);
		
			//make camera look at ninja node
			mCamera->lookAt(ptrToNinja->sceneNodeObject->getPosition());
		    
			//set camera Inherit Orientation to false so it doesn't rotate with the ninja
			cameraNode->setInheritOrientation(false);
		
			//more camera setup
			bool infiniteClip =
			mRoot->getRenderSystem()->getCapabilities()->hasCapability(
				Ogre::RSC_INFINITE_FAR_PLANE);
		
			if (infiniteClip)
			mCamera->setFarClipDistance(0);
		else
			mCamera->setFarClipDistance(50000);
		        }
		

}
bool ProjectApplication::frameStarted(const Ogre::FrameEvent &evt)
{
	
	mKeyboard->capture();
	mMouse->capture();
	dynamicsWorld->stepSimulation(evt.timeSinceLastFrame);
	CheckCollisions();
	return true;
}

bool ProjectApplication::frameEnded(const Ogre::FrameEvent &evt) {
	Ogre::LogManager::getSingleton().logMessage("blahblah"); 
	for (int i = 0; i < ptrToOgreObjects.size(); i++) {
		
		ogreObject* currentObject = ptrToOgreObjects[i];
		if (isNinja(currentObject->objectType)) {
			
			std::vector<ogreObject*> collidedObjects = currentObject->objectCollisions;

			for (int j = 0; j < collidedObjects.size(); j++) {
				std::cout << "delet\n";
				collidedObjects[j]->objectDelete = true;
			}
		}

		if (currentObject->objectDelete) {
			++score;
			//bool isSpecial = currentObject->isSpecial;
			RemoveObject(currentObject, i);

			
		}
	}
	return true;
}

void ProjectApplication::RemoveObject(ogreObject* object, int index) {
	object->entityObject->detachFromParent();
	mSceneMgr->destroyEntity(object->entityObject);
	object->entityObject = NULL;
	mSceneMgr->destroySceneNode(object->sceneNodeObject);
	object->sceneNodeObject = NULL;

	dynamicsWorld->removeRigidBody(object->btRigidBodyObject);
	if (object->btRigidBodyObject && object->btRigidBodyObject->getMotionState())
		delete object->btRigidBodyObject->getMotionState();
	object->myMotionStateObject = NULL;
	dynamicsWorld->removeCollisionObject(object->btCollisionObjectObject);
	delete object->btCollisionObjectObject;
	object->btCollisionObjectObject = NULL;
	object->btRigidBodyObject = NULL;
	delete object->btCollisionShapeObject;
	object->btCollisionShapeObject = NULL;

	ptrToOgreObjects.erase(ptrToOgreObjects.begin() + index);
}

void ProjectApplication::CheckCollisions() {
	//dynamicsworld->stepsimulation called in frameStarted function
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {

		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		ogreObject* ogreA = (ogreObject*)obA->getUserPointer();
		ogreObject* ogreB = (ogreObject*)obB->getUserPointer();

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f) {
				if (ogreA == NULL || ogreB == NULL) {
					++score;
					continue;
				}

				if (isNinja(ogreA->objectType) && isOgre(ogreB->objectType)) {
					if (!ogreB->objectDelete) {
						++score;

						ogreA->objectCollisions.push_back(ogreB);
					}
				}

				if (isNinja(ogreB->objectType) && isOgre(ogreA->objectType)) {
					if (!ogreA->objectDelete) {
						++score;

						ogreB->objectCollisions.push_back(ogreA);
					}
				}
			}
		}
	}
}

ProjectApplication::ogreObject* ProjectApplication::getOgreObject(const btCollisionObject * obj) {
	for (int i = 0; i < ptrToOgreObjects.size(); ++i) {
		if (ptrToOgreObjects[i]->btCollisionObjectObject == obj)
			return ptrToOgreObjects[i];

	}
	ogreObject* ret = new ogreObject;
	return ret;
}






void ProjectApplication::eraseObject(ogreObject* object) {
	if (object->sceneNodeObject != NULL) {
		// delete the ogre aspect of the object
		// detach the entity from the parent sceneNode, destroy the entity, destroy the sceneNode, and set the sceneNode to NULL
		object->sceneNodeObject->detachObject(object->entityObject);
		mSceneMgr->destroyEntity(object->entityObject);
		mSceneMgr->destroySceneNode(object->sceneNodeObject);
		object->sceneNodeObject = NULL;

		// delete the bullet aspect of the object, ours should always have motion state
		if (object->btRigidBodyObject && object->btRigidBodyObject->getMotionState())
			delete object->btRigidBodyObject->getMotionState();

		delete object->btRigidBodyObject->getCollisionShape();

		dynamicsWorld->removeCollisionObject(object->btRigidBodyObject);

		object->btRigidBodyObject = NULL;

		for (int i = 0; i < ogreHeads.size(); ++i) {
			if (object->headStruct.ogreNode == ogreHeads[i].ogreNode)
				ogreHeads.erase(ogreHeads.begin() + i);
		}

		removeDynamicOgreObject(object, ptrToOgreObjects);
	}
}

void ProjectApplication::removeDynamicOgreObject(ogreObject * ptrToOgreObject, std::vector<ogreObject *> &ptrToOgreObjects)
{
	// only iterate through the vector if it is not empty, it should NOT be empty
	if (!ptrToOgreObjects.empty())
		// an iterator returned from an erase is a valid iterator to the next element in the vector
		for (std::vector<ogreObject *>::iterator itr = ptrToOgreObjects.begin(); itr != ptrToOgreObjects.end(); ++itr) {
			// find the match, it must be in the vector because the object exists
			if (*itr == ptrToOgreObject) {
				// delete that dynamic ogreObject
				delete *itr;
				// set the pointer to that ogreObject to NUll
				*itr = NULL;
				// and then delete that pointer out of the vector
				ptrToOgreObjects.erase(itr);
				// no need to continue iterating and the iterator just became invalid
				break;
			}
		}
}

void ProjectApplication::checkDeletions() {
	/*for (int i = 0; i < ptrToOgreObjects.size(); i++) {
		if (ptrToOgreObjects[i]->canDelete == true)
			eraseObject(ptrToOgreObjects[i]);
		
	}*/
}


bool ProjectApplication::keyPressed(const OIS::KeyEvent& ke)
{
	switch (ke.key)
	{
	case OIS::KC_UP:
	case OIS::KC_W:
		
		break;

	case OIS::KC_DOWN:
	case OIS::KC_S:
		
		break;

	case OIS::KC_LEFT:
	case OIS::KC_A:
		
		break;

	case OIS::KC_H:

		break;
		break;

	default:
		break;
	}
	return true;
}


void ProjectApplication::createFrameListener()
	{
	//for CEGUI setup
	 BaseApplication::createFrameListener();
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	
	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	
	mInputManager = OIS::InputManager::createInputSystem(pl);
	
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
	
	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
	
	//Set initial mouse clipping size
	windowResized(mWindow);
	
	//Register as a Window listener
	Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
	
	//this is to adjust the mouse state so it lines up with the Ogre mouse
	//this MUST be included or else mouse cursor on CEGUI won't line up with Ogre's
	//Ogre's starts at (0,0) as in the top left corner, CEGUI starts (0.5, 0.5) as in the center of the screen
	OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
	mutableMouseState.X.abs = mCamera->getViewport()->getActualWidth() / 2;
	mutableMouseState.Y.abs = mCamera->getViewport()->getActualHeight() / 2;
	
	mRoot->addFrameListener(this);
	
	}







bool ProjectApplication::keyReleased(const OIS::KeyEvent& ke)
{
	return true;
}

	//function that activates when button pressed
	bool ProjectApplication::quitGame(const CEGUI::EventArgs &e) {
		        mShutDown = true;
		        return true;
	
}


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