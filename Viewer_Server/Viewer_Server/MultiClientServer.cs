using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;


namespace Viewer_Server
{


    public enum StateObjectType
    {
        UNKNOWN,
        UE4,
        MAYA,

    }
    public enum CurrentState
    {
        WatingForResponceHeader,
        WaitingForPackage
    }

    public enum ResponceHeaders
    {
        SetType = 0,
        Command = 1,
        Action = 2,
    }

    // State object for reading client data asynchronously
    public class StateObject
    {
        // Client  socket.
        public Socket workSocket = null;
        // Size of receive buffer.
        public const int BufferSize = 1024;
        // Receive buffer.
        public byte[] buffer = new byte[BufferSize];
        public List<byte> m_IncommingData = new List<byte>();

        public StateObjectType StateObjType = StateObjectType.UNKNOWN;
        public ClientBase ParentClientObject = null;
        public CurrentState m_StateObjectListenState = CurrentState.WatingForResponceHeader;

        public ResponceHeaders m_nextPackageType = ResponceHeaders.SetType;
        public int m_nextPackageSize = -1;

        internal bool IsPackageSizeValidToProcess()
        {
            return m_IncommingData.Count >= m_nextPackageSize;
        }

        private bool IsAlive = true;

        internal void Deactivate()
        {
            IsAlive = false;
            workSocket = null;
        }

        internal bool IsActive()
        {
            return IsAlive;
        }


    }

    public class AsynchronousServer
    {
        private readonly int m_CommandSize = 32;

        // Public events
        public delegate void NewConnectionCreated(ref StateObject NewStateobj);
        public NewConnectionCreated NewConectionCreated;
        // Thread signal.
        public static ManualResetEvent allDone = new ManualResetEvent(false);

        //---------Lisning -----------
        private IPEndPoint localEndPoint;
        private Socket listener;

        private List<StateObject> StateObjectList = new List<StateObject>();
        public AsynchronousServer()
        {
        }

        public void StartServer()
        {
            StartListening();

        }

        private void StartListening()
        {
            // Data buffer for incoming data.
            byte[] bytes = new Byte[1024];

            // Establish the local endpoint for the socket.
            // The DNS name of the computer
            // running the listener is "host.contoso.com".
            IPHostEntry ipHostInfo = Dns.Resolve(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList[0];
            //IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 65432);
            localEndPoint = new IPEndPoint(new IPAddress(new byte[4] { 127, 0, 0, 1 }), 65432);

            // Create a TCP/IP socket.
            listener = new Socket(AddressFamily.InterNetwork,
                SocketType.Stream, ProtocolType.Tcp);

            listener.Bind(localEndPoint);
            listener.Listen(100);

            ListenForNextConnection();
        }

        internal void SendData(StateObject clientState, string content, ResponceHeaders ResponceType)
        {
            if (!clientState.workSocket.Connected)
            {
                Console.WriteLine("Client Not connected removing it" + clientState.StateObjType.ToString());
                clientState.Deactivate();
                StateObjectList.Remove(clientState);
            }
            else
            {
                if (!SendTextMessage(clientState.workSocket, content, ResponceType))
                {
                    Console.WriteLine("Client could not be reached Removing the client:" + clientState.StateObjType.ToString());
                }
            }
        }


        private void ListenForNextConnection()
        {
            // Bind the socket to the local endpoint and listen for incoming connections.
            try
            {
                // Set the event to nonsignaled state.
                allDone.Reset();

                // Start an asynchronous socket to listen for connections.
                Console.WriteLine("Waiting for a connection...");
                listener.BeginAccept(
                    new AsyncCallback(AcceptListenCallback),
                    listener);

                // Wait until a connection is made before continuing.
                //allDone.WaitOne();

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

        }

        private void AcceptListenCallback(IAsyncResult ar)
        {
            // Signal the main thread to continue.
            allDone.Set();

            // Get the socket that handles the client request.
            Socket listener = (Socket)ar.AsyncState;
            Socket handler = listener.EndAccept(ar);

            // Create the state object.
            StateObject state = new StateObject();
            StateObjectList.Add(state);
            state.workSocket = handler;
            GetClientType(state);

            handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReadCallback), state);


            ListenForNextConnection();
        }

