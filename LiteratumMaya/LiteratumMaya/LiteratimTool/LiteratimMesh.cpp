#include "LiteratimMesh.h"

#include "maya/MFnMesh.h"
#include "maya/MPlug.h"

#include <maya/MGlobal.h>

#include <iostream>   // std::cout
#include <string>   
#include <algorithm>    // std::min

//Json
#include "External/json.hpp"
#include "LiteratimNetworking.h"

LiteratimMesh::LiteratimMesh()
{
}


LiteratimMesh::LiteratimMesh(MObjectHandle objHandle)
{
	m_ObjHandle = objHandle;
}

LiteratimMesh::~LiteratimMesh()
{
}

bool LiteratimMesh::IsValid() const
{
	return m_ObjHandle.isAlive() && m_ObjHandle.isValid();
}


void LiteratimMesh::StartQuery()
{
	MGlobal::displayInfo("-------StartQuery-------");

	if (!m_ObjHandle.isValid()) return;

	IsBeingQueried = true; 
	
	MFnMesh msh(m_ObjHandle.object());

	// Copy Tri Data
	msh.getTriangles(m_FaceTriangles, m_TriangleInexies);
	// copy verts
	msh.getPoints(m_vertLocations);
	//copy normals
	msh.getNormals(m_normals, MSpace::kObject);

	 these normals are shared so useless less make a new plan
	for (unsigned int i = 0; i < m_FaceTriangles.length(); i++)
	{
		MIntArray ThisFaceNormals;
		msh.getFaceNormalIds( i, ThisFaceNormals );
		for (unsigned int newI = 0; newI < ThisFaceNormals.length(); newI++)
		{
			m_FaceNormals.append(ThisFaceNormals[newI]);
		}
	}
	//Update Buckets
	CheckBucketSizes();
	CheckShaders();
	DirtyAllBuckets();

	// start mesh qury
// 	MGlobal::displayInfo(std::to_string(m_FaceTriangles.length()).c_str());
// 	MGlobal::displayInfo(std::to_string(m_FaceNormals.length()).c_str());
// 	MGlobal::displayInfo(std::to_string(m_TriangleInexies.length()).c_str());
// 	MGlobal::displayInfo(std::to_string(m_vertLocations.length()).c_str());
// 	MGlobal::displayInfo(std::to_string(m_normals.length()).c_str());

	m_CurrTriIndex = 0;
	m_CurrentBucketIndex = 0;
	m_CurrFaceIndex = 0;
}

void LiteratimMesh::RunQuery(LiteratimNetworking* LitNetWork)
{
	// #TODO: Run the vert and tri things here
	
	QueryTriangles();
	TickQuery(LitNetWork);
	//#TODO: if the mesh is done, state it is so and clean up copied data

	CheckForIsClean(LitNetWork);
}

std::string LiteratimMesh::GetObjectHashAsString() const
{
	return std::to_string(m_ObjHandle.hashCode());
}

void LiteratimMesh::CheckShaders()
{
	if (!m_ObjHandle.isValid()) return;

	MFnMesh msh(m_ObjHandle.object());

	MObjectArray ShaderArray;
	MIntArray intArrayShaderIndex;
	msh.getConnectedShaders(0, ShaderArray, intArrayShaderIndex);
	
	// relist the materials
	// #TODO: On change message render to remap materials
	{
		m_Materials.clear();
		for (unsigned int i = 0; i < ShaderArray.length(); i++)
		{
			MFnDependencyNode shaderGroup(ShaderArray[i]);
			MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
			MPlugArray connectedPlugs;
			shaderPlug.connectedTo(connectedPlugs, true, false);
			MFnDependencyNode fnDN(connectedPlugs[0].node());
			m_Materials.push_back(MString(fnDN.name()));
		}
		if (m_Materials.size() == 0)
		{
			m_Materials.push_back("None");
		}
	}
}


void LiteratimMesh::CheckBucketSizes()
{
	// Tris
	{
		int num_expexted_buckets = 1 + int(ceil(m_FaceTriangles.length() / m_PerBucketFaceCount));
		if (m_MeshBuckets.size() != num_expexted_buckets)
		{
			m_MeshBuckets.clear();
			for (int i = 0; i < num_expexted_buckets; i++)
			{
				m_MeshBuckets.push_back(MeshBucketInfo());
			}
		}
	}
 }

void LiteratimMesh::QueryTriangles()
{

}


