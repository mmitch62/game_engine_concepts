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

	// Game Over / Start Screen
	startQuit = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", "overVal");
	startQuit->setProperty("HorzFormatting", "WordWrapCenterAligned");
	sheet->addChild(startQuit);
	startQuit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.4f, 0.0f)));
	startQuit->setSize(CEGUI::USize(CEGUI::UDim(0.25f, 0.0f), CEGUI::UDim(0.25f, 0.0f)));
	startQuit->setText("Welcome\n\n\
		Use WASD to move up, left, down, and right.\n\
		Press Space to attack.\n\
		\nPlay: (Enter)\nQuit Game: (Esc)");
	startQuit->show();

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
				
		
		//Checking to see if there is an inactive somewhere
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

		//if there is an inactive ogre, place it where you are spawning a "new" ogre
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


			//ptrToOgreObject->objectType = "Ogre";

			//ptrToOgreObject->btRigidBodyObject->setActivationState(DISABLE_DEACTIVATION);

			//// Add it to the physics world
			//dynamicsWorld->addRigidBody(ptrToOgreObject->btRigidBodyObject);
			//collisionShapes.push_back(ptrToOgreObject->btCollisionShapeObject);

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
			++maxOgres; //increase ogre limit
			spawning = true;
			roundOgreCount = 0;
			++waveNum;

			newWaveAvailable = false;
		}
	}
	void ProjectApplication::Die() //Player dying
	{
		gameOver == true;
		playing = false;

		startQuit->setText("GAME OVER\n\nFinal Score " + std::to_string(score) + "\n\nPress esc to quit \nPress N to start a new game.");

		startQuit->show();

		
	}

	void ProjectApplication::NewGame()
	{
		//spawn the first two ogres by moving the first two in the vector and activating them if need be
		for (std::vector<OgreHeadStruct>::iterator iterator = ogreHeads.begin(); iterator != ogreHeads.end(); ++iterator)
		{
			if (iterator == ogreHeads.begin() || iterator == ogreHeads.begin() + 1)
			{
				iterator->ogreNode->setPosition(Ogre::Vector3(-650, 200, -650));
			
			}

			//Any remaining ones will be mooved far off screen and deactivated
			if (iterator != ogreHeads.begin() || iterator != ogreHeads.begin() + 1)
			{
				iterator->ogreNode->setPosition(Ogre::Vector3(-65000, 200, -650));
				iterator->isActive = false;

			}

		}
		//NewWave();

		//reset variables
		score = 0;
		numOgres = 0;
		roundOgreCount = 0;
		maxOgres = 1;
		ninjaHealth = 10;
		waveNum = 1;
		
		gameOver = false;
		





		//reset ninja position
		ptrToNinja->sceneNodeObject->setPosition(Ogre::Vector3(0, 0, 0));
	}
