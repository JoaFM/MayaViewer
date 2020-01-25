using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Viewer_Server.Clients;

namespace Viewer_Server
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        // Create the OnPropertyChanged method to raise the event
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
        private ClientManager ClientMan = null;

        public ClientManager m_ClientMan
        {
            get
            {
                return ClientMan;
            }
            set
            {
                ClientMan = value;
                OnPropertyChanged("m_ClientMan");
            }   
        }

        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        private void Button1_Click(object sender, RoutedEventArgs e)
        {
            m_ClientMan = new ClientManager();
            m_ClientMan.StartServer();
            bt_Connect.Content = "STARTED";
            bt_Connect.IsEnabled = false;
        }

        private void Bt_refresh_Click(object sender, RoutedEventArgs e)
        {
        }
    }
}
