using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Runtime.CompilerServices;

namespace webX3._0
{
    class DataTransit
    {
        public static string data = null;
        
        public static void cPrint(String std)
        {
            MainWindow.Svcon.Dispatcher.Invoke(() =>
            {
                MainWindow.Svcon.Cprintf(std);
            });
        }

        public static void Start()
        {
            byte[] bytes = new Byte[1024];

            IPAddress ipAdress = IPAddress.Parse("0.0.0.0");
            IPEndPoint localEndPoint = new IPEndPoint(ipAdress, 70);

            Socket listener = new Socket(ipAdress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                listener.Bind(localEndPoint);
                listener.Listen(20);

                while (true)
                {
                    Socket handler = listener.Accept();
                    data = null;

                    int bytesRec = handler.Receive(bytes);
                    data += Encoding.ASCII.GetString(bytes, 0, bytesRec);
                    cPrint(data);
                }
            } catch (Exception e) {
                cPrint(e.ToString());
            }
        }
    }
}
