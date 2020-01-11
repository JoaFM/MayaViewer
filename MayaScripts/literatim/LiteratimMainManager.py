import maya.cmds as cmds
import threading
import time
import maya.utils as utils

import MayaTools
reload(MayaTools)

import LiteratimClient
reload(LiteratimClient)

import SceneManager 
reload(SceneManager)

class RVViewManager():
    def __init__(self):
        self.ShouldTick = True
        self.tick_thread = None
        self.scene = SceneManager.SceneManager()
        
    def Start(self,*arg):
        print(arg)
        print ("Starting client")
        #Network client
        self.viewClient = LiteratimClient.LiteratimClient()
        self.viewClient.Start() 
        self.viewClient.SetOnCommandCallback(self.OnCommand)
    
        ##Tick Thread
        print ("Starting Tick")
        self.ShouldTick = True
        self.tick_thread = threading.Thread(None, target = self.TickThread , args =(self,) )
        self.tick_thread.start()

        
    def Stop(self,*arg):
        print(arg)
        if (self.viewClient is not None):
            self.viewClient.Stop()
        self.ShouldTick = False
        self.tick_thread = None
        
    def TickThread(self,*args):
        while self.ShouldTick: 
            time.sleep(1)
            utils.executeDeferred(self.Tick)
    
    def Tick(self):
        self.UpdateCamera()
        self.viewClient.Tick()
        
    def OnCommand(self, Command):
        if (Command["Command"] == "GetSceneDescription"):
            self.viewClient.SendMessage(self.scene.getSceneDescriptionJsonCommand(), LiteratimClient.ResponceHeaders.Action)
        elif  (Command["Command"]  == "GetObjectTransform"):
            print ("Responding to GetObjectTransform")
            self.viewClient.SendMessage(self.scene.getObjectTransformCommand(Command["ObjectName"]),LiteratimClient.ResponceHeaders.Action)
        elif  (Command["Command"] == "GetSceneDescription"):
            print ("Responding to GetSceneDescription")
            self.viewClient.SendMessage(self.scene.getSceneDescriptionJsonCommand(), LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"] == "GetObjectMeta"):
            print ("Responding to GetObjectMeta")
            self.viewClient.SendMessage(self.scene.getObjectMeshMeta(Command["ObjectName"]), LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"] == "GetObjectWholeData"):
            print ("Responding to GetObjectWholeData")
            self.viewClient.SendMessage(self.scene.GetObjectWholeData(Command["ObjectName"]), LiteratimClient.ResponceHeaders.Action)             
        else:
            print "There was a engored command " + str(Command)
        
    def UpdateCamera(self):
        Command = MayaTools.GetCameraPosCommand()
        self.viewClient.SendMessage(Command,LiteratimClient.ResponceHeaders.Action)
    
server = RVViewManager()
