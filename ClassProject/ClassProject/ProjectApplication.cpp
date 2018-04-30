#include "ProjectApplication.h"

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
	sphereVelocity(100),
	sphereIt(0),
	timer(4.5f), 
	respawnTime(5),
	numOgres(0),
	ogreMove(0.5f), 
	maxOgres(8),
	ogreHeads()
{
	
}

//--------------------------------------------------------------------------
ProjectApplication::~ProjectApplication(void)
{
}
void ProjectApplication::createBulletSim(void) {
		        ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
			        collisionConfiguration = new btDefaultCollisionConfiguration();
	
			        ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
			        dispatcher = new   btCollisionDispatcher(collisionConfiguration);
	
			        ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
			        overlappingPairCache = new btDbvtBroadphase();
	
			        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
			        solver = new btSequentialImpulseConstraintSolver;
	
			        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		        dynamicsWorld->setGravity(btVector3(0, 0, 0)); //gravity not needed in this game
	
}

	


//--------------------------------------------------------------------------
void ProjectApplication::createScene(void)
{

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
	CEGUI::Window *quit = wmgr.createWindow("TaharezLook/Button", "CEGUIDemo/QuitButton");
	quit->setText("Quit");
	quit->setSize(CEGUI::USize(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)));
	quit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.85f, 0.0f), CEGUI::UDim(0.95f, 0.0f)));
	sheet->addChild(quit);
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(sheet);
	quit->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&ProjectApplication::quitGame, this)); //it's cubeMaker because it activates that event

																														   //setup the textbox that displays the Ninja's health
	healthWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	sheet->addChild(healthWindow);
	healthWindow->setText("Health: " + std::to_string(ninjaHealth));
	healthWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.88f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	healthWindow->setSize(CEGUI::USize(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));
	/*
	//setup the score window
	scoreWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	sheet->addChild(scoreWindow);
	scoreWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.02f, 0.0f), CEGUI::UDim(0.05f, 0.0f)));
	scoreWindow->setSize(CEGUI::USize(CEGUI::UDim(0.1f, 0.0f), CEGUI::UDim(0.1f, 0.0f)));
	scoreWindow->setText("Score: " + std::to_string(score));

	//setup the gameOverWindow
	gameOverWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUIDemo/Title");
	sheet->addChild(gameOverWindow);
	gameOverWindow->setText("Game Over!\n\nRestart the game to play again.");
	gameOverWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.0f), CEGUI::UDim(0.25f, 0.0f)));
	gameOverWindow->setSize(CEGUI::USize(CEGUI::UDim(0.5f, 0.0f), CEGUI::UDim(0.5f, 0.0f)));
	gameOverWindow->hide();
	*/
	createBulletSim();


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

