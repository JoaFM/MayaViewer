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
            Console.WriteLine("COMMAND|"+ content);

        }
    }
}
