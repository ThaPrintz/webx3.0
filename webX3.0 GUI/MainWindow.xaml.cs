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
using webX3._0.Controls;

namespace webX3._0
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow() => InitializeComponent();

        public static ServerConsole Svcon;

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void Pb_click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void Con_click(object sender, RoutedEventArgs e)
        {
            Svcon = new ServerConsole();
            Svcon.Show();
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

                    Pages.Console conpage       = new Pages.Console();

                    GridPrincipal.Children.Add(conpage);
                    break;
                default:
                    break;
            }
        }
    }
}
