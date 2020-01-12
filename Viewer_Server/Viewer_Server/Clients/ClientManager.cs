using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Timers;
using System.Threading.Tasks;

namespace Viewer_Server.Clients
{

    public class ClientManager : INotifyPropertyChanged
    {

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public int NumberOfClients{
            get {return Clients.Count;}
        }

        public ObservableCollection<ClientBase> Clients
        {
            get {return m_Clients;}
            set {
                m_Clients = value;
                OnPropertyChanged("Clients");
            }
        }

        private static Timer m_ClientMantance;
        private AsynchronousServer ASSView = null;
        private ObservableCollection<ClientBase> m_Clients = new ObservableCollection<ClientBase>();


        public ClientManager()
        {
            ASSView = new AsynchronousServer();
            ASSView.NewConectionCreated = NewConnectionCreated;

            m_ClientMantance = new System.Timers.Timer();
            m_ClientMantance.Interval = 2000;
            m_ClientMantance.Elapsed += RunMaintance;
            m_ClientMantance.AutoReset = true;
            m_ClientMantance.Enabled = true;
        }

        private void RunMaintance(object sender, ElapsedEventArgs e)
        {
            //POLL
            for (int i = m_Clients.Count - 1; i >= 0; i--)
            {
                if (m_Clients[i].m_ClientState.IsAlive )
                {
                    StateObject SO = (m_Clients[i].m_ClientState.Target as StateObject);

                    if (SO.IsActive())
                    {
                        if ((m_Clients[i].m_ClientState.Target as StateObject).workSocket.Poll(1000, System.Net.Sockets.SelectMode.SelectRead))
                        {
                            (m_Clients[i].m_ClientState.Target as StateObject).Deactivate();
                        }
                    }
                }
            }

            // Remove Dead 
            for (int i = m_Clients.Count - 1; i >= 0; i--)
            {
                if (!m_Clients[i].m_ClientState.IsAlive || !(m_Clients[i].m_ClientState.Target as StateObject).IsActive())
                {
                    App.Current.Dispatcher.Invoke((Action)delegate // <--- HERE
                    {
                        m_Clients.Remove(m_Clients[i]);
                    });
                }
            }
        }

        private void  NewConnectionCreated(ref StateObject NewStateobj)
        {
            Console.WriteLine("CONNECTED TO ::" + NewStateobj.workSocket.RemoteEndPoint.ToString());
            if (NewStateobj.StateObjType == StateObjectType.UE4)
            {
                ClientUE4 NewUE4Client = new ClientUE4(ref NewStateobj, Clients, ASSView);

                App.Current.Dispatcher.Invoke((Action) delegate
                {
                    Clients.Add(NewUE4Client);
                });
            }
            else if (NewStateobj.StateObjType == StateObjectType.MAYA)
            {
                ClientMAYA NewMayaClient = new ClientMAYA(ref NewStateobj, Clients, ASSView);

                App.Current.Dispatcher.Invoke((Action)delegate // <--- HERE
                {
                    Clients.Add(NewMayaClient);
                });
            }
        }
        internal void StartServer()
        {
            ASSView.StartServer();
        }
    }
}
