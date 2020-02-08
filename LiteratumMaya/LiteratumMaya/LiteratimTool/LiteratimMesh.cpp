#include "LiteratimMesh.h"

#include "maya/MFnMesh.h"
#include "maya/MFnMesh.h"
#include "maya/MDagPath.h"
#include "maya/MPlug.h"
#include "maya/MUuid.h"
#include "maya/MMatrix.h"
#include "maya/MFnTransform.h"
#include "maya/MTransformationMatrix.h"
#include <maya/MGlobal.h>
#include <maya/MFnMatrixData.h>

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
	if (!m_ObjHandle.isValid()) return;
	ClearData();
	IsBeingQueried = true; 
	
	
 	MFnMesh msh(m_ObjHandle.object());

	

	// Copy Tri Data
	msh.getTriangles(m_FaceTriangles, m_TriangleInexies);
	// copy verts
	msh.getPoints(m_vertLocations);


	MIntArray FaceId;
	FaceId.clear();
	for (unsigned int i = 0; i < m_FaceTriangles.length(); i++)
	{
		MFloatVectorArray Fnormals;
		msh.getFaceVertexNormals(i, Fnormals);
		msh.getPolygonVertices(i, FaceId);
		int FaceArrayIndex = (int)m_faceInfo.size();
		m_faceInfo.push_back(std::map<unsigned int, MFloatVector>());
		for (unsigned int fi = 0; fi < Fnormals.length(); fi++)
		{
			m_faceInfo[FaceArrayIndex].insert(std::pair<unsigned int, MFloatVector>(FaceId[fi], Fnormals[fi]));
		}
	}


	//Update Buckets
	CheckBucketSizes();
	GetShaderInfo();
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
	m_indexList = 0;
}

void LiteratimMesh::RunQuery(LiteratimNetworking* LitNetWork)
{
	// #TODO: Run the vert and tri things here
	if (!LitNetWork->IsConnected())
	{
		MGlobal::displayInfo("Server Not Connected: Not checking mesh");
	}

	QueryTriangles();
	TickQuery(LitNetWork);
	if (m_MaterialsDirty) SendMaterialUpdate(LitNetWork);

	CheckForIsClean(LitNetWork);
}

bool LiteratimMesh::QueryTransform(LiteratimNetworking* LitNetWork)
{
	MFnDagNode dagPath(m_ObjHandle.object());
	MDagPath thisDagNode;
	dagPath.getPath(thisDagNode);
	MMatrix WorldMatrix = thisDagNode.inclusiveMatrix();

	
	std::vector<float> UnpackedMatrix;
	UnpackedMatrix.reserve(16);
	float NewTransformHash = 0;
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{	
			UnpackedMatrix.push_back((float)WorldMatrix(row, col));
			NewTransformHash += (float)WorldMatrix(row, col);
		}
	}


	if (m_TransformHash != NewTransformHash)
	{
		json::JSON obj;

		obj["Command"] = "SetObjectTransform";
		obj["ObjectName"] = GetHash();
		obj["WorldMatrix"] = json::Array();
		
		for (int i = 0; i < 16; i++)
		{
			obj["WorldMatrix"].append(UnpackedMatrix[i]);
		}

		std::string MessageJsonString = obj.dump(0, "");
		if (LitNetWork->LitSendMessage(MessageJsonString, ResponceHeaders::Command))
		{
			m_TransformHash = NewTransformHash;
		}
		return true;
	}
	else
	{
		return false;
	}
}

std::string LiteratimMesh::GetHashFromMObject(const  MObject& Obj)
{
	MFnDependencyNode DG(Obj);
	int len = 0;
	const char* rv = DG.uuid().asString().asChar(len);

	std::string ts(rv);

	return ts;
}

void LiteratimMesh::Dirty()
{
	DirtyAllBuckets();
	m_MaterialsDirty = true;
	m_TransformHash = -1;
}

std::string LiteratimMesh::GetHash() 
{
	if (m_Hash == "")
	{
		MFnDependencyNode DG(m_ObjHandle.object());
		int len = 0;
		const char* rv = DG.uuid().asString().asChar(len);
		m_Hash = std::string(rv);
	}

	return m_Hash;
}

void LiteratimMesh::SendCreateMe(LiteratimNetworking* LitNetWork)
{
	json::JSON obj;
	obj["Command"] = "CreateMesh";
	obj["ObjectName"] = GetHash();
	std::string MessageJsonString = obj.dump(0, "");
	LitNetWork->LitSendMessage(MessageJsonString, ResponceHeaders::Command);
}

void LiteratimMesh::SendDeleteMe(LiteratimNetworking* LitNetWork)
{
	json::JSON obj;
	obj["Command"] = "DeleteMesh";
	obj["ObjectName"] = GetHash();
	std::string MessageJsonString = obj.dump(0, "");
	LitNetWork->LitSendMessage(MessageJsonString, ResponceHeaders::Command);
}

