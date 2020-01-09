import maya.cmds as cmds
import json

class SceneManager():
    def __init__(self):
        #self._objects = set()
        self._objects = {}


    def UpdateSceneDescription(self):
        scene_transform_nodes = cmds.ls(g=1)
        self._objects.clear()
        for sceneObj in scene_transform_nodes:
            self._objects[sceneObj] = "G" # C for camera

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
