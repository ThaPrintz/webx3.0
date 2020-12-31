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
using System.Windows.Shapes;
using webX3._0.Controls;

namespace webX3._0
{
    /// <summary>
    /// Interaction logic for ServerConsole.xaml
    /// </summary>
    public partial class ServerConsole : Window
    {
        public ServerConsole() => InitializeComponent();

        private void Gmd(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void Pbconclick(object sender, RoutedEventArgs e)
        {
            MainWindow.Svcon.Hide();

            MainWindow._active = false;
        }

        public void Cprintf(String std)
        {
            ConsoleMsg cmsg = new ConsoleMsg();
            cmsg.Msgg.Text = std;

            ConGrid.Children.Add(cmsg);
        }

        public void Cerrf(string std)
        {
            ConsoleError cmsg = new ConsoleError();
            cmsg.Msgg.Text = std;

            ConGrid.Children.Add(cmsg);
        }
    }
}
