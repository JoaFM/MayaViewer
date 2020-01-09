using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Viewer_Server.Clients
{
    public class ClientUE4 : ClientBase
    {
        public ClientUE4(ref StateObject ClientStateParam, List<ClientBase> Clientsparam, AsynchronousServer ASSView) 
            : base(ref ClientStateParam, Clientsparam, ASSView)
        {
            Console.WriteLine("MADE A ClientUE4");

        }

        internal override void HandleCommand(string content)
        {
            Console.WriteLine("Get Action from UE4" + content);

            dynamic json = JValue.Parse(content);

           // if (json.Command == "GetSceneDescription")
            {
                for (int i = m_Clients.Count - 1; i >= 0; i--)
                {
                    StateObject ClientState = (m_Clients[i].m_ClientState.Target as StateObject);
                    if (ClientState == null || !ClientState.IsActive())
                    {
                        m_Clients.Remove(m_Clients[i]);
                    }
                    else if (ClientState.StateObjType == StateObjectType.MAYA)
                    {
                        m_ASSView.SendData(ClientState, content, ResponceHeaders.Action);
                    }
                }

            }

        }
    }
}