void LiteratimMesh::TickQuery(LiteratimNetworking* LitNetWork)
{
	MGlobal::displayInfo("Start QT");

	SendBucket sendBucket;
	{
		unsigned int indexList = 0;

		unsigned int FaceCounter = m_CurrFaceIndex;
		for (; FaceCounter < std::min(m_CurrFaceIndex + m_PerBucketFaceCount, m_FaceTriangles.length()); FaceCounter++)
		{
			//MGlobal::displayInfo("Process a face");
			//MGlobal::displayInfo(std::to_string(FaceCounter).c_str());

			for ( int faceTriIndex = 0; faceTriIndex < m_FaceTriangles[FaceCounter]; faceTriIndex++)
			{
				unsigned int TriIndex = m_TriangleInexies[m_CurrTriIndex++];
				//MGlobal::displayInfo(std::to_string(TriIndex).c_str());

				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].x);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].y);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].z);
				sendBucket.IndexList.push_back(indexList++);
				//MGlobal::displayInfo(std::to_string(m_CurrTriIndex).c_str());


				TriIndex = m_TriangleInexies[m_CurrTriIndex++];
				//MGlobal::displayInfo(std::to_string(TriIndex).c_str());

				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].x);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].y);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].z);
				sendBucket.IndexList.push_back(indexList++);
				//MGlobal::displayInfo(std::to_string(m_CurrTriIndex).c_str());


				TriIndex = m_TriangleInexies[m_CurrTriIndex++];
				//MGlobal::displayInfo(std::to_string(TriIndex).c_str());

				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].x);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].y);
				sendBucket.VertNormals.push_back(m_normals[TriIndex].z);
				sendBucket.IndexList.push_back(indexList++);
				//MGlobal::displayInfo(std::to_string(m_CurrTriIndex).c_str());
			}
		}
		m_CurrFaceIndex += FaceCounter - m_CurrFaceIndex;
	}
	sendBucket.GenerateHash();
	if (m_MeshBuckets[m_CurrentBucketIndex].hash != sendBucket.hashNum)
	{
		MGlobal::displayInfo("----------------");

		SendMeshUpdate(sendBucket, m_CurrentBucketIndex, LitNetWork);
		m_MeshBuckets[m_CurrentBucketIndex].hash = sendBucket.hashNum;
		m_MeshBuckets[m_CurrentBucketIndex].isDirty = false;
		m_MeshWasEdited = true;
	}
	else
	{
		m_MeshBuckets[m_CurrentBucketIndex].isDirty = false;
	}
	m_CurrentBucketIndex++;
}


void SendBucket::GenerateHash()
{
	float hashnum = 1;

	for (unsigned int i = 0; i < VertPositions.size(); i++)
	{
		hashnum += VertPositions[i] * (i * 1.12352456f);
		hashnum += VertNormals[i] * (i * 1.12352456f);
	}

	unsigned int ihash = 1;
	for (unsigned int i = 0; i < IndexList.size(); i++)
	{
		ihash += IndexList[i] * (ihash * 197258);
	}
	hashNum = hashnum + (float)ihash;
}


void LiteratimMesh::SendMeshUpdate(SendBucket& sendBucket, int BucketIndex, LiteratimNetworking* LitNetWork)
{

	json::JSON obj;

	
	obj["Command"] = "SetMeshBucket";
	obj["ObjectName"] = std::to_string(this->m_ObjHandle.hashCode());
	obj["BucketIndex"] = BucketIndex;
	obj["VertPositionsXYZ"] = json::Array();
	obj["VertNormalsXYZ"] = json::Array();
	obj["TriIndices"] = json::Array();
	obj["NumbBuckets"] = (int)m_MeshBuckets.size();



	for (int i = 0; i < sendBucket.VertPositions.size(); i++)
	{
		obj["VertPositionsXYZ"].append(sendBucket.VertPositions[i]);
		obj["VertNormalsXYZ"].append(sendBucket.VertNormals[i]);
	}

	for (int i = 0; i < sendBucket.IndexList.size(); i++)
	{
		obj["TriIndices"].append(sendBucket.IndexList[i]);
	}
	std::string MessageJsonString = obj.dump(0, "");

	float dataSent = (float)MessageJsonString.size() / 1024.0f;
	MString feedback(std::to_string(dataSent).c_str());
	MGlobal::displayInfo("Send mesh update: " + feedback + "kbps");

	LitNetWork->LitSendMessage(MessageJsonString, ResponceHeaders::Command);
}

void LiteratimMesh::CheckForIsClean(LiteratimNetworking* LitNetWork)
{
	bool isDone = true;
	for (auto& hs : m_MeshBuckets)
	{
		if (hs.isDirty)
		{
			return;
		}
	}
	// cleanup
	IsBeingQueried = false;
	m_FaceTriangles.clear();
	m_FaceNormals.clear();
	m_TriangleInexies.clear();
	m_vertLocations.clear();
	m_normals.clear();
	if (m_MeshWasEdited)
	{
		std::string SCommand = std::string("{\"Command\" : \"MeshDone\",\"ObjectName\":\"") + GetObjectHashAsString() + std::string("\"}");
		LitNetWork->LitSendMessage(SCommand, ResponceHeaders::Command);
	}
	m_MeshWasEdited = false;
}

void LiteratimMesh::DirtyAllBuckets()
{
	bool isDone = true;
	for (auto& hs : m_MeshBuckets)
	{
		hs.isDirty = true;
	}
}
