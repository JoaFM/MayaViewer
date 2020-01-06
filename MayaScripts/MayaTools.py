import json
import maya.cmds as cmds 


import maya.cmds as mc
import maya.OpenMaya as om

def getLocalVecToWorldSpaceAPI(obj, vec=om.MVector.yAxis):

	# If the input vec is not an MVector assume it's an iterable
	# and convert it to an MVector
	if not isinstance(vec, om.MVector):
		vec = om.MVector(vec[0], vec[1], vec[2])
		
	selList = om.MSelectionList()
	selList.add(obj)

	nodeDagPath = om.MDagPath()
	selList.getDagPath(0, nodeDagPath)

	matrix = om.MFnTransform(nodeDagPath).transformation().asMatrix()
	
	vec = vec * matrix
	return vec.x, vec.y, vec.z




def GetCameraPosCommand():
    #CamPos = cmds.xform("persp",q=1,ws=1,rp=1)
    CamMat = cmds.xform( "persp", q=True, matrix=True, ws=True )
    CamRotation = cmds.xform("persp",q=1,ws=1,ro=1)
    #CameraYAxis =  getLocalVecToWorldSpaceAPI("persp",vec=om.MVector.yAxis)
    #CameraZAxis =  getLocalVecToWorldSpaceAPI("persp",vec=om.MVector.zAxis)
    
    data = {}


    
    data['Command'] = 'SetCamera'
    #data["location"] = {"x":CamPos[0],"y":CamPos[1], "z":CamPos[2]}
    data["MayaRotation"] =  {"x":CamRotation[0],"y":CamRotation[1], "z":CamRotation[2]}
    #data["UpVector"] = {"x":CameraYAxis[0],"y":CameraYAxis[1], "z":CameraYAxis[2]}
    #data["ForwardVector"] = {"x":CameraZAxis[0],"y":CameraZAxis[1], "z":CameraZAxis[2]}
    data["WorldMatrix"] = [  CamMat[0],CamMat[1],CamMat[2],CamMat[3],
                        CamMat[4],CamMat[5],CamMat[6],CamMat[7],
                        CamMat[8],CamMat[9],CamMat[10],CamMat[11],
                        CamMat[12],CamMat[13],CamMat[14],CamMat[15],
                        ]


    json_data = json.dumps(data)
    return json_data