void ProjectApplication::CreateBall(const btVector3 &Position, btScalar Mass, const btVector3 &scale, std::string name, Ogre::Vector3 direction) {
		        // empty ogre vectors for the cubes size and position
			        Ogre::Vector3 size = Ogre::Vector3::ZERO;
		        Ogre::Vector3 pos = Ogre::Vector3::ZERO;
		        // Convert the bullet physics vector to the ogre vector
			        pos.x = Position.getX();
		        pos.y = Position.getY();
		        pos.z = Position.getZ();
	
			        ptrToOgreObject = new ogreObject;
	
			        //put info into ogreobject
			        ptrToOgreObject->entityObject = mSceneMgr->createEntity(name, "sphere.mesh");
	
			        ptrToOgreObject->entityObject->setCastShadows(false);
		        ptrToOgreObject->sceneNodeObject = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		        ptrToOgreObject->sceneNodeObject->attachObject(ptrToOgreObject->entityObject);
		        ptrToOgreObject->sceneNodeObject->scale(Ogre::Vector3(scale.getX(), scale.getY(), scale.getZ()));
	
			        //Give the sphere texture!
			        //ptrToOgreObject->entityObject->setMaterialName("Examples/sphere");
		
			        Ogre::AxisAlignedBox boundingB = ptrToOgreObject->entityObject->getBoundingBox();
	
			        boundingB.scale(Ogre::Vector3(scale.getX(), scale.getY(), scale.getZ()));
		        size = boundingB.getSize()*0.95f;
		        btTransform Transform;
		        Transform.setIdentity();
		        Transform.setOrigin(Position);
		        ptrToOgreObject->myMotionStateObject = new MyMotionState(Transform, ptrToOgreObject->sceneNodeObject);
		        //Give the rigid body half the size
			        // of our cube and tell it to create a BoxShape (cube)
			        btVector3 HalfExtents(size.x*0.5f, size.y*0.5f, size.z*0.5f);
		        ptrToOgreObject->btCollisionShapeObject = new btBoxShape(HalfExtents);
		        btVector3 LocalInertia;
		        ptrToOgreObject->btCollisionShapeObject->calculateLocalInertia(Mass, LocalInertia);
		        ptrToOgreObject->btRigidBodyObject = new btRigidBody(Mass, ptrToOgreObject->myMotionStateObject, ptrToOgreObject->btCollisionShapeObject, LocalInertia);
		        // btRigidBody is derived from btCollisionObject so the pointer ends up being the same
		
			        ptrToOgreObject->objectType = "Sphere";
	
			        //change rigidBody's activation state so it doesn't deactivate
			        ptrToOgreObject->btRigidBodyObject->setActivationState(DISABLE_DEACTIVATION);
	
			        // Add it to the physics world
			        dynamicsWorld->addRigidBody(ptrToOgreObject->btRigidBodyObject);
		        collisionShapes.push_back(ptrToOgreObject->btCollisionShapeObject);
	
			        //Convert the vector to bullet's so you can give velocity
			        btVector3 initVelocity = btVector3(direction.x, direction.y, direction.z);
	
			        //Give the bullet vector a speed
			        initVelocity.normalize();
		        initVelocity *= 500;
	
			        //btVector3 initVelocity = btVector3(50, -50, 50);
			        ptrToOgreObject->btRigidBodyObject->setLinearVelocity(initVelocity);
	
			        //add it to the list
			        collisionShapes.push_back(ptrToOgreObject->btCollisionShapeObject);
		        ptrToOgreObjects.push_back(ptrToOgreObject);
	
			        //store a pointer so you can access later
			        ptrToOgreObject->btRigidBodyObject->setUserPointer(ptrToOgreObject);
	
		
}

	
	void ProjectApplication::CreateOgre(const btVector3 &Position) {
		Ogre::Vector3 tempPoint = Ogre::Vector3(mSceneMgr->getSceneNode("PlayerNode")->getPosition());
		Ogre::Vector3 ninjaPoint = Ogre::Vector3(tempPoint.x, tempPoint.y + 50, tempPoint.z);
		        // empty ogre vectors for the cubes size and position
			        Ogre::Vector3 size = Ogre::Vector3::ZERO;
		        Ogre::Vector3 pos = Ogre::Vector3::ZERO;
		        // Convert the bullet physics vector to the ogre vector
			        pos.x = Position.getX();
		        pos.y = Position.getY();
		        pos.z = Position.getZ();
	
			        ptrToOgreObject = new ogreObject;
	
			        ptrToOgreObject->entityObject = mSceneMgr->createEntity("ogrehead.mesh");
		        ptrToOgreObject->entityObject->setCastShadows(false);
		        ptrToOgreObject->sceneNodeObject = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		        ptrToOgreObject->sceneNodeObject->attachObject(ptrToOgreObject->entityObject);
		        //set ogreNode position --- Without this, RigidBody and Node start in different positions?
			        ptrToOgreObject->sceneNodeObject->setPosition(pos);
		        //the scale will always be the same
			        ptrToOgreObject->sceneNodeObject->scale(1, 1, 1);
		        Ogre::AxisAlignedBox boundingB = ptrToOgreObject->entityObject->getBoundingBox();
		        boundingB.scale(Ogre::Vector3(1, 1, 1));
		        size = boundingB.getSize()*0.95f;
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
		
			        OgreHeadStruct newHead = OgreHeadStruct(ptrToOgreObject->sceneNodeObject, ptrToOgreObject->btRigidBodyObject);
					newHead.destination = ninjaPoint;
					newHead.direction = ninjaPoint;
		        ogreHeads.emplace_back(newHead);
		        ptrToOgreObject->headStruct = newHead;
		        ptrToOgreObjects.push_back(ptrToOgreObject);
	
			        ptrToOgreObject->btRigidBodyObject->setUserPointer(ptrToOgreObject);
	
			        ++numOgres;
	
}





