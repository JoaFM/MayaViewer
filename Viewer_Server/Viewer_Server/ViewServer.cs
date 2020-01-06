using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Viewer_Server
{
    class ViewClient
    {
        private TcpClient client = null;
        private NetworkStream stream = null;

        public ViewClient(TcpClient clientparam)
        {
            client = clientparam;
            stream = client.GetStream();

            StreamWriter writer = new StreamWriter(stream, Encoding.ASCII) { AutoFlush = true };
            StreamReader reader = new StreamReader(stream, Encoding.ASCII);


            while (true)
            {

                string inputLine = "";
                while (inputLine != null)
                {
                    try
                    {
                        inputLine = reader.ReadLine();
                    }
                    catch
                    {
                        Console.WriteLine("!!!!!!!!! Client Disconnected HARD");
                        return;
                    }


                    writer.WriteLine("Echoing string: " + inputLine);
                    Console.WriteLine("Echoing string: " + inputLine);
                }
                Console.WriteLine("Server saw disconnect from client.");
            }
        }



        private void Dissconect()
        {
            client.Client.Disconnect(false);
            stream.Close();
            client.Close();
            Console.WriteLine("Dissconect");
        }
    }


    class ViewServer
    {
        List<ViewClient> Clients = new List<ViewClient>();


        public ViewServer()
        {
            Console.WriteLine("Starting echo server...");

            int port = 65432;
            TcpListener listener = new TcpListener(IPAddress.Loopback, port);
            listener.Start();
            Clients.Add( new ViewClient( listener.AcceptTcpClient()));
        }

       
    }
}