        private void ReadCallback(IAsyncResult ar)
        {
          
            String content = String.Empty;

            // Retrieve the state object and the handler socket
            // from the asynchronous state object.
            StateObject state = (StateObject)ar.AsyncState;
            if (!state.workSocket.Connected)
            {
                Console.WriteLine("Object disconnected :(" + state.StateObjType.ToString());
            }
            Socket handler = state.workSocket;

            // Read data from the client socket. 
            int bytesRead = handler.EndReceive(ar);

            if (bytesRead > 0)
            {
                // There  might be more data, so store the data received so far.


                // copy new data
                for (int i = 0; i < bytesRead; i++)
                {
                    state.m_IncommingData.Add(state.buffer[i]);
                }

                if (state.m_StateObjectListenState == CurrentState.WatingForResponceHeader)
                {
                    if (state.m_IncommingData.Count >= 8)
                    {
                        state.m_nextPackageType = (ResponceHeaders)BitConverter.ToInt32(state.m_IncommingData.ToArray(), 0);
                        state.m_nextPackageSize = BitConverter.ToInt32(state.m_IncommingData.ToArray(), 4);
                        state.m_IncommingData.RemoveRange(0, 8);
                        state.m_StateObjectListenState = CurrentState.WaitingForPackage;

                        //Console.WriteLine("processed a HEADER");
                        //Console.WriteLine("state.m_nextPackageType : \t\t" + state.m_nextPackageType.ToString());
                        //Console.WriteLine("state.m_nextPackageSize : \t\t" + state.m_nextPackageSize.ToString());
                    }
                }
                if (state.m_StateObjectListenState == CurrentState.WaitingForPackage)
                {
                    if (state.IsPackageSizeValidToProcess())
                    {
                        if (state.m_nextPackageType == ResponceHeaders.SetType)
                        {
                            string command = Encoding.ASCII.GetString(state.m_IncommingData.ToArray(), 0, state.m_nextPackageSize);
                            HandleCommand(command, state);
                            state.m_IncommingData.RemoveRange(0, state.m_nextPackageSize);
                            state.m_StateObjectListenState = CurrentState.WatingForResponceHeader;
                        }
                        else
                        {
                            if (state.ParentClientObject != null)
                            {
                                state.ParentClientObject.ProcessPackageAndTrimData();
                            }
                            else
                            {
                                Console.WriteLine("!!! ParentClientObject Null");
                            }
                        }
                    }
                }

                {
                    // Not all data received. Get more.
                    handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                    new AsyncCallback(ReadCallback), state);
                }
            }
        }

        private void HandleCommand(string content, StateObject state)
        {

            if (content == "UE4\n")
            {
                state.StateObjType = StateObjectType.UE4;
                NewConectionCreated(ref state);
                Console.WriteLine("---- Making a UE4");

            }
            else if (content == "MAYA")
            {
                state.StateObjType = StateObjectType.MAYA;
                NewConectionCreated(ref state);
                Console.WriteLine("---- Making a Maya client");
            }
            else
            {
                if (state.ParentClientObject != null)
                {
                    state.ParentClientObject.HandleCommand(content);
                }
                else
                {
                    Console.WriteLine("NULL PARENT OBJECT: state.ParentClientObject.HandleCommand(content);");
                }
            }
        }

        bool SendResponceHeader(Socket handler, ResponceHeaders ResponceType, int ResponceSize)
        {
            byte[] RType = BitConverter.GetBytes(((int)ResponceType));
            byte[] ResponceSize_ba = BitConverter.GetBytes(((int)ResponceSize));

            try
            {
                handler.BeginSend(RType, 0, 4, 0, new AsyncCallback(SendCallback), handler);
                handler.BeginSend(ResponceSize_ba, 0, 4, 0, new AsyncCallback(SendCallback), handler);
                //Console.WriteLine("--------SendResponceHeader -----------");
                return true;
            }
            catch
            {
                Console.WriteLine("Error: Failed to send Message Header to client:  Presuming lost server");
                return false;
            }
        }


        private bool SendTextMessage(Socket handler, String data, ResponceHeaders ResponceType)
        {
            // Convert the string data to byte data using ASCII encoding.

            byte[] byteData = Encoding.ASCII.GetBytes(data);
             if (!SendResponceHeader(handler, ResponceType, byteData.Length)) return false;

            // Begin sending the data to the remote device.

            try
            {
                handler.BeginSend(byteData, 0, byteData.Length, 0,
                new AsyncCallback(SendCallback), handler);
               // Console.WriteLine("sending " + data);
                return true;
            }
            catch
            {
                Console.WriteLine("Error: Failed to send Text message to client:  Presuming lost server");
                return false;
            }
        }


        private void SendCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.
                Socket handler = (Socket)ar.AsyncState;

                // Complete sending the data to the remote device.
                int bytesSent = handler.EndSend(ar);
                //Console.WriteLine("Sent {0} bytes to client.", bytesSent);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        internal void GetClientType(StateObject newStateobj)
        {
            //TODO: this could fail if the client is down immediately on connect
            SendTextMessage(newStateobj.workSocket, "{\"Command\": \"WhatTypeAreYou\"}", ResponceHeaders.Command);
            newStateobj.m_StateObjectListenState = CurrentState.WatingForResponceHeader;
        }
    }
}