//--------------------------------------------------------------------------
bool ProjectApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
	bool ret = BaseApplication::frameRenderingQueued(fe);
	timer += fe.timeSinceLastFrame;
	Ogre::Vector3 tempPoint = Ogre::Vector3(mPlayerEntity->getWorldBoundingBox().getCenter().x,0, mPlayerEntity->getWorldBoundingBox().getCenter().z);
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
	if (timer >= respawnTime && numOgres < maxOgres) {
			                CreateOgre(btVector3(mMainNode->getPosition().x, mMainNode->getPosition().y, mMainNode->getPosition().z)); //should start in top right corner
		
				                timer = 0; //reset timer to zero
		
	}

	for (std::vector<OgreHeadStruct>::iterator iterator = ogreHeads.begin(); iterator != ogreHeads.end(); ++iterator) {
		
		//if (iterator->direction == Ogre::Vector3::ZERO) {
			//attack
			
		//}

		//else
		//{
			Ogre::Real move = ogreMove * fe.timeSinceLastFrame;
			//iterator->distance = iterator->distance - move;
		


			//if (iterator->ogreNode->getPosition().positionCloses(ninjaPoint, 1))
		//	{
		//		iterator->ogreNode->setPosition(iterator->destination); //want to set it on top of destination
		//		iterator->direction = Ogre::Vector3::ZERO;
//
		//	}
			iterator->destination = ninjaPoint;
			iterator->direction = iterator->destination - iterator->ogreNode->getPosition();
			iterator->distance = iterator->direction.normalise();


	//	}
		iterator->ogreNode->lookAt(ninjaPoint, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Z);
		iterator->ogreNode->translate(ogreMove * iterator->direction);
					 //adjust the ogre head's rigidbody every frame as well
					Ogre::SceneNode* locationNode = iterator->ogreNode;
					 btTransform rigidTrans;
					iterator->ogreBody->getMotionState()->getWorldTransform(rigidTrans);
					 //reset the rigidbody's place every frame
					  btVector3 rigidLoc = btVector3(locationNode->getPosition().x, 50, locationNode->getPosition().z);
					 rigidTrans.setOrigin(rigidLoc);
			 iterator->ogreBody->setWorldTransform(rigidTrans);
		
	}


	

	
			        //erase spheres that have been here for 4 seconds
	for (int i = 0; i < ptrToOgreObjects.size(); i++) {
		if (ptrToOgreObjects[i]->objectType == "Sphere") {
			ptrToOgreObjects[i]->timer = ptrToOgreObjects[i]->timer + fe.timeSinceLastFrame;
			if (ptrToOgreObjects[i]->timer >= 4)
				eraseObject(ptrToOgreObjects[i]);

		}
	}
	
	
			        //update the healthWindow if the Ninja's health changes
			        healthWindow->setText("Health: " + std::to_string(ninjaHealth));
	return ret;
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


ProjectApplication::ogreObject* ProjectApplication::getOgreObject(const btCollisionObject * obj) {
		        for (int i = 0; i < ptrToOgreObjects.size(); i++) {
			                if (ptrToOgreObjects[i]->btCollisionObjectObject == obj)
				                {
				                        return ptrToOgreObjects[i];
				                }
		
	}
		        ogreObject* ret = new ogreObject;
		        return ret;
	
}

	//delete an object fully
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

	// This method deletes the specific ogreObject that is passed in as a pointer by iterating through the
	// vector of ptrToOgreObjects until it finds that match
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
				 * itr = NULL;
				                                // and then delete that pointer out of the vector
					                                ptrToOgreObjects.erase(itr);
				                                // no need to continue iterating and the iterator just became invalid
					                                break;
			
		}
		
	}
		}

	void ProjectApplication::checkCollisions() {
		        int TotalManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	
			        for (int i = 0; i < TotalManifolds; i++) {
			                btPersistentManifold* Manifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
			                const btCollisionObject* object1 = Manifold->getBody0();
			                const btCollisionObject* object2 = Manifold->getBody1();
		
				                ogreObject *ogOb1 = new ogreObject;
			                ogOb1 = ((ogreObject*)object1->getUserPointer());
			                ogreObject *ogOb2 = new ogreObject;
			                ogOb2 = ((ogreObject*)object2->getUserPointer());
		
				                std::string type1 = ogOb1->objectType;
			                std::string type2 = ogOb2->objectType;
		
				                //checks if sphere hits cube then set both to delete
				                if ((type1 == "Ninja") && (type2 == "Sphere")) {
				                        ogOb2->canDelete = true;
				                        ninjaHealth--;
			
		}
			                else if ((type1 == "Ogre") && (type2 == "Sphere")) {
				                        //delete the sphere
					                        ogOb2->canDelete = true;
				                        //update score on a hit
					                      //  score = score + 2;
				                        //update the ogre's health
					                        ogOb1->health = ogOb1->health - 1;
				                        //check if the ogreHead is at 0 health now and if so delete it
					                        if (ogOb1->health <= 0) {
					                                ogOb1->canDelete = true;
					                               // score = score + 5;
					                                numOgres--;
				
			}
			
		}
			                else if ((type1 == "Sphere") && (type2 == "Sphere")) {
				                        ogOb1->canDelete = true;
				                        ogOb2->canDelete = true;
			
		}
		
	}
	
}

	//objects marked with canDelete = true are deleted here
	void ProjectApplication::checkDeletions() {
		        for (int i = 0; i < ptrToOgreObjects.size(); i++) {
			                if (ptrToOgreObjects[i]->canDelete == true)
				                        eraseObject(ptrToOgreObjects[i]);
		
	}
	
}
	bool ProjectApplication::frameStarted(const Ogre::FrameEvent &evt)
			{
			        //the capture is on for CEGUI and also key press functions
				        mKeyboard->capture();
			        mMouse->capture();
			        dynamicsWorld->stepSimulation(evt.timeSinceLastFrame);
		
				        //run collision detection functions every frame
				        checkCollisions();
			        checkDeletions();
		
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