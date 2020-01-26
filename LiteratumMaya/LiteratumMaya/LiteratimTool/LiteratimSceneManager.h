#pragma once
#include "LiteratimMesh.h"

#include <iostream>
#include <map>

class LiteratimNetworking;


class LiteratimSceneManager
{
public:
	LiteratimSceneManager();
	~LiteratimSceneManager();

	void Tick();
	void SetConnection(LiteratimNetworking* LitNetwork);

public:
	std::map<unsigned int, LiteratimMesh*> m_SceneObjects;


private:
	LiteratimNetworking* m_LitNetwork;

	//Update Loop
	/* Iterator for interegating meshes*/
	std::map<unsigned int, LiteratimMesh*>::iterator m_CurrUpdatemeshIterator;
	unsigned int m_ActiveQuryHash = 0;

	void TickMeshQuery();
	void ProgressToNextMesh();
	void RemoveSceneObject(unsigned int ItemToRemove);
private:
	void UpdateSceneDescription();
	bool AddSceneMesh(MObjectHandle ObjectHandle);
	void RemoveSceneMesh(unsigned int itemToRemove);
};

