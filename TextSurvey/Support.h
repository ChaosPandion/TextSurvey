#pragma once

#include "Common.h"

namespace TextSurvey 
{
    struct Range
    {
        const uint Min;
        const uint Max;

        Range() :
            Min(0u), Max(0u)
        {

        }

        Range(const uint value) :
            Min(value), Max(value)
        {

        }

        Range(const uint min, const uint max) :
            Min(min), Max(max)
        {

        }
    };
    
    static const Range& ZeroOrMore = Range(0u, 0u);
    static const Range& OneOrMore = Range(1u, 0u);

    inline bool InRange(const Range range, const uint n)
    {
        return (range.Min == 0u || n >= range.Min) && (range.Max == 0u || n <= range.Max);
    }
    
    template<typename T>
    class Option 
    {
    private:

        bool _isSome;
        T _value;

    public:

        Option() : 
            _isSome(false)
        {

        }

        Option(T value) : 
            _isSome(true), 
            _value(value)
        {

        }

        Option(const T& value) : 
            _isSome(true), 
            _value(value)
        {

        }

        Option(const Option& other) : 
            _isSome(other._isSome), 
            _value(other._value)
        {

        }

        inline bool IsSome()
        {
            return _isSome;
        }

        inline const T& GetValue()
        {
            return T;
        }
    };

    template<typename T>
    inline const Option<T>& Some(T value)
    {
        return Option<T>(value);
    }

    template<typename T>
    inline const Option<T>& Some(const T& value)
    {
        return Option<T>(value);
    }

    template<typename T>
    inline const Option<T>& None()
    {
        return Option<T>();
    }
}