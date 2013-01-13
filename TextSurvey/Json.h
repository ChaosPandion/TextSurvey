#pragma once

#include "TextSurvey.h"

namespace Json
{
    enum struct JsonValueType
    {
        Null,
        String,
        Number,
        Object,
        Array,
        Boolean
    };

    class JsonValue 
    {
    public:
        virtual JsonValueType GetType() const = 0;
    };

    class JsonNumber : 
        public JsonValue
    {
    public:
        double Value;
        JsonValueType GetType() const
        {
            return JsonValueType::Number;
        }
    };

    class JsonBoolean : 
        public JsonValue
    {
    public:
        bool Value;

        JsonValueType GetType() const
        {
            return JsonValueType::Boolean;
        }
    };

    class JsonNull : 
        public JsonValue
    {
    public:
        JsonValueType GetType() const
        {
            return JsonValueType::Null;
        }
    };

    class JsonString : 
        public JsonValue
    {
    public:
        string Value;

        JsonValueType GetType() const
        {
            return JsonValueType::String;
        }
    };

    class JsonObject : 
        public JsonValue
    {
    public:
        map<string, unique_ptr<JsonValue>> Members;
        JsonValueType GetType() const
        {
            return JsonValueType::Object;
        }
    };

    class JsonArray : 
        public JsonValue
    {
    public:
        vector<unique_ptr<JsonValue>> Elements;
        JsonValueType GetType() const
        {
            return JsonValueType::Array;
        }
    };














    auto Parse(string text) -> JsonValue*
    {
        return nullptr;
    }
}