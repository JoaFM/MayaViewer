import maya.cmds as cmds
import maya.OpenMaya as om
import json
import MayaTools

class SceneManager():
    def __init__(self):
        self._scene_description = {}
        self.UpdateList = []
        self._mesh_data = {}
        self._transformData = {}


    def UpdateSceneDescription(self):
        scene_transform_nodes = cmds.ls(g=1,noIntermediate=True)
        OldList = self._scene_description.copy()
        self._scene_description.clear()
        for sceneObj in scene_transform_nodes:
            if (sceneObj in OldList):
                self._scene_description[sceneObj] = OldList[sceneObj]    
            else:
                self._scene_description[sceneObj] = {"c":"g", "h":"", "t":""} # C for camera
        self.TickMeshUpdate()


    def TickMeshUpdate(self):
        """
        Essentialy take One mesh every tick and see if it is up to date
        IE make a hash and store it in hte scene description
        """

        #Loop the list
        if ( len(self.UpdateList) == 0):
            self.UpdateList =  self._scene_description.keys()
        
        #take one object
        if (len(self.UpdateList) == 0):
            return
        nextMeshToUpdate = self.UpdateList.pop()
        self._mesh_data[nextMeshToUpdate] = self.GetObjectWholeData(nextMeshToUpdate)
        self._transformData[nextMeshToUpdate] = self.getObjectTransformCommand(nextMeshToUpdate)
        self._scene_description[nextMeshToUpdate] = self.UpdateHash(
                                                        self._scene_description[nextMeshToUpdate],
                                                        None,
                                                        self.HashStr(self._mesh_data[nextMeshToUpdate]),
                                                        self.HashStr(self._transformData[nextMeshToUpdate])
                                                    )


    def HashStr(self, _str):
        return str(abs(hash(_str)) % (10 ** 8) )


    def getSceneDescriptionJsonCommand(self):
        self.UpdateSceneDescription()
        data = {}
        data["Command"] = "SetSceneDescription"
        data["SceneObjects"] = self._scene_description 
        json_data = json.dumps(data)
        return json_data


    def UpdateHash(self, _originalHash, _type, _meshHash, _TransformHash):
        NewHash = _originalHash.copy()
        if (_type is not None):
            NewHash["c"] = _type
        if (_meshHash is not None):
            NewHash["h"] = _meshHash
        if (_TransformHash is not None):
            NewHash["t"] = _TransformHash
        return NewHash

    #################################### Commands ##############################################################

    def getObjectTransformCommand(self, obj_name):
        data = {}
        parent_transform_name = cmds.listRelatives((obj_name), parent=True)[0]
        object_matrix= cmds.xform( parent_transform_name, q=True, matrix=True, ws=True )
        object_scale = cmds.xform( parent_transform_name, q=True, s=True, r=True)
        data["WorldMatrix"] = object_matrix
        data["scale"] = {"x":object_scale[0],"y":object_scale[1],"z":object_scale[2]}
        data["objectName"] = obj_name
        data["Command"] = "SetObjectTransform"
        json_data = json.dumps(data)
        return json_data
    
    
    def getObjectMeshMeta(self, obj_name):
        bounding_box = cmds.exactWorldBoundingBox(obj_name)
        data = {}
        data["objectName"] = obj_name
        data["Command"] = "SetObjectMeta"
        data["min"] = {"x":bounding_box[0],"y":bounding_box[1],"z":bounding_box[2]}
        data["max"] = {"x":bounding_box[3],"y":bounding_box[4],"z":bounding_box[5]}
        json_data = json.dumps(data)
        return json_data


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

        FaceTriangles = om.MIntArray ( )
        TriangleInexies = om.MIntArray ( )
        mesh.getTriangles ( FaceTriangles , TriangleInexies )
        #print "per Face Triangles count",FaceTriangles
        #print "Triangle Inexies",TriangleInexies
        
        ShaderArray = om.MObjectArray (  )
        intArrayShaderIndex = om.MIntArray ( )
        mesh.getConnectedShaders(0,ShaderArray,intArrayShaderIndex)
        #print "ShaderArray",ShaderArray
        #print "intArrayShaderIndex",intArrayShaderIndex   
        data["materials"] = []
        materialsTriangles = []
        #-----------------Material names-----------------------
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
        #-----------------Material names-----------------------
        
        TriIndex = 0    
        for FaceTrianglesIndex in range(len(FaceTriangles)):
            TriPerFace = FaceTriangles[FaceTrianglesIndex]
        #for TriPerFace in  FaceTriangles:
            #print "Tri Per Face",TriPerFace
            for CurTri  in range(TriPerFace):
                #Pint most usefull info in a list
                #print TriIndex, TriangleInexies[TriIndex*3:][:3],intArrayShaderIndex[FaceTrianglesIndex]
                NewTris = TriangleInexies[TriIndex*3:][:3]
                #print "intArrayShaderIndex" , intArrayShaderIndex
                #print "FaceTrianglesIndex", FaceTrianglesIndex
                #print "NewTris", NewTris

                #print "intArrayShaderIndex[FaceTrianglesIndex]",intArrayShaderIndex[FaceTrianglesIndex]

                if (intArrayShaderIndex[FaceTrianglesIndex] != -1):
                    #print ">>>>>1"
                    materialsTriangles[intArrayShaderIndex[FaceTrianglesIndex]].extend(NewTris)
                else:
                    #print "<<<<<<<<<<!= -1"
                    materialsTriangles[0].extend(NewTris)
                    
                TriIndex +=1
        data["materialsTriangles"] = []   
        data["TriangleMateralStartStop"] = []
            
        for trilist in materialsTriangles:
            data["TriangleMateralStartStop"].append(len(data["materialsTriangles"]))
            data["materialsTriangles"].extend(trilist)
       
        return json.dumps(data)