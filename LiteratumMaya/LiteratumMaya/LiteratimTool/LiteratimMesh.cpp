#include "LiteratimMesh.h"



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


void LiteratimMesh::RunQuery(LiteratimNetworking* LitNetWork)
{
	// TODO: Run the vert and tri things here
	
	QueryShaders();
	QueryTriangles();
	QueryVerts();

	// this should only be set if the object is clean
	// just here until this system is actalay checking the mesh
	IsBeingQueried = false;

}

void LiteratimMesh::QueryShaders()
{
}

void LiteratimMesh::QueryTriangles()
{
}

void LiteratimMesh::QueryVerts()
{
}
