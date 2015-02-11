using System;
using System.Reflection;
using Newtonsoft.Json;

namespace Interop
{
    [JsonConverter(typeof(MapStringEnumConverter))]
    public enum TileType
    {
        Empty,
        VerticalStraight,
        HorizontalStraight,
        RightCorner,
        LeftCorner,
        RightFlippedCorner,
        LeftFlippedCorner,
    }

    public class MapStringEnumConverter : JsonConverter
    {
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }

        static string ToEnumName(Type enumType, string enumName)
        {
            // Some reflection hacks
            var newtonAssembly = Assembly.GetAssembly(typeof(JsonConverter));
            var type = newtonAssembly.GetType("Newtonsoft.Json.Utilities.EnumUtils");
            var method = type.GetMethod("ToEnumName");
            return (string)method.Invoke(null, new object[] { enumType, enumName, false });
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var t = objectType;

            if (reader.TokenType != JsonToken.String)
                throw new Exception(string.Format((string)"Unexpected token {0} when parsing enum.", (object)reader.TokenType));

            var enumText = reader.Value.ToString();
            switch (enumText)
            {
                case ".":
                    enumText = "Empty";
                    break;
                case "`":
                    enumText = "RightCorner";
                    break;
                case "/":
                    enumText = "RightFlippedCorner";
                    break;
                case "\\":
                    enumText = "LeftFlippedCorner";
                    break;
                case ",":
                    enumText = "LeftCorner";
                    break;
                case "|":
                    enumText = "VerticalStraight";
                    break;
                case "-":
                    enumText = "HorizontalStraight";
                    break;
            }

            return ParseEnumName(enumText, t);
        }

        static object ParseEnumName(string enumText, Type type)
        {
            // Some reflection hacks
            var newtonAssembly = Assembly.GetAssembly(typeof(JsonConverter));
            var enumUtilstype = newtonAssembly.GetType("Newtonsoft.Json.Utilities.EnumUtils");
            var method = enumUtilstype.GetMethod("ParseEnumName");
            return method.Invoke(null, new object[] { enumText, false, type });
        }

        public override bool CanConvert(Type objectType)
        {
            return objectType.IsEnum;
        }
    }
}