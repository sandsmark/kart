using Newtonsoft.Json;

namespace Interop
{
    public class PathData
    {
        [JsonProperty(PropertyName = "tile_x")]
        public int TileX;

        [JsonProperty(PropertyName = "tile_y")]
        public int TileY;
    }
}