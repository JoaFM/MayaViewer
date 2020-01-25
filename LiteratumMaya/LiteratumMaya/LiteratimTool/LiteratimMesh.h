#pragma once


#include <maya/MObjectHandle.h>


class LiteratimNetworking;

class LiteratimMesh
{
public:

	LiteratimMesh(MObjectHandle objHandle);
	LiteratimMesh();
	~LiteratimMesh();


	bool IsValid() const;
	void SetIsBeingQueried() { IsBeingQueried = true; };
	bool IsQuryDone() { return !IsBeingQueried; }

	void RunQuery(LiteratimNetworking* LitNetWork);
private:
	MObjectHandle m_ObjHandle;
	bool IsBeingQueried = false;

	void QueryShaders();
	void QueryTriangles();
	void QueryVerts();
};

