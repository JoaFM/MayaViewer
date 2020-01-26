#include "LiteratimSceneManager.h"


#include <maya/MItDependencyNodes.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MObjectHandle.h>


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
}


void LiteratimSceneManager::Tick()
{
	UpdateSceneDescription();

	TickMeshQuery();
}


void LiteratimSceneManager::TickMeshQuery()
{
	if (m_ActiveQuryHash)
	{
		LiteratimMesh* LitMesh = m_SceneObjects[m_ActiveQuryHash];
		if (!LitMesh->IsQuryDone())
		{
			LitMesh->RunQuery(m_LitNetwork);
		}
		else
		{
			ProgressToNextMesh();
		}
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
			m_ActiveQuryHash = 0;
		}
	}
	else
	{
		m_ActiveQuryHash = (m_CurrUpdatemeshIterator->first);
		m_CurrUpdatemeshIterator->second->StartQuery();
	}
}

void LiteratimSceneManager::RemoveSceneObject(unsigned int ItemToRemove)
{
	auto it = m_SceneObjects.find(ItemToRemove);

	it->second = nullptr;
	m_SceneObjects.erase(it);
}

void LiteratimSceneManager::UpdateSceneDescription()
{
	
	MItDependencyNodes it(MFn::Type::kMesh);

	std::set<unsigned int> ActiveObjects;

	// Add new objects
	while (!it.isDone())
	{
		// get the object the iterator is referencing
		MObject obj = it.item();
		MFnMesh ms(obj);
		//MString path = ms.fullPathName();
		MObjectHandle OH(obj);
		unsigned int HashCode = OH.hashCode();
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
		std::vector<unsigned int> vints;
		vints.reserve(m_SceneObjects.size());
		for (auto const& imap : m_SceneObjects)
			vints.push_back(imap.first);
		//cross compare keys
		for (unsigned int itemHash : vints)
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
	m_SceneObjects.insert_or_assign(ObjectHandle.hashCode(), LMesh);

	MGlobal::displayInfo(MString("Added mesh") );

	m_CurrUpdatemeshIterator = m_SceneObjects.begin();
	m_ActiveQuryHash = m_CurrUpdatemeshIterator->first;
	m_CurrUpdatemeshIterator->second->StartQuery();
	//#TODO: Send make new object
	return true;
}

void LiteratimSceneManager::RemoveSceneMesh(unsigned int itemToRemove)
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
				m_ActiveQuryHash = 0;
			}
		}
	}
	MGlobal::displayInfo(MString("removed mesh"));

	//#TODO: Send remove object

}
