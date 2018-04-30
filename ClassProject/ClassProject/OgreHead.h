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

static std::vector<Ogre::Vector3> moveList;

struct OgreHeadStruct {
	Ogre::SceneNode* ogreNode;
	Ogre::Vector3 destination;
	Ogre::Vector3 direction;
	Ogre::Real distance;
	btRigidBody * ogreBody;
	Ogre::Real shootTimer;

	//constructors
	OgreHeadStruct() 
	{
		destination = Ogre::Vector3::ZERO;
		direction = Ogre::Vector3::ZERO;
		distance = 0;
	}

	OgreHeadStruct(Ogre::SceneNode* node, btRigidBody *body) 
	{
		ogreNode = node;
		destination = Ogre::Vector3::ZERO;
		direction = Ogre::Vector3::ZERO;
		distance = 0;
		ogreBody = body;
	}
};
