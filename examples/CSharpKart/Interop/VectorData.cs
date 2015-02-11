using Newtonsoft.Json;

namespace Interop
{
    public class VectorData
    {
        [JsonProperty(PropertyName = "x")]
        public double X;

        [JsonProperty(PropertyName = "y")]
        public double Y;
    }
}