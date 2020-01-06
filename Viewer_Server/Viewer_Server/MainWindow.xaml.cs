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
    public partial class MainWindow : Window
    {

        private ClientManager ClientMan = null;

    
        public MainWindow()
        {
            InitializeComponent();
           
        }

        private void Button1_Click(object sender, RoutedEventArgs e)
        {
            ClientMan = new ClientManager();

            ClientMan.StartServer();
            bt_Connect.Content = "Server Started";
        }

        private void Bt_refresh_Click(object sender, RoutedEventArgs e)
        {
            tb_numCleint.Text = ClientMan.NumberOfClients.ToString();
        }
    }
}
