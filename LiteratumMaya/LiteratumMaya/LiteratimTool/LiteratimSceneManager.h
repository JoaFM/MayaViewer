#pragma once
#include "LiteratimMesh.h"
#include "LiteratimWorker.h"

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

	void DirtyAll();
public:
	std::map<std::string, LiteratimMesh*> m_SceneObjects;

private:
	LiteratimNetworking* m_LitNetwork;
	LiteratimWorker m_Worker;
	//Update Loop
	/* Iterator for interegating meshes*/
	std::map<std::string, LiteratimMesh*>::iterator m_CurrUpdatemeshIterator;
	std::string m_ActiveQuryHash = "";
	float m_cameraHash = 0;

private:
	void TickMeshQuery();
	void ProgressToNextMesh();
	void RemoveSceneObject(std::string ItemToRemove);
	void TickCamera();
	void SyncTransforms();

	void UpdateSceneDescription();
	bool AddSceneMesh(MObjectHandle ObjectHandle);
	void RemoveSceneMesh(std::string itemToRemove);
};

