#pragma once


#include <maya/MObjectHandle.h>
#include <maya/MFloatVectorArray.h>
#include "maya/MPlugArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MObjectArray.h"
#include "maya/MIntArray.h"

#include <iostream>   // std::cout
#include <vector>
#include <map>


struct MeshBucketInfo {
	MeshBucketInfo()
	{
		isDirty = true;
		hash = 0;
	}
	float hash;
	bool isDirty;
};

struct SendBucket
{
	std::vector<float> VertPositions;
	std::vector<float> VertNormals;
	std::vector<int> IndexList;
	int MaterialCount = -1;
	float hashNum = 0;
	void GenerateHash();
};

class LiteratimNetworking;

class LiteratimMesh
{
public:

	LiteratimMesh(MObjectHandle objHandle);
	LiteratimMesh();
	~LiteratimMesh();


	bool IsValid() const;
	void StartQuery();
	bool IsQuryDone() { return !IsBeingQueried; }

	void RunQuery(LiteratimNetworking* LitNetWork);
	bool QueryTransform(LiteratimNetworking* LitNetWork);
	static std::string GetHashFromMObject(const  MObject& Obj) ;

	std::string GetHash();
private:
	MObjectHandle m_ObjHandle;
	bool IsBeingQueried = false;

	void GetShaderInfo();
	void QueryTriangles();
	
	//transform

	float m_TransformHash = 0;
	//MeshData
	MIntArray m_FaceTriangles;
	MIntArray m_TriangleInexies;
	MFloatPointArray m_vertLocations;
	MIntArray m_TriShaderIndex;
	std::vector<std::map<unsigned int, MFloatVector>> m_faceInfo;

	//Materials
	std::vector<MString> m_Materials;

	//------------------------
	void TickQuery(LiteratimNetworking* LitNetWork);
	unsigned int m_CurrTriIndex = -1;
	unsigned int m_CurrFaceIndex = -1;
	int m_CurrentBucketIndex = -1;
	void CheckBucketSizes();
	const int m_PerBucketFaceCount =1000;
	std::vector<MeshBucketInfo> m_MeshBuckets;
	bool m_MeshWasEdited = false;
	unsigned int m_indexList = 0;


	void SendMeshUpdate(SendBucket& sendBucket, int BucketIndex, LiteratimNetworking* LitNetWork);
	void CheckForIsClean(LiteratimNetworking* LitNetWork);
	void DirtyAllBuckets();
	void ClearData();
	void SendMaterialUpdate(LiteratimNetworking* LitNetWork);
	bool m_MaterialsDirty;
};