void LiteratimMesh::GetShaderInfo()
{
	if (!m_ObjHandle.isValid()) return;

	MFnMesh msh(m_ObjHandle.object());


	MObjectArray ShaderArray;
	msh.getConnectedShaders(0, ShaderArray, m_TriShaderIndex);
	
	// relist the materials
	// #TODO: On change message render to remap materials
	
	std::vector<MString> NewMaterials;

	// List materials applied
	{
		NewMaterials.clear();
		for (unsigned int i = 0; i < ShaderArray.length(); i++)
		{
			MFnDependencyNode shaderGroup(ShaderArray[i]);
			MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
			MPlugArray connectedPlugs;
			shaderPlug.connectedTo(connectedPlugs, true, false);
			MFnDependencyNode fnDN(connectedPlugs[0].node());
			NewMaterials.push_back(MString(fnDN.name()));
		}
		if (NewMaterials.size() == 0)
		{
			NewMaterials.push_back("None");
		}
	}

	// Check if materials have changed
	{
		if (NewMaterials.size() != m_Materials.size())
		{
			m_Materials = NewMaterials;
			m_MaterialsDirty = true;
		}
		{
			for (int i = 0; i < m_Materials.size(); i++)
			{
				if (m_Materials[i] != NewMaterials[i])
				{
					m_MaterialsDirty = true;
					m_Materials = NewMaterials;
					return;
				}
			}
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
	SendBucket sendBucket;
	std::vector<std::vector<int>> materialJumps;
	sendBucket.MaterialCount = (int)m_Materials.size();
	for (auto& mat : m_Materials)
	{
		materialJumps.push_back(std::vector<int>());
	}

	{
		unsigned int FaceCounter = m_CurrFaceIndex;
		for (; FaceCounter < std::min(m_CurrFaceIndex + m_PerBucketFaceCount, m_FaceTriangles.length()); FaceCounter++)
		{
			std::vector<int>& CurTriList = materialJumps[m_TriShaderIndex[FaceCounter]];

			for ( int faceTriIndex = 0; faceTriIndex < m_FaceTriangles[FaceCounter]; faceTriIndex++)
			{
				unsigned int TriIndex = m_TriangleInexies[m_CurrTriIndex++];

				MFloatVector& Normal = m_faceInfo[FaceCounter][TriIndex];
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(Normal.x);
				sendBucket.VertNormals.push_back(Normal.y);
				sendBucket.VertNormals.push_back(Normal.z);
				CurTriList.push_back(m_indexList++);

				TriIndex = m_TriangleInexies[m_CurrTriIndex++];
				Normal = m_faceInfo[FaceCounter][TriIndex];
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(Normal.x);
				sendBucket.VertNormals.push_back(Normal.y);
				sendBucket.VertNormals.push_back(Normal.z);
				CurTriList.push_back(m_indexList++);

				TriIndex = m_TriangleInexies[m_CurrTriIndex++];
				Normal = m_faceInfo[FaceCounter][TriIndex];
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].x);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].y);
				sendBucket.VertPositions.push_back(m_vertLocations[TriIndex].z);
				sendBucket.VertNormals.push_back(Normal.x);
				sendBucket.VertNormals.push_back(Normal.y);
				sendBucket.VertNormals.push_back(Normal.z);
				CurTriList.push_back(m_indexList++);
			}
		}
		m_CurrFaceIndex += FaceCounter - m_CurrFaceIndex;
	}
		
	for (std::vector<int>& TriList : materialJumps)
	{
		sendBucket.IndexList.push_back((int)TriList.size());
		sendBucket.IndexList.insert(sendBucket.IndexList.end(), TriList.begin(), TriList.end());
	}
	sendBucket.GenerateHash();

	if (m_MeshBuckets[m_CurrentBucketIndex].hash != sendBucket.hashNum)
	{
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
	unsigned int hashnum = 1;

	for (unsigned int i = 0; i < VertPositions.size(); i++)
	{
		hashnum += (unsigned int)((VertPositions[i] * 100) * (i * 1285 ));
		hashnum += (unsigned int)((VertNormals[i] * 1000) * (i * 123));
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
	obj["ObjectName"] = GetHash();
	obj["BucketIndex"] = BucketIndex;
	obj["VertPositionsXYZ"] = json::Array();
	obj["VertNormalsXYZ"] = json::Array();
	obj["TriIndices"] = json::Array();
	obj["MaterialCount"] = sendBucket.MaterialCount;
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
	//MGlobal::displayInfo("Send mesh update: " + feedback + "kbps");
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

	ClearData();
	// cleanup
	IsBeingQueried = false;

	if (m_MeshWasEdited)
	{
		std::string SCommand = std::string("{\"Command\" : \"MeshDone\",\"ObjectName\":\"") + GetHash() + std::string("\"}");
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

void LiteratimMesh::ClearData()
{
	m_FaceTriangles.clear();
	m_faceInfo.clear();
	m_TriangleInexies.clear();
	m_vertLocations.clear();
}

void LiteratimMesh::SendMaterialUpdate(LiteratimNetworking* LitNetWork)
{
	 
	std::string SCommand = std::string("{\"Command\" : \"SetMaterialNames\", \"ObjectName\":\"")
		+ GetHash()
		+ std::string("\",\"MaterialNames\":[");
		
		
	
	for (int i = 0; i < m_Materials.size(); i++)
	{
		SCommand += std::string("\"") + m_Materials[i].asUTF8() + std::string("\"");
		if (i != m_Materials.size() - 1) 
		{
			SCommand +=std::string(",");
		}
	}
	SCommand += std::string("]}");
	m_MaterialsDirty = !LitNetWork->LitSendMessage(SCommand, ResponceHeaders::Command);
}
