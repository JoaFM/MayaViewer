#include "LiteratimSceneManager.h"


#include <maya/MItDependencyNodes.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MObjectHandle.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MMatrix.h>


#include <string>
#include <set>
#include <vector>
#include "LiteratimNetworking.h"


LiteratimSceneManager::LiteratimSceneManager()
{
}


LiteratimSceneManager::~LiteratimSceneManager()
{
}

void LiteratimSceneManager::SetConnection(LiteratimNetworking* LitNetwork)
{
	m_LitNetwork = LitNetwork;
	m_Worker.SetConnection(m_LitNetwork);
}


void LiteratimSceneManager::DirtyAll()
{
	for (std::pair<std::string, LiteratimMesh*> LitMesh : m_SceneObjects)
	{
		LitMesh.second->Dirty();

	}
	m_SceneObjects.clear();
}

void LiteratimSceneManager::Tick()
{
	UpdateSceneDescription();
	SyncTransforms();
	TickMeshQuery();
	TickCamera();
}


void LiteratimSceneManager::TickMeshQuery()
{
	if (m_ActiveQuryHash != "")
	{
		if (m_Worker.IsReadForNextMesh())
		{
			ProgressToNextMesh();
		}


		/*
		LiteratimMesh* LitMesh = m_SceneObjects[m_ActiveQuryHash];
		if (!LitMesh->IsQuryDone())
		{
			LitMesh->RunQuery(m_LitNetwork);
		}
		else
		{
			ProgressToNextMesh();
		}
		*/
	}
}

void LiteratimSceneManager::ProgressToNextMesh()
{

	m_CurrUpdatemeshIterator++;
	if (m_CurrUpdatemeshIterator == m_SceneObjects.end())
	{
		if (m_SceneObjects.size())
		{
			m_ActiveQuryHash = (m_SceneObjects.begin()->first);
			m_CurrUpdatemeshIterator = m_SceneObjects.begin();
			m_CurrUpdatemeshIterator->second->StartQuery();
		}
		else
		{
			m_ActiveQuryHash = "";
		}
	}
	else
	{
		m_ActiveQuryHash = (m_CurrUpdatemeshIterator->first);
		m_CurrUpdatemeshIterator->second->StartQuery();
	}
	LiteratimMesh* LitMesh = m_SceneObjects[m_ActiveQuryHash];
	m_Worker.QueryThisMesh(LitMesh);
}

void LiteratimSceneManager::RemoveSceneObject(std::string ItemToRemove)
{
	auto it = m_SceneObjects.find(ItemToRemove);

	it->second->SendDeleteMe(m_LitNetwork);
	it->second = nullptr;
	m_SceneObjects.erase(it);
}

void LiteratimSceneManager::TickCamera()
{
	MString nodeName("persp");
	MObject nodeObj;
	MSelectionList sList;
	if (MGlobal::getSelectionListByName(nodeName, sList))
	{
		sList.getDependNode(0, nodeObj);
	}
	if (!nodeObj.isNull())
	{
		
		MFnDagNode dagPath(nodeObj);
		MDagPath thisDagNode;
		dagPath.getPath(thisDagNode);
		MMatrix worldPos = thisDagNode.inclusiveMatrix();

		//TODO send this matric to ue4
		std::vector<float> CameraMatrix;
		float NewcameraHash = 0;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				CameraMatrix.push_back((float)worldPos(row, col));
				NewcameraHash += (float)worldPos(row, col);
			}
		}
		if (m_cameraHash != NewcameraHash)
		{
			std::string sendCommand = "";
			sendCommand += "{ \"Command\":\"SetCamera\",\"WorldMatrix\": [";
			for (int i = 0; i < 16; i++)
			{
				sendCommand += std::to_string(CameraMatrix[i]);
				if (i!=15) sendCommand += ",";
			}
			sendCommand += "]}";

			m_Worker.SendDeferredCommand(sendCommand);
// 			if (m_LitNetwork->LitSendMessage(sendCommand, ResponceHeaders::Command))
// 			{
 				m_cameraHash = NewcameraHash;
// 			}

		}
	}
}

void LiteratimSceneManager::SyncTransforms()
{
	for (const std::pair<std::string, LiteratimMesh*>& SceneMesh : m_SceneObjects)
	{
		if (SceneMesh.second->QueryTransform(m_LitNetwork))
		{
			return;
		}
	}
}

void LiteratimSceneManager::UpdateSceneDescription()
{
	
	MItDependencyNodes it(MFn::Type::kMesh);

	std::set<std::string> ActiveObjects;

	// Add new objects
	while (!it.isDone())
	{
		// get the object the iterator is referencing
		MObject obj = it.item();
		// Make sure it the prim shape not an intermediate
		MStatus rs;
		MDagPath dp = MDagPath::getAPathTo(obj, &rs);
		MDagPath tdp = MDagPath::getAPathTo(dp.transform(), &rs);
		tdp.extendToShape();
		obj = tdp.node();

		std::string HashCode = LiteratimMesh::GetHashFromMObject(obj);
		MObjectHandle OH(obj);
		ActiveObjects.insert(HashCode);
		if (m_SceneObjects.count(HashCode))
		{
			// its all ready present
		}
		else
		{
			AddSceneMesh(OH);
		}
		// move to next item
		it.next();
	}

	// remove deleted objects
	{
		// get all keys
		std::vector<std::string> vints;
		vints.reserve(m_SceneObjects.size());
		for (auto const& imap : m_SceneObjects)
			vints.push_back(imap.first);
		//cross compare keys
		for (std::string itemHash : vints)
		{
			if (!ActiveObjects.count(itemHash) || (!m_SceneObjects[itemHash]->IsValid()))
			{
				RemoveSceneMesh(itemHash);
			}
		}
	}
}

bool LiteratimSceneManager::AddSceneMesh(MObjectHandle ObjectHandle)
{
	if (ObjectHandle.object().apiType() != MFn::Type::kMesh) {
		MGlobal::displayInfo(MString("Error Tried to add an object that is not a mesh!!!!"));
		return false;
	}

	LiteratimMesh*  LMesh = new LiteratimMesh(ObjectHandle);
	m_SceneObjects.insert_or_assign(LMesh->GetHash(), LMesh);


	LMesh->SendCreateMe(m_LitNetwork);

	m_CurrUpdatemeshIterator = m_SceneObjects.begin();
	m_ActiveQuryHash = m_CurrUpdatemeshIterator->first;
	m_CurrUpdatemeshIterator->second->StartQuery();

	//#TODO: Send make new object
	return true;
}

void LiteratimSceneManager::RemoveSceneMesh(std::string itemToRemove)
{
	auto& it = m_SceneObjects.find(itemToRemove);

	if (it != m_SceneObjects.end())
	{
		RemoveSceneObject(it->first);
		if (m_ActiveQuryHash == itemToRemove)
		{
			// make sure we are not qurying a deleted object
			if (m_SceneObjects.size())
			{
				m_ActiveQuryHash = (m_SceneObjects.begin()->first);
				m_CurrUpdatemeshIterator = m_SceneObjects.begin();

			}
			else
			{
				m_ActiveQuryHash = "";
			}
		}
	}
	MGlobal::displayInfo(MString("removed mesh"));

	//#TODO: Send remove object

}
