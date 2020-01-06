import maya.cmds as cmds
import threading
import time
import maya.utils as utils
import MayaTools
reload(MayaTools)

import RVViewerClient
reload(RVViewerClient)

class RVViewManager():
    ShouldTick = True
    t = None
    def Start(self,*arg):
        print(arg)
        print ("Starting client")
        #Network client
        self.viewClient = RVViewerClient.RVViewerClient()
        self.viewClient.Start() 
        self.viewClient.SetOnCommandCallback(self.OnCommand)
    
        ##Tick Thread
        print ("Starting Tick")
        self.ShouldTick = True
        self.t = threading.Thread(None, target = self.TickThread , args =(self,) )
        self.t.start()

        
    def Stop(self,*arg):
        print(arg)
        if (self.viewClient is not None):
            self.viewClient.Stop()
        self.ShouldTick = False
        self.t = None
        
    def TickThread(self,*args):
        while self.ShouldTick: 
            time.sleep(1)
            utils.executeDeferred(self.Tick)
            # always use executeDeferred or evalDeferredInMainThreadWithResult if you're running a thread in Maya!
    
    def Tick(self):
        print(cmds.currentTime(q=1))
        self.UpdateCamera()
        self.viewClient.Tick()
        
    def OnCommand(self, Command):
        print "There was a engored command " + Command
        
    def UpdateCamera(self):
        Command = MayaTools.GetCameraPosCommand()
        self.viewClient.SendMessage(Command,RVViewerClient.ResponceHeaders.Action)
    
server = RVViewManager()

#import RVViewServer
#reload(RVViewServer)
#RVViewServer.server.Start()
#RVViewServer.server.Stop()