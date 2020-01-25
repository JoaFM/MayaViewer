import maya.cmds as cmds
import maya.OpenMaya as om
import json
import MayaTools

import LiteratimMeshManager
reload(LiteratimMeshManager)

class SceneManager():
    def __init__(self):
        # a orderd list of all the scene names
        self._sceneMeshes = []

        # A dictonary with all the sece mesh data
        self._scene = {}
        
        
        # Each tick check if a mash is dirty
        # This holds the last dirty mesh to check if it has become clean
        # if clean the number will be incremented
        self._sceneMeshesUpdateIndex = 0
        
        #On each tick check a certain amount of transforms
        #This holds the last checked position
        self._transformUpdateIndex = 0

        # A list of transforms to send on next request
        self._transformUpdateData = {}        

        self._out_commands = []

        self._meshStarted= False

    def UpdateSceneDescription(self):
        scene_Shape_nodes = cmds.ls(g=1,noIntermediate=True)
        OldList = self._scene.copy()

        self._scene.clear()

        has_scene_changed = False
        #check new 
        for sceneObj in scene_Shape_nodes:
            if (sceneObj in OldList):
                self._scene[sceneObj] = OldList[sceneObj]    
            else:
                self._scene[sceneObj] = {"Mesh" : LiteratimMeshManager.MeshManager(sceneObj), "MH" : True, "TH":True   } #meshManager, mesh has , tansform hash
                has_scene_changed = True
                self.signel_new_Object(sceneObj)

        for old_scene_obj in OldList:
            if old_scene_obj not in self._scene:
                self.signel_del_Object(old_scene_obj)


        if has_scene_changed:
            print ">>> Scene Changed interal"
            self._sceneMeshes = []
            self._sceneMeshesUpdateIndex = 0
            for sceneObjkey in self._scene:
                self._sceneMeshes.append(sceneObjkey)


    def signel_new_Object(self, sceneObj):
        data = {}
        data["Command"] = "SpawnNewObject"
        data["objectName"] = sceneObj
        json_data = json.dumps(data)
        self._out_commands.append(json_data)

    def signel_del_Object(self, sceneObj):
        data = {}
        data["Command"] = "SpawnNewObject"
        data["objectName"] = sceneObj
        json_data = json.dumps(data)
        self._out_commands.append(json_data)


    def clear_command_list(self):
        print "Clear commands"
        self._out_commands *= 0

    def get_command_list (self):
        print "Get Commands",self._out_commands
        return self._out_commands

    def Tick(self):
        self.UpdateSceneDescription()

        ## check for empty scene
        if len(self._sceneMeshes) == 0:
            return

        mesh_key = self._sceneMeshes[self._sceneMeshesUpdateIndex]

        self._scene[mesh_key]["Mesh"].Tick()
        ## increment and loop the mesh index
        
        
        BucketCommands = self._scene[mesh_key]["Mesh"].GetNextDirtyTriBucketCommand()
        TriCommand =  self._scene[mesh_key]["Mesh"].GetNextDirtyVertBucketCommand()

        if ((BucketCommands is None) and (TriCommand is None) ):
            if self._meshStarted:
                self._meshStarted = False
                self.sendMeshDone(self._sceneMeshes[self._sceneMeshesUpdateIndex])
            self._sceneMeshesUpdateIndex = (self._sceneMeshesUpdateIndex + 1) % len(self._sceneMeshes)
            

        if BucketCommands is not None :
            self._meshStarted = True
            for buckCommand in BucketCommands:
                self._out_commands.append(buckCommand)
        if TriCommand is not None :
            self._meshStarted = True
            self._out_commands.append(TriCommand)
             
        
    def sendMeshDone(self, meshName):
        data = {}
        data["Command"] = "MeshDone"
        data["objectName"] = meshName
        json_data = json.dumps(data)
        self._out_commands.append(json_data)
        
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
        
        ShaderArray = om.MObjectArray (  )
        intArrayShaderIndex = om.MIntArray ( )
        mesh.getConnectedShaders(0,ShaderArray,intArrayShaderIndex)
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
            data["materials"].append(fnDN.name())
            materialsTriangles.append([])

        if (len(data["materials"]) == 0):
            data["materials"].append("NONE")
            materialsTriangles.append([])
        #-----------------Material names-----------------------
        
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