using System;
using System.Reflection;
using Newtonsoft.Json;

namespace Interop
{
    public class ServerStringEnumConverter : JsonConverter
    {
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            if (value == null)
            {
                writer.WriteNull();
                return;
            }

            var e = (Enum)value;
            var enumName = e.ToString("G");

            if (char.IsNumber(enumName[0]) || enumName[0] == '-')
            {
                writer.WriteValue(value);
            }
            else
            {
                var enumType = e.GetType();
                var finalName = ToEnumName(enumType, enumName).ToLower();
                writer.WriteValue(finalName);
            }
        }

        static string ToEnumName(Type enumType, string enumName)
        {
            // Some reflection hacks
            var newtonAssembly = Assembly.GetAssembly(typeof (JsonConverter));
            var type = newtonAssembly.GetType("Newtonsoft.Json.Utilities.EnumUtils");
            var method = type.GetMethod("ToEnumName");
            return (string)method.Invoke(null, new object[] { enumType, enumName, false });
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var t = objectType;

            if (reader.TokenType != JsonToken.String)
                throw new Exception(string.Format((string) "Unexpected token {0} when parsing enum.", (object) reader.TokenType));
            
            var enumText = reader.Value.ToString();
            enumText = enumText.Substring(0, 1).ToUpper() + enumText.Substring(1);
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