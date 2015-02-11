using System.Collections.Generic;
using Newtonsoft.Json;

namespace Interop
{
    public class GameState
    {
        [JsonProperty(PropertyName = "cars")]
        public List<CarData> Cars;

        [JsonProperty(PropertyName = "shells")]
        public List<ShellData> Shells;

        [JsonProperty(PropertyName = "boxes")]
        public List<BoxData> Boxes;
    }
}