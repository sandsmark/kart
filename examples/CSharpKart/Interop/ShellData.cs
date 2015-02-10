using Newtonsoft.Json;

namespace Interop
{
    public enum ShellType
    {
        Blue
    }

    public class ShellData
    {
        [JsonProperty(PropertyName = "type"), JsonConverter(typeof(ServerStringEnumConverter))]
        public ShellType Type;

        [JsonProperty(PropertyName = "x")]
        public double X;

        [JsonProperty(PropertyName = "y")]
        public double Y;

        [JsonProperty(PropertyName = "dx")] 
        public double Dx;

        [JsonProperty(PropertyName = "dy")] 
        public double Dy;
    }
}