using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
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
using webX3._0.Controls;
using webX3._0.Pages;

namespace webX3._0
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public static bool _active = false;
        public static bool _online = false;
        public static ServerConsole Svcon = new ServerConsole();

        public MainWindow()
        {
            InitializeComponent();

            Thread _DT = new Thread(new ThreadStart(DataTransit.Start));
            _DT.SetApartmentState(ApartmentState.STA);
            _DT.Start();
        }

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void Pb_click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void Adminc(object sender, RoutedEventArgs e)
        {
            
        }

        private void Con_click(object sender, RoutedEventArgs e)
        {
            if (_active == false) {
                Svcon.Show();

                _active = true;
            } else if (_active == true) {
                Svcon.Hide();

                _active = false;
            }
        }

        private void MoveCursorMenu(int index)
        {
            TrainsitionigContentSlide.OnApplyTemplate();
            GridCursor.Margin = new Thickness(0, (100 + (60 * index)), 0, 0);
        }

        private void ListViewMenu_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            int index = ListViewMenu.SelectedIndex;
            MoveCursorMenu(index);

            switch (index)
            {
                case 0:
                    GridPrincipal.Children.Clear();
                    GridPrincipal.Children.Add(new Pages.Server());
                    break;
                case 1:
                    GridPrincipal.Children.Clear();
                    GridPrincipal.Children.Add(new Pages.Settings());
                    break;
                case 2:
                    GridPrincipal.Children.Clear();
                    GridPrincipal.Children.Add(new Pages.Console());
                    break;
                case 3:
                    GridPrincipal.Children.Clear();
                    GridPrincipal.Children.Add(new Pages.DBManager());
                    break;
                case 4:
                    break;
                case 5:
                    GridPrincipal.Children.Clear();
                    GridPrincipal.Children.Add(new Pages.AboutWebxlib());
                    break;
                default:
                    break;
            }
        }
    }
}
