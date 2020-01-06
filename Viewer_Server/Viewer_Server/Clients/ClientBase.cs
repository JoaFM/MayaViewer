using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Viewer_Server
{
    public class ClientBase
    {
        internal WeakReference m_ClientState;
        //internal StateObject m_ClientState;
        internal List<ClientBase> m_Clients;
        internal AsynchronousServer m_ASSView;
        public ClientBase(ref StateObject ClientStateParam, List<ClientBase> Clientsparam, AsynchronousServer ASSView)
        {
            //m_ClientState = ClientStateParam;
            m_ClientState = new WeakReference(ClientStateParam);
            ClientStateParam.ParentClientObject = this;
            m_Clients = Clientsparam;
            m_ASSView = ASSView;
        }

        internal virtual void HandleCommand(string content)
        {
           
        }

        internal virtual void ProcessData()
        {
            
        }
 

        internal void ProcessPackageAndTrimData()
        {
            StateObject ClientState = (m_ClientState.Target as StateObject);
            if (ClientState == null)
                return;

            if ((m_ClientState.Target as StateObject).m_nextPackageType == ResponceHeaders.Command)
            {
                string command = Encoding.ASCII.GetString(ClientState.m_IncommingData.ToArray(), 0, ClientState.m_nextPackageSize);
                Console.WriteLine("Got A command " + command);
                ClientState.m_IncommingData.RemoveRange(0, ClientState.m_nextPackageSize);
                ClientState.m_StateObjectListenState = CurrentState.WatingForResponceHeader;
            }
            else if (ClientState.m_nextPackageType == ResponceHeaders.Action)
            {
                string command = Encoding.ASCII.GetString(ClientState.m_IncommingData.ToArray(), 0, ClientState.m_nextPackageSize);
                Console.WriteLine("Got A Action " + command);
                ClientState.m_IncommingData.RemoveRange(0, ClientState.m_nextPackageSize);
                HandleCommand(command);
                ClientState.m_StateObjectListenState = CurrentState.WatingForResponceHeader;
            }
       
        }
    }
}
