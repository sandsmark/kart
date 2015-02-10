using System.Collections.Generic;
using Newtonsoft.Json;

namespace Interop
{
    public class CarData
    {
        [JsonProperty(PropertyName = "id")]
        public int Id;

        [JsonProperty(PropertyName = "direction")]
        public VectorData Direction;

        [JsonProperty(PropertyName = "velocity")]
        public VectorData Velocity;

        [JsonProperty(PropertyName = "pos")]
        public VectorData Pos;

        [JsonProperty(PropertyName = "drift")]
        public int Drift;

        [JsonProperty(PropertyName = "width")]
        public int Width;

        [JsonProperty(PropertyName = "height")]
        public int Height;
    }

    public class BoxData
    {
        [JsonProperty(PropertyName = "x")]
        public double X;

        [JsonProperty(PropertyName = "y")] 
        public double Y;

        [JsonProperty(PropertyName = "width")] 
        public int Width;

        [JsonProperty(PropertyName = "height")] 
        public int Height;
    }
}