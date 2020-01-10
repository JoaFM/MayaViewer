import socket
import sys
import select
import socket
import sys
import struct
import MayaTools
import json

class CurrentState: 
    WatingForResponceHeader = 1
    WaitingForPackage = 2

class ResponceHeaders:
    SetType = 0
    Command = 1
    Action = 2

class LiteratimClient():
    OnCommandFunction = None
    isStarted = False
    serverisReady = False
    
    def SetOnCommandCallback(self, _OnCommandFunction):
        self.OnCommandFunction = _OnCommandFunction
    
    
    def Start(self):
        # Create a TCP/IP socket
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Connect the socket to the port where the server is listening
        server_address = ('localhost', 65432)
        print (sys.stderr, 'connecting to %s port %s' % server_address)
        self.server.connect(server_address)
        self.server.setblocking(False)

        self.inputs = [self.server]
        self.outputs = []
        self.isStarted = True
        self.serverisReady = False
        
        self.m_StateObjectListenState = CurrentState.WatingForResponceHeader
        self.m_nextPackageType = ResponceHeaders.SetType
        self.m_nextPackageSize = -1 
        self.m_IncommingData = b""
        
    def Stop(self):
        self.server = None
        self.isStarted = False
        self.serverisReady = False


    def Tick(self):
        if (not self.isStarted): return
        
        readable, writable, exceptional = select.select(self.inputs, [], [],0)
        
        while len(readable) > 0:
            for s in readable:
                data = s.recv(4096*8)
                if data:
                    self.m_IncommingData += data
                readable, writable, exceptional = select.select(self.inputs, [], [],0)
       
        self.ProcessAcumData()

    
   
    def ProcessAcumData(self):
        ProcessedData = True

        while ProcessedData:
            ProcessedData = False
            if self.m_StateObjectListenState == CurrentState.WatingForResponceHeader:
                if len(self.m_IncommingData) >= 8:
                    
                    unpacker = struct.Struct('I I')
                    self.m_nextPackageType, self.m_nextPackageSize = unpacker.unpack(self.m_IncommingData[:8])
                    
                    # Set up for next action                
                    self.m_IncommingData = self.m_IncommingData[8:]
                    self.m_StateObjectListenState = CurrentState.WaitingForPackage
                    ProcessedData = True  
                    
            if self.m_StateObjectListenState == CurrentState.WaitingForPackage:
                if len(self.m_IncommingData) >= self.m_nextPackageSize:
                    
                    Command = self.m_IncommingData[:self.m_nextPackageSize]
                    try:
                        CommandDict = json.loads(Command)
                    except :
                        print "!!!!!!!!!!!Command error!!!!!!!!!!!!!!!"
                        print Command, "len:",len(self.m_IncommingData) , "nexSi" , self.m_nextPackageSize
                        print self.m_IncommingData

                    if  CommandDict["Command"] == "WhatTypeAreYou":
                        print ("Responding to WhatTypeAreYou")
                        self.SendMessage("MAYA", ResponceHeaders.SetType)
                        self.serverisReady = True
                    elif (self.OnCommandFunction != None):
                        print "----------------------------------"
                        print Command
                        self.OnCommandFunction(CommandDict)
                        
                        
                    # Set up for next action
                    self.m_IncommingData = self.m_IncommingData[self.m_nextPackageSize:]
                    self.m_StateObjectListenState = CurrentState.WatingForResponceHeader  
                    ProcessedData = True   
            
    
    def SendMessage(self, _Message, ResponceHeaders_type):
        
        if ((ResponceHeaders_type != ResponceHeaders.SetType) and (not self.serverisReady )):

            print "Server state not ready"    
            return 
        
        # print ("SendMessage::" + _Message,ResponceHeaders_type)
        
        values = (ResponceHeaders_type, len(_Message))
        packer = struct.Struct('I I')
        packed_data = packer.pack(*values)
        try :
            self.server.sendall(packed_data)
            self.server.sendall(_Message)
        except socket.error, exc:
            print "Caught exception socket.error : %s" % exc
            self.Stop()


        
























        
