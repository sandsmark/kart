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

            var rdGen = new Random(1337);
            var interop = new InteropService(-1, address, port);
            var map = interop.QueryForState<MapState>().Result;
            interop.Send(0); // Ask for game state update

            while (true)
            {
                var gameState = interop.QueryForState<GameState>().Result;

                var nextMove = rdGen.Next() % 5;
                var moveInByte = 1 << nextMove;
                interop.Send((byte)moveInByte);
            }
        }
    }
}
