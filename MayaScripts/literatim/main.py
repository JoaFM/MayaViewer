
import maya.cmds as cmds

import LiteratimMainManager
reload(LiteratimMainManager)

def RELOAD(*arg):
    reload(LiteratimMainManager)

def TestSceneM(*arg):
    import SceneManager
    reload(SceneManager)
    SM = SceneManager.SceneManager()
    SM.UpdateSceneDescription()
    #print (SM.getSceneDescriptionJson())
    print SM.getObjectTransform("pSphereShape22")

    
def Start():
    cmds.window(title='Test Window')
    cmds.rowLayout(nc=4)
    cmds.button(label="Start", width=100, c=LiteratimMainManager.server.Start)
    cmds.button(label="Stop", width=100,c=LiteratimMainManager.server.Stop)
    cmds.button(label="RELOAD", width=100,c=RELOAD)
    cmds.button(label="Test_SceneManager", width=100,c=TestSceneM)

    cmds.showWindow()

#import literatim.main as main
#main.Start()
