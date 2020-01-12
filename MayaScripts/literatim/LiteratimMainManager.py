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
        self.tickRate = 1
    
    
    def Start(self,*arg):
        print(arg)
        print ("Starting client")
        #Network client
        self._literatimMayaClient = LiteratimClient.LiteratimClient()
        self._literatimMayaClient.Start() 
        self._literatimMayaClient.SetOnCommandCallback(self.OnCommand)
    
        ##Tick Thread
        print ("Starting Tick")
        self.ShouldTick = True
        self.tick_thread = threading.Thread(None, target = self.TickThread , args =(self,) )
        self.tick_thread.start()

        
    def Stop(self,*arg):
        print ("Stopping client")
        if (self._literatimMayaClient is not None):
            self._literatimMayaClient.Stop()
        self._literatimMayaClient = None
        self.ShouldTick = False
        self.tick_thread = None

        
    def TickThread(self,*args):
        while self.ShouldTick: 
            time.sleep(self.tickRate)
            utils.executeDeferred(self.Tick)
    
    def Tick(self):
        if self._literatimMayaClient != None:
            if ( self._literatimMayaClient.ReadyToKill):
                self._literatimMayaClient = None
                print "Server Down"
                self.Stop()  
                return 
            self.UpdateCamera()
            self._literatimMayaClient.Tick()
        
    def OnCommand(self, Command):
        if (Command["Command"] == "GetSceneDescription"):
            self._literatimMayaClient.SendMessage(self.scene.getSceneDescriptionJsonCommand(), LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"]  == "GetObjectTransform"):
            print ("Responding to GetObjectTransform")
            self._literatimMayaClient.SendMessage(self.scene.getObjectTransformCommand(Command["ObjectName"]),LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"] == "GetSceneDescription"):
            print ("Responding to GetSceneDescription")
            self._literatimMayaClient.SendMessage(self.scene.getSceneDescriptionJsonCommand(), LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"] == "GetObjectMeta"):
            print ("Responding to GetObjectMeta")
            self._literatimMayaClient.SendMessage(self.scene.getObjectMeshMeta(Command["ObjectName"]), LiteratimClient.ResponceHeaders.Command)
        elif  (Command["Command"] == "GetObjectWholeData"):
            print ("Responding to GetObjectWholeData")
            self._literatimMayaClient.SendMessage(self.scene.GetObjectWholeData(Command["ObjectName"]), LiteratimClient.ResponceHeaders.Command)             
        else:
            print "There was a engored command " + str(Command)
        
    def UpdateCamera(self):
        Command = MayaTools.GetCameraPosCommand()
        self._literatimMayaClient.SendMessage(Command,LiteratimClient.ResponceHeaders.Command)
    
server = RVViewManager()
