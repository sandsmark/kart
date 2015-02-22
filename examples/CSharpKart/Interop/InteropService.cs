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

        public InteropService(string botName, int connectTries, string address, int port)
        {
            Connect(botName, connectTries, address, port);            
        }

        void Connect(string botName, int connectTries, string address, int port)
        {
            var curTry = 0;
            while (connectTries == -1 || (++curTry) <= connectTries)
            {
                try
                {
                    _client = new TcpClient(address, port);

                    var msgInBytes = Encoding.ASCII.GetBytes(botName);
                    _client.GetStream().Write(msgInBytes, 0, msgInBytes.Length); 
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
            var msgInBytes = Encoding.ASCII.GetBytes(msg);
            _client.GetStream().Write(msgInBytes, 0, msgInBytes.Length);
        }
    }
}