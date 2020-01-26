#pragma once


#include <maya/MObjectHandle.h>
#include <maya/MFloatVectorArray.h>
#include "maya/MPlugArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MObjectArray.h"
#include "maya/MIntArray.h"

#include <iostream>   // std::cout
#include <vector>


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
	std::string GetObjectHashAsString() const;
private:
	MObjectHandle m_ObjHandle;
	bool IsBeingQueried = false;

	void CheckShaders();
	void QueryTriangles();

	//MeshData
	MIntArray m_FaceTriangles;
	MIntArray m_FaceNormals;
	MIntArray m_TriangleInexies;
	MFloatPointArray m_vertLocations;
	MFloatVectorArray m_normals;

	//Materials
	std::vector<MString> m_Materials;

	//------------------------
	void TickQuery(LiteratimNetworking* LitNetWork);
	unsigned int m_CurrTriIndex = -1;
	unsigned int m_CurrFaceIndex = -1;
	int m_CurrentBucketIndex = -1;
	void CheckBucketSizes();
	const int m_PerBucketFaceCount = 1000;
	std::vector<MeshBucketInfo> m_MeshBuckets;
	bool m_MeshWasEdited = false;


	void SendMeshUpdate(SendBucket& sendBucket, int BucketIndex, LiteratimNetworking* LitNetWork);
	void CheckForIsClean(LiteratimNetworking* LitNetWork);
	void DirtyAllBuckets();
};

