import maya.cmds as cmds
import threading
import time
import maya.utils as utils

class RVViewServer():
    ShouldTick = True
    t = None
    def Start(self):
        self.ShouldTick = True
        self.t = threading.Thread(None, target = self.TickThread , args =(self,) )
        self.t.start()
        
    
    def Stop(self):
        self.ShouldTick = False
        self.t = None
        
    def TickThread(self,*args):
        while self.ShouldTick: 
            time.sleep(1)
            utils.executeDeferred(self.Tick)
            # always use executeDeferred or evalDeferredInMainThreadWithResult if you're running a thread in Maya!
    
    def Tick(self):
        print cmds.currentTime(q=1)
    
server = RVViewServer()

#import RVViewServer
#reload(RVViewServer)
#RVViewServer.server.Start()
#RVViewServer.server.Stop()