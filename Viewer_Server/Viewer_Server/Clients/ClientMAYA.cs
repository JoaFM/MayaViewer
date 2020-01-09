using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace Viewer_Server.Clients
{
    class ClientMAYA : ClientBase
    {
        public ClientMAYA(ref StateObject ClientStateParam, List<ClientBase> Clientsparam, AsynchronousServer ASSView)
            : base(ref ClientStateParam, Clientsparam, ASSView)
        {
            Console.WriteLine("MADE A ClientMAYA");

        }

        internal override void HandleCommand(string content)
        {
            Console.WriteLine("Get Action from Maya" + content);

            dynamic json = JValue.Parse(content);

            //TODO make a flag that means pass on commands
            //if (json.Command == "SetCamera")
            {
                for (int i = m_Clients.Count -1; i >= 0; i--)
                {

                    StateObject ClientState = (m_Clients[i].m_ClientState.Target as StateObject);
                    if (ClientState == null || !ClientState.IsActive())
                    {
                        m_Clients.Remove(m_Clients[i]);
                    }
                    else if (ClientState.StateObjType == StateObjectType.UE4)
                    {
                        m_ASSView.SendData(ClientState, content, ResponceHeaders.Action);
                    }
                }
              
            }

        }
    }
}
