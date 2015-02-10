using System.Collections.Generic;
using Newtonsoft.Json;

namespace Interop
{
    public class MapState
    {
        [JsonProperty(PropertyName = "tile_width")]
        public int TileWidth;

        [JsonProperty(PropertyName = "tile_height")]
        public int TileHeight;

        [JsonProperty(PropertyName = "tiles")]
        public List<List<char>> Tiles;

        [JsonProperty(PropertyName = "modifiers")]
        public List<ModifierData> Modifiers;

        [JsonProperty(PropertyName = "path")]
        public List<PathData> Path;
    }
}