//--------------------------------------------------------------------------
bool ProjectApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
	if (ninjaHealth <= 0 && !dead)
	{
		dead = true;
		mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Death1");
		mPlayerAnimation->setLoop(false);
		mPlayerAnimation->setEnabled(true);
		Die();
	}
	
	
	//exit game on escape
	if (mKeyboard->isKeyDown(OIS::KC_ESCAPE)) {
		mShutDown = true;
	}
	
	//Start game
	if (mKeyboard->isKeyDown(OIS::KC_RETURN) && !playing && !gameOver) {
		playing = true;
		startQuit->setVisible(false);
	}

	//Restart game
	if (mKeyboard->isKeyDown(OIS::KC_N)) {
		playing = true;
		dead = false;
		NewGame();	

		startQuit->setVisible(false);
	}


	

	bool ret = BaseApplication::frameRenderingQueued(fe);
	//Increment timer 
	
	
	if (!processUnbufferedInput(fe))
		return false;

	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	mKeyboard->capture();
	//mMouse->capture();
	if (!playing) return true;
	mTrayMgr->frameRenderingQueued(fe);

		//incrementing timers
		timer += fe.timeSinceLastFrame;
		damageCooldownTimerNinja += fe.timeSinceLastFrame;

		//Points to track ninja
		Ogre::Vector3 tempPoint = Ogre::Vector3(ptrToNinja->entityObject->getWorldBoundingBox().getCenter().x, 0, ptrToNinja->entityObject->getWorldBoundingBox().getCenter().z);
		Ogre::Vector3 ninjaPoint = Ogre::Vector3(tempPoint.x, tempPoint.y + 50, tempPoint.z);


		if (!dead) //Only animate player while alive
		{

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
		}

		//Start a new Wave if all ogres are killed
		if (numOgres <= 0) 
		{
			NewWave();
		}

		if (attackTimer >= attackTime) //Prevents player frome attacking all of the time
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

			++roundOgreCount; //Round persistent counter of ogres. So the ogres aren't spawned infinitely

			//Randomly choose a psition for an ogre to be spawned
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
			//Moving active ogres towards ninja
			Ogre::Real move = ogreMove * fe.timeSinceLastFrame;
			Ogre::SceneNode* currentNode = iterator->ogreNode;
			iterator->destination = ninjaPoint;
			iterator->direction = iterator->destination - iterator->ogreNode->getPosition();
			iterator->distance = iterator->direction.normalise();
			iterator->ogreNode->lookAt(ninjaPoint, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Z);

			if (iterator->isActive)//Ony move if active
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
			//If an ogre and player collide while the player isn't attacking
			if (ptrToNinja->entityObject->getWorldBoundingBox().intersects(iterator->ogreEntity->getWorldBoundingBox()) && (!attacking))
			{
				TakeDamage();
			}
			//If an ogre and player collide while the player i attacking

			if (ptrToNinja->entityObject->getWorldBoundingBox().intersects(iterator->ogreEntity->getWorldBoundingBox()) && (attacking))
			{
				iterator->ogreNode->setPosition(Ogre::Vector3(10000, 0, 0));//"Deleting" ogre. Moving off screen.
				iterator->isActive = false;
				++score;
				--numOgres;
				if (numOgres == 0)//Only check if there are 0 ogres when one is being killed. This prevents multiple waves beig incremented at a time
				{

					newWaveAvailable = true;
				}
			}

		}
		if (!dead)
		{
			if (mPlayerAnimation->hasEnded()) {
				mPlayerAnimation->setTimePosition(0);
				mPlayerAnimation->setLoop(false);
				mPlayerAnimation->setEnabled(false);
				mPlayerAnimation = ptrToNinja->entityObject->getAnimationState("Idle1");
				mPlayerAnimation->setLoop(true);
				//	mPlayerAnimation->setEnabled(true);
			}
		}
	
	//update the healthWindow if the Ninja's health changes
	healthWindow->setText("Health: " + std::to_string(ninjaHealth));
	scoreWindow->setText("Score: " + std::to_string(score));
	enemiesWindow->setText("Enemies Remaining: " + std::to_string(numOgres));
	waveWindow->setText("Wave: " + std::to_string(waveNum));

	return ret;
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
	if (!playing) return true;
	
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
			//	dirVec.z += .25*mPlayerSpd;
			dirVec.x += .25*mPlayerSpd;

			isPlayerMoving = true;
		}

		if (mKeyboard->isKeyDown(OIS::KC_W) && mKeyboard->isKeyDown(OIS::KC_D))
		{
			mSceneMgr->getSceneNode("MoveNode")->setOrientation(
				Ogre::Quaternion(Ogre::Degree(315), Ogre::Vector3::UNIT_Y));
			//dirVec.z += .25*mPlayerSpd;
			dirVec.x -= .25*mPlayerSpd;

			isPlayerMoving = true;
		}

		if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_A))
		{
			mSceneMgr->getSceneNode("MoveNode")->setOrientation(
				Ogre::Quaternion(Ogre::Degree(135), Ogre::Vector3::UNIT_Y));
			//dirVec.z -= .01*mPlayerSpd;
			dirVec.x += .01*mPlayerSpd;

			isPlayerMoving = true;
		}

		if (mKeyboard->isKeyDown(OIS::KC_S) && mKeyboard->isKeyDown(OIS::KC_D))
		{
			mSceneMgr->getSceneNode("MoveNode")->setOrientation(
				Ogre::Quaternion(Ogre::Degree(225), Ogre::Vector3::UNIT_Y));
			//.z -= .01*mPlayerSpd;
			dirVec.x -= .01*mPlayerSpd;

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

	
	return true;
}

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


	////the zeros in these two are so then wthe ninja has no mass and is easy to control
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
	dynamicsWorld->stepSimulation(evt.timeSinceLastFrame);

	return true;
}

bool ProjectApplication::keyPressed(const OIS::KeyEvent& ke)
{
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

	mRoot->addFrameListener(this);
	
	}


bool ProjectApplication::keyReleased(const OIS::KeyEvent& ke)
{
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