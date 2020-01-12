using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Viewer_Server.Clients
{
    public class ClientUE4 : ClientBase
    {
        public ClientUE4(ref StateObject ClientStateParam, ObservableCollection<ClientBase> Clientsparam, AsynchronousServer ASSView) 
            : base(ref ClientStateParam, Clientsparam, ASSView)
        {
            Console.WriteLine("MADE A ClientUE4");
        }

        internal override void HandleCommand(string content)
        {
            dynamic json = JValue.Parse(content);
            for (int i = m_Clients.Count - 1; i >= 0; i--)
            {
                StateObject ClientState = (m_Clients[i].m_ClientState.Target as StateObject);
                if (ClientState == null || !ClientState.IsActive())
                {
                    return;
                }
                else if (ClientState.StateObjType == StateObjectType.MAYA)
                {
                    m_ASSView.SendData(ClientState, content, ResponceHeaders.Command);
                }
            }

        }
    }
}
