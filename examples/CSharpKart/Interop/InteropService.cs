using System;
using System.Globalization;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace Interop
{
    public class InteropService : IDisposable
    {
        TcpClient _client;     

        public InteropService(int connectTries, string address, int port)
        {
            Connect(connectTries, address, port);            
        }

        void Connect(int connectTries, string address, int port)
        {
            var curTry = 0;
            while (connectTries == -1 || (++curTry) <= connectTries)
            {
                try
                {
                    _client = new TcpClient(address, port);                    
                    return;
                }
                catch (SocketException e)
                {
                    Console.WriteLine(e.Message);
                    Thread.Sleep(50);
                }
            }
        }

        public async Task<T> QueryForState<T>()
        {
            var stream = _client.GetStream();            
            var streamReader = new StreamReader(stream);

            var readTask = await streamReader.ReadLineAsync();
            var deserializeTask = await Task.Run(() => JsonConvert.DeserializeObject<T>(readTask));

            return deserializeTask;
        }

        public void Dispose()
        {            
            _client.Close();            
        }

        public void Send(byte key)
        {
            var msg = key.ToString(new NumberFormatInfo()) + '\n';
            _client.GetStream().Write(Encoding.ASCII.GetBytes(msg), 0, 1);
        }
    }
}