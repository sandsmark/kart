using Newtonsoft.Json;

namespace Interop
{
    [JsonConverter(typeof(ServerStringEnumConverter))]
    public enum ModifierType
    {
        Mud,
        Booster,
        Ice
    }

    public class ModifierData
    {
        [JsonProperty(PropertyName = "type")]
        public ModifierType Type;

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