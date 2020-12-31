using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace webX3._0
{
    public class webxcore
    {
        [DllImport("webxcore.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void StartWebServer();

        [DllImport("webxcore.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void StopWebServer();

        public static void StartWebx3()
        {
            Thread webserver_thread = new Thread(new ThreadStart(StartWebServer));
            webserver_thread.Start();
        }

        public static void StopWebx3()
        {
            StopWebServer();
        }
    }
}
