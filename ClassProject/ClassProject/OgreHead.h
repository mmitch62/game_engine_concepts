#pragma once
#include "BaseApplication.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include "OgreManualObject.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"
#include "BulletCollision\CollisionShapes\btCollisionShape.h"
#include "BaseApplication.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <vector>
//this struct is used to hold info about the ogre heads

struct OgreHeadStruct {
	Ogre::SceneNode* ogreNode;
	Ogre::Entity* ogreEntity;
	Ogre::Vector3 destination;
	Ogre::Vector3 direction;
	Ogre::Real distance;
	btRigidBody * ogreBody;
	bool isActive;

	double damageCooldownTimerOgre;
	//constructors
	OgreHeadStruct() 
	{
		destination = Ogre::Vector3::ZERO;
		direction = Ogre::Vector3::ZERO;
		distance = 0;
		damageCooldownTimerOgre = 0.0f;
		isActive = true;

	}

	OgreHeadStruct(Ogre::SceneNode* node, Ogre::Entity* entity, btRigidBody *body)
	{
		ogreNode = node;
		ogreEntity = entity;
		isActive = true;
		destination = Ogre::Vector3::ZERO;
		direction = Ogre::Vector3::ZERO;
		distance = 0;
		ogreBody = body;
		damageCooldownTimerOgre = 0.0f;
		

	}


};
