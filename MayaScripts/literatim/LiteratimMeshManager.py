import maya.cmds as cmds
import maya.OpenMaya as om
import pymel.core as pm
import math
import json
import time

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
            

    def __init__(self, _ReferenceObject, vertBucketSize = 2000, triBucketSize = 1000):
        self._obj_name = _ReferenceObject
        
        ### verts
        self._vertBucket = []
        self._vertBucketSize = vertBucketSize
        self._vertBucketIndex = 0
        self._numVert = -1

        ### Triangles    
        self._triBucket = []
        self._triBucketSize = triBucketSize
        self._triBucketIndex = 0
        self._numTri = -1
        self._numTriIndex = -1

        ### Materials
        self._materials = []
        ##Mesh reference
        self._obj = self.getMObject(self._obj_name)
        self._mesh = om.MFnMesh ( self._obj )

    def Tick(self):
        self.TickShaders()
        self.TickVertUpdate()
        self.TickTriUpdate()

    def TickShaders(self):
        ShaderArray = om.MObjectArray (  )
        intArrayShaderIndex = om.MIntArray ( )
        self._mesh.getConnectedShaders(0,ShaderArray,intArrayShaderIndex)
        self._materials = []
        #-----------------Material names-----------------------
        for i in range(ShaderArray.length () ):
            shaderGroup =om.MFnDependencyNode( ShaderArray[i])
            shaderPlug  = shaderGroup.findPlug( "surfaceShader" )
            connectedPlugs = om.MPlugArray ()
            shaderPlug.connectedTo(connectedPlugs, True, False)
            connectedPlugs[0].node()
            fnDN = om.MFnDependencyNode(connectedPlugs[0].node())
            #print "Material Names", i, fnDN.name()
            self._materials.append(fnDN.name())
        if (len(self._materials) == 0):
            self._materials.append("NONE")




    def CheckVertBucketSizes(self,numverts):
        self._numVert = numverts
        num_expexted_buckets = int(math.ceil(math.ceil(numverts / float(self._vertBucketSize))))
        if (len(self._vertBucket) != num_expexted_buckets):
            self._vertBucket = []
            for i in range(num_expexted_buckets):
                self._vertBucket.append({"Vert":[], "Hash":0.0, "Dirty":False})


    def CheckTriBucketSizes(self,numfaces, numTriIndex):
        self._numTri = numfaces
        self._numTriIndex = numTriIndex
        num_expexted_buckets = 1 + int(math.ceil(numfaces / self._triBucketSize))
        if (len(self._triBucket) != num_expexted_buckets):
            self._triBucket = []
            for i in range(num_expexted_buckets):
                self._triBucket.append({"Tri":[], "Hash":0.0, "Dirty":False})
    


    def TickTriUpdate(self):
        ## get the faces
        FaceTriangles = om.MIntArray ( )
        TriangleInexies = om.MIntArray ( )
        self._mesh.getTriangles ( FaceTriangles , TriangleInexies )
        ## Ready the buckets
        self.CheckTriBucketSizes(FaceTriangles.length(),TriangleInexies)
        self._triBucketIndex += 1
        if self._triBucketIndex >= len(self._triBucket):
            self._triBucketIndex = 0

        ## get the shader list so we can add tris to a per shader sitioation
        ShaderArray = om.MObjectArray (  )
        intArrayShaderIndex = om.MIntArray ( )
        self._mesh.getConnectedShaders(0,ShaderArray,intArrayShaderIndex)
        materialsTriangles = []
        for i in range (len(self._materials)):
            materialsTriangles.append([])
        if len(materialsTriangles) == 0:
            materialsTriangles.append([])


        scriptUtil =om.MScriptUtil()
        scriptUtil.createFromInt(0)
        dummyIndex = scriptUtil.asIntPtr()

        polygonIter = om.MItMeshPolygon(self._obj)
        for faceIndex in range(self._triBucketIndex * self._triBucketSize , min(((self._triBucketIndex * self._triBucketSize) +self._triBucketSize  ), FaceTriangles.length () )):
            polygonIter.setIndex(faceIndex, dummyIndex)
            MPointArray = om.MPointArray ( )
            NewTris = om.MIntArray  ( )
            polygonIter.getTriangles( MPointArray,NewTris )    

            if (intArrayShaderIndex[faceIndex] != -1):
                materialsTriangles[intArrayShaderIndex[faceIndex]].extend(NewTris)
            else:
                materialsTriangles[0].extend(NewTris)

        #FinalTriData = []
        #for lst in materialsTriangles:
        #    IndexIndex = len(FinalTriData)
        #    #FinalTriData.append(-1)
        #    FinalTriData.extend(lst)
            #FinalTriData[IndexIndex] = IndexIndex

        hs = 0
        for ls in materialsTriangles:
            hs += hash(tuple(ls)) 

        if (hs != self._triBucket[self._triBucketIndex]["Hash"]):
            self._triBucket[self._triBucketIndex]["Hash"] = hs          
            self._triBucket[self._triBucketIndex]["Tri"] = materialsTriangles          
            self._triBucket[self._triBucketIndex]["Dirty"] = True


    def GetDirtyTriBucket(self):
        for i in range(len(self._triBucket)):
            if self._triBucket[i]["Dirty"]:
                return i
        return None

    def GetNextDirtyTriBucketCommand(self, cleanBucketOnGet = True):

        returnCommands = None
        AnythingDirty = False
        i = self.GetDirtyTriBucket()    
        if (i != None):
            if cleanBucketOnGet:
                self._triBucket[i]["Dirty"] = False

            for matI in range(len(self._triBucket[i]["Tri"] )):
                if (returnCommands is None):
                    returnCommands = []

                matTri = self._triBucket[i]["Tri"][matI]
                data = {}
                data["objectName"] = self._obj_name
                data["Command"] = "SetMeshBucketTris" 
                data["Tri"] =  matTri
                data["MatIndex"] = matI
                data["HashNum"] =  self._triBucket[i]["Hash"]
                data["Num"] = self._numTri
                data["BucketIndex"] = i
                data["NumBuckets"] = len(self._triBucket)
                data["BucketSize"] = self._triBucketSize
                returnCommands.append(json.dumps(data))
        return returnCommands


    def TickVertUpdate(self):
        msPointArray = om.MFloatPointArray ( )
        self._mesh.getPoints (msPointArray )
        
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
            data["HashNum"] =  self._vertBucket[i]["Hash"]
            data["BucketIndex"] = i
            data["Num"] = self._numVert
            data["NumBuckets"] = len(self._vertBucket)
            data["BucketSize"] = self._vertBucketSize
            AnythingDirty = True
        if AnythingDirty:
            return json.dumps(data) 
        else:
            return None   


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