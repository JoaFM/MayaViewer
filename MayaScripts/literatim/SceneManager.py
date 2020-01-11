import maya.cmds as cmds
import maya.OpenMaya as om
import json
import MayaTools

class SceneManager():
    def __init__(self):
        #self._objects = set()
        self._objects = {}


    def UpdateSceneDescription(self):
        scene_transform_nodes = cmds.ls(g=1)
        self._objects.clear()
        for sceneObj in scene_transform_nodes:
            self._objects[sceneObj] = "g" # C for camera


    def getSceneDescriptionJsonCommand(self):
        self.UpdateSceneDescription()
        data = {}
        data["Command"] = "SetSceneDescription"
        data["SceneObjects"] = self._objects 
        json_data = json.dumps(data)
        return json_data


    def getObjectTransformCommand(self, obj_name):
        data = {}
        parent_transform_name = cmds.listRelatives((obj_name), parent=True)[0]
        object_matrix= cmds.xform( parent_transform_name, q=True, matrix=True, ws=True )
        data["WorldMatrix"] = object_matrix
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
            shaderPlug  = shaderGroup.findPlug( "surfaceShader" );
            
            connectedPlugs = om.MPlugArray ();
            
            shaderPlug.connectedTo(connectedPlugs, True, False);
            connectedPlugs[0].node()
            fnDN = om.MFnDependencyNode(connectedPlugs[0].node())
            #print "Material Names", i, fnDN.name()
            data["materials"].append(fnDN.name())

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
                materialsTriangles[intArrayShaderIndex[FaceTrianglesIndex]].extend(TriangleInexies[TriIndex*3:][:3])
                TriIndex +=1
        data["materialsTriangles"] = []   
        data["TriangleMateralStartStop"] = []
            
        for trilist in materialsTriangles:
            data["TriangleMateralStartStop"].append(len(data["materialsTriangles"]))
            data["materialsTriangles"].extend(trilist)
       
        return json.dumps(data)