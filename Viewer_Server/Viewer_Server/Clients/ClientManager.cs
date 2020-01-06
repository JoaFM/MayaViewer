using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Viewer_Server.Clients
{

    public class ClientManager
    {
        public int NumberOfClients
        {
            get
            {
                return m_Clients.Count;
            }
        }
        public string NumberOfClients2
        {
            get
            {
                return "1231232";
            }
            set
            {
            }

        }

        private AsynchronousServer ASSView = null;
        private List<ClientBase> m_Clients = new List<ClientBase>();
        public ClientManager()
        {
            ASSView = new AsynchronousServer();
            ASSView.NewConectionCreated = NewConnectionCreated;
        }

        private void  NewConnectionCreated(ref StateObject NewStateobj)
        {
            Console.WriteLine("CONNECTED TO ::" + NewStateobj.workSocket.RemoteEndPoint.ToString());
            if (NewStateobj.StateObjType == StateObjectType.UE4)
            {
                ClientUE4 NewUE4Client = new ClientUE4(ref NewStateobj, m_Clients, ASSView);
                m_Clients.Add(NewUE4Client);
            }
            else if (NewStateobj.StateObjType == StateObjectType.MAYA)
            {
                ClientMAYA NewMayaClient = new ClientMAYA(ref NewStateobj, m_Clients, ASSView);
                m_Clients.Add(NewMayaClient);
            }
        }


        internal void StartServer()
        {
            ASSView.StartServer();
        }

      
    }
}
