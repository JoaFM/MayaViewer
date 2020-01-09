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
    CamMat = cmds.xform( "persp", q=True, matrix=True, ws=True )
    
    data = {}


    
    data['Command'] = 'SetCamera'
    data["WorldMatrix"] = [  CamMat[0],CamMat[1],CamMat[2],CamMat[3],
                        CamMat[4],CamMat[5],CamMat[6],CamMat[7],
                        CamMat[8],CamMat[9],CamMat[10],CamMat[11],
                        CamMat[12],CamMat[13],CamMat[14],CamMat[15],
                        ]


    json_data = json.dumps(data)
    return json_data
