using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;

namespace Viewer_Server
{
    public class ClientBase : INotifyPropertyChanged
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

        internal AsynchronousServer m_ASSView;
        internal WeakReference m_ClientState;

        public string ClientType
        {
            get;
            private set;
        }

        private ObservableCollection<ClientBase> clients;
        internal ObservableCollection<ClientBase> m_Clients
        {
            get {return clients;}
            set {
                clients = value;
                OnPropertyChanged("Clients");
            }
        }

        private string name = "????";
        public string Name
        {
            get {return name;}
            set{
                name = value;
                OnPropertyChanged("Name");
            }
        }


        public ClientBase(ref StateObject ClientStateParam, ObservableCollection<ClientBase> Clientsparam, AsynchronousServer ASSView)
        {
            Name = ClientStateParam.workSocket.RemoteEndPoint.ToString();
            m_ClientState = new WeakReference(ClientStateParam);
            ClientStateParam.ParentClientObject = this;
            m_Clients = Clientsparam;
            m_ASSView = ASSView;
            ClientType = ClientStateParam.StateObjType.ToString();
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
                ClientState.m_IncommingData.RemoveRange(0, ClientState.m_nextPackageSize);
                ClientState.m_StateObjectListenState = CurrentState.WatingForResponceHeader;
                HandleCommand(command);
            }
            else
            {
                throw new Exception("Only Command should get here!!");
            }
        }
    }
}
