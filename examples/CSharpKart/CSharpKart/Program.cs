using System;
using Interop;

namespace CSharpKart
{
    class Program
    {
        static void Main(string[] args)
        {
            var address = "127.0.0.1";
            var port = 31337;

            if (args.Length >= 2)
            {
                address = args[0];
                port = int.Parse(args[1]);
            }

            try
            {
                var rdGen = new Random(1337);
                var interop = new InteropService("CSharpKart", -1, address, port);
                var map = interop.QueryForState<GameInfo>().Result;
                interop.Send(0); // Ask for game state update

                while (true)
                {
                    var gameState = interop.QueryForState<GameState>().Result;

                    var nextMove = rdGen.Next() % 5;
                    var moveInByte = 1 << nextMove;
                    interop.Send((byte) moveInByte);
                }
            }
            catch (Exception e)
            {
                var aggregateException = e as AggregateException;
                if (aggregateException != null)
                {
                    foreach(var innerException in aggregateException.InnerExceptions)
                        Console.WriteLine(innerException.Message);
                }
                else
                    Console.WriteLine(e.Message);
            }
        }
    }
}
