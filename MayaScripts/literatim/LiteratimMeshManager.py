import maya.cmds as cmds
import maya.OpenMaya as om
import pymel.core as pm
import math
import json

class MeshManager():

    def getMObject(self,node):
        """
            Return the MObject for a node
        
            Args:
                node: A PyNode or node name
        """
        if isinstance(node, pm.nt.DependNode):
            return node.__apimobject__()
        else:
            sel = om.MSelectionList()
            try:
                sel.add(node)
            except:
                print "ERROR getMObject: # node does not exist or invalid arg"
                return
            mobj = om.MObject()
            sel.getDependNode(0, mobj)
            return mobj 
            


    def __init__(self, _ReferenceObject):
        self._obj_name = _ReferenceObject
        
        self._vertBucket = []
        self._vertBucketSize = 200
        self._vertBucketIndex = 0
        
    def Update(self):
        self.TickVertUpdate()

    def CheckVertBucketSizes(self,numverts):
        num_expexted_buckets = int(math.ceil(numverts / self._vertBucketSize))
        if (len(self._vertBucket) != num_expexted_buckets):
            self._vertBucket = []
            for i in range(num_expexted_buckets):
                self._vertBucket.append({"Vert":[], "Hash":0.0, "Dirty":False})


    def TickVertUpdate(self):
        obj = self.getMObject(self._obj_name)
        mesh = om.MFnMesh ( obj )
        msPointArray = om.MFloatPointArray ( )
        mesh.getPoints (msPointArray )
        
        self.CheckVertBucketSizes(msPointArray.length () )

        self._vertBucketIndex += 1
        if self._vertBucketIndex >= len(self._vertBucket):
            self._vertBucketIndex = 0

        VertexList = []
        hashNum = 0
        for i in range(self._vertBucketIndex * self._vertBucketSize , min(((self._vertBucketIndex * self._vertBucketSize) +self._vertBucketSize  ), msPointArray.length () )):
            VertexList.append({"x":msPointArray[i].x, "y":msPointArray[i].y, "z":msPointArray[i].z})
            hashNum += ((msPointArray[i].x + i) * 1.51214) +  ((msPointArray[i].y + i) * 2.723) +   ((msPointArray[i].z + i) * 251.214) 

        if self._vertBucket[self._vertBucketIndex]["Hash"] != hashNum:
            print "FoundDirty Vert data bucket:", self._vertBucketIndex ,self._vertBucket[self._vertBucketIndex]["Hash"],hashNum
            self._vertBucket[self._vertBucketIndex]["Dirty"] = True 
            self._vertBucket[self._vertBucketIndex]["Vert"] = VertexList
            self._vertBucket[self._vertBucketIndex]["Hash"] = hashNum
            print self._vertBucket[self._vertBucketIndex]["Hash"]

    def GetDirtyVertBucket(self):
        for i in range(len(self._vertBucket)):
            if self._vertBucket[i]["Dirty"]:
                return i
        return None

    def GetNextDirtyVertBucketCommand(self, cleanBucketOnGet = True):
        data = {}
        data["objectName"] = self._obj_name
        data["Command"] = "SetMeshBucketVerts" 

        AnythingDirty = False
        i = self.GetDirtyVertBucket()    
        if (i != None):
            if cleanBucketOnGet:
                self._vertBucket[i]["Dirty"] = False
            data["VertexPositions"] =  self._vertBucket[i]["Vert"]
            data["Hash"] =  self._vertBucket[i]["Hash"]
            data["BucketIndex"] = i
            AnythingDirty = True
       
        if AnythingDirty:
            return json.dumps(data) 
        else:
            return None   

    def GetObjectWholeData(self, obj_name):
        data = {}
        data["objectName"] = obj_name
        data["Command"] = "SetObjectWholeData"

        obj = MayaTools.getMObject(obj_name)
        mesh = om.MFnMesh ( obj )
        floatArray = om.MFloatPointArray ( )
        #------------------------------------------
        mesh.getPoints (floatArray )
        VertexList = []
        for i in range(floatArray.length () ):
            VertexList.append({"x":floatArray[i].x, "y":floatArray[i].y, "z":floatArray[i].z})
        data["VertexPositions"] = VertexList
        #------------------------------------------


        #-----------------Shaders names-----------------------
        ShaderArray = om.MObjectArray (  )
        intArrayShaderIndex = om.MIntArray ( )
        mesh.getConnectedShaders(0,ShaderArray,intArrayShaderIndex)
        data["materials"] = []
        materialsTriangles = []
        for i in range(ShaderArray.length () ):
            
            shaderGroup =om.MFnDependencyNode( ShaderArray[i])
            shaderPlug  = shaderGroup.findPlug( "surfaceShader" )
            connectedPlugs = om.MPlugArray ()
            shaderPlug.connectedTo(connectedPlugs, True, False)
            connectedPlugs[0].node()
            fnDN = om.MFnDependencyNode(connectedPlugs[0].node())
            #print "Material Names", i, fnDN.name()
            data["materials"].append(fnDN.name())
            materialsTriangles.append([])

        if (len(data["materials"]) == 0):
            data["materials"].append("NONE")
            materialsTriangles.append([])


        #----------------- Update Tris -----------------------
        FaceTriangles = om.MIntArray ( )
        TriangleInexies = om.MIntArray ( )
        mesh.getTriangles ( FaceTriangles , TriangleInexies )
        TriIndex = 0    
        for FaceTrianglesIndex in range(len(FaceTriangles)):
            TriPerFace = FaceTriangles[FaceTrianglesIndex]
            for CurTri  in range(TriPerFace):
                NewTris = TriangleInexies[TriIndex*3:][:3]
                if (intArrayShaderIndex[FaceTrianglesIndex] != -1):
                    materialsTriangles[intArrayShaderIndex[FaceTrianglesIndex]].extend(NewTris)
                else:
                    materialsTriangles[0].extend(NewTris)
                    
                TriIndex +=1
        data["materialsTriangles"] = []   
        data["TriangleMateralStartStop"] = []
            
        for trilist in materialsTriangles:
            data["TriangleMateralStartStop"].append(len(data["materialsTriangles"]))
            data["materialsTriangles"].extend(trilist)
       
        return json.dumps(data)





"""
import maya.cmds as cmds

import MeshManager
reload(MeshManager)

TheCurrentMesh = cmds.ls(g=1)[0]

MSMan = MeshManager.MeshManager(TheCurrentMesh)


MSMan.TickVertUpdate()

MSMan.GetDirtyVertBucket()

print MSMan.GetNextDirtyVertBucketCommand()
"""