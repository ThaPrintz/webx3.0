using System;
using System.Collections.Generic;
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

namespace webX3._0.Pages
{
    /// <summary>
    /// Interaction logic for Server.xaml
    /// </summary>
    public partial class Server : UserControl
    {
        public Server()
        {
            InitializeComponent();

            if (MainWindow._online)
            {
                sv_status.Text = "ONLINE";
                sv_status.Foreground = Brushes.SpringGreen;
            }
        }

        public void LoadWebxFunc(object sender, RoutedEventArgs e)
        {
            webx_check.Foreground = Brushes.SpringGreen; 
        }

        public void StartWebSv(object sender, RoutedEventArgs e)
        {
            MainWindow.Svcon.Cprintf("Server launched");

            sv_status.Text = "ONLINE";
            sv_status.Foreground = Brushes.SpringGreen;

            webxcore.StartWebx3();

            MainWindow._online = true;
        }

        public void StopWebSv(object sender, RoutedEventArgs e)
        {
            MainWindow.Svcon.Cprintf("Server stopped");

            sv_status.Text = "OFFLINE";
            sv_status.Foreground = Brushes.IndianRed;

            webxcore.StopWebx3();
        }
    }
}
