import maya.cmds as cmds
import threading
import time
import maya.utils as utils

import MayaTools
reload(MayaTools)

import LiteratimClient
reload(LiteratimClient)

import LiteratimSceneManager 
reload(LiteratimSceneManager)

class RVViewManager():
    def __init__(self):
        self.ShouldTick = True
        self.tick_thread = None
        self.scene = LiteratimSceneManager.SceneManager()
        self.tickRate = 1
        self.lastCameraHash = -1
    
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
            self.scene.Tick()
            self.UpdateCamera()
            self.send_command_list()
            self._literatimMayaClient.Tick()
            
    def OnCommand(self, Command):
        #if (Command["Command"] == "GetSceneCommand"):

        #else:
        print "There was a engored command " + str(Command)

    def send_command_list(self):
        SceneCommands = self.scene.get_command_list()
        for Command in SceneCommands:
            print "Command>>>"  + str(Command) + "<<<<<<<<<<"
            if not self._literatimMayaClient.SendMessage(Command, LiteratimClient.ResponceHeaders.Command):
                return 

        self.scene.clear_command_list()


    def UpdateCamera(self):
        Command = MayaTools.GetCameraPosCommand()
        commandhash = hash(Command) #  
        if (self.lastCameraHash == commandhash):
            return
        
        if self._literatimMayaClient.SendMessage(Command,LiteratimClient.ResponceHeaders.Command):
            self.lastCameraHash = commandhash

    
server = RVViewManager()
