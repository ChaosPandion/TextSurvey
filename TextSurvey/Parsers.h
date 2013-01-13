#pragma once

#include "Common.h"
#include "Support.h"
#include "TextStream.h"

#define ParserType(R, U) function<Result<R>(State<U>)>

using namespace std;

namespace TextSurvey
{

    typedef nullptr_t unit;

    enum struct ResultCode
    {
        Success, 
        Failure 
    };

    template<typename R> 
    struct Result {
        ResultCode Code;
        R Value;
        // Construct Failure Result
        Result() :
            Code(ResultCode::Failure),
            Value()
        {

        }
        // Construct Success Result
        Result(R value) :
            Code(ResultCode::Success),
            Value(value)
        {

        }
    };

    template<typename U> 
    struct State {
        TextStream& Stream;
        const U* UserState;
        State(TextStream& stream) :
            Stream(stream), UserState(nullptr)
        {

        }
        State(TextStream& stream, const U* userState) :
            Stream(stream), UserState(userState)
        {

        }
    };

    template<typename R, typename U> 
    auto Zero() -> ParserType(R, U)
    {
        return [] (State<U> state) -> Result<R>
        {
            return Result<R>();
        };
    }

    template<typename R, typename U> 
    auto Return(R value) -> ParserType(R, U)
    {
        return [value] (State<U> state) -> Result<R>
        {
            return Result<R>(value);
        };
    }

    template<typename R, typename U> 
    auto Optional(
        function<Result<R>(State<U>)> parser
        ) -> ParserType(Option<R>, U)
    {
        return [parser] (State<U> state) -> Result<Option<R>>
        {
            auto result = parser(state);
            auto value = result.Code == ResultCode::Success ? Some(result.Vale) : None<R>();
            return Result<Option<R>>(value);
        };
    }

    template<typename R1, typename R2, typename U> 
    auto Bind(
        function<Result<R1>(State<U>)> parser, 
        function<function<Result<R2>(State<U>)>(R1)> continuation
        ) -> ParserType(R2, U)
    {
        return [parser, continuation] (State<U> state) -> Result<R2>
        {
            auto r = parser(state);
            if (r.Code == ResultCode::Failure)
                return Result<R2>();
            return continuation(r.Value)(state);
        };
    }

    template<typename R1, typename R2, typename U> 
    auto Sequence(
        function<Result<R1>(State<U>)> parser1, 
        function<Result<R2>(State<U>)> parser2) -> function<Result<tuple<R1, R2>>(State<U>)>
    {
        return [parser1, parser2] (State<U> state) -> Result<tuple<R1, R2>>
        {
            auto snapshot = state.Stream.GetSnapshot();
            auto result1 = parser1(state);
            if (result1.Code == ResultCode::Failure)
                return Result<tuple<R1, R2>>();
            auto result2 = parser2(state);
            if (result2.Code == ResultCode::Failure)
            {
                snapshot.Restore();
                return Result<tuple<R1, R2>>();
            }
            auto resultValues = make_tuple(result1.Value, result2.Value);
            return Result<tuple<R1, R2>>(resultValues);
        };
    }

    template<typename R1, typename R2, typename R3, typename U> 
    auto Sequence(function<Result<R1>(State<U>)> parser1, 
        function<Result<R2>(State<U>)> parser2, 
        function<Result<R3>(State<U>)> parser3) -> function<Result<tuple<R1, R2, R3>>(State<U>)>
    {
        typedef Result<tuple<R1, R2, R3>> Result;
        return [parser1, parser2, parser3] (State<U> state) -> Result
        {
            auto snapshot = state.Stream.GetSnapshot();
            auto result1 = parser1(state);
            if (result1.Code == ResultCode::Failure)
                return Result();
            auto result2 = parser2(state);
            if (result2.Code == ResultCode::Failure)
            {
                snapshot.Restore();
                return Result();
            }
            auto result3 = parser3(state);
            if (result3.Code == ResultCode::Failure)
            {
                snapshot.Restore();
                return Result();
            }
            auto resultValues = make_tuple(result1.Value, result2.Value, result3.Value);
            return Result(resultValues);
        };
    }

    template<typename R, typename U> 
    auto Many(
        function<Result<R>(State<U>)> parser, 
        const Range& range = ZeroOrMore
        ) -> function<Result<vector<R>>(State<U>)>
    {
        typedef Result<vector<R>> Result;
        return [parser, range] (State<U> state) -> Result
        {
            auto snapshot = state.Stream.GetSnapshot();
            vector<R> results;
            auto result = parser(state);
            while (result.Code == ResultCode::Success) 
            {
                if (InRange(range, results.size()))
                {
                    snapshot.Restore();
                    return Result();
                }
                results.push_back(result.Value);
                result = parser(state);
            }
            if (InRange(range, results.size()))
            {
                snapshot.Restore();
                return Result();
            }
            return Result(results);
        };
    }

    template<typename R1, typename R2, typename U> 
    auto Split(
        function<Result<R1>(State<U>)> parser, 
        function<Result<R2>(State<U>)> separatorParser, 
        const Range& range = ZeroOrMore
        ) -> function<Result<vector<R1>>(State<U>)> 
    {
        typedef Result<vector<R1>> Result;
        return [parser, splitter, range] (State<U> state) -> Result
        {
            auto snapshot = state.Stream.GetSnapshot();
            vector<R1> results;
            auto result = parser(state);
            while (result.Code == ResultCode::Success) 
            {
                if (InRange(range, results.size()))
                {
                    snapshot.Restore();
                    return Result();
                }
                results.push_back(result.Value);
                auto separatorResult = separatorParser(state);
                if (separatorResult.Code == ResultCode::Failure)
                    break;
                result = parser(state);
            }
            if (InRange(range, results.size()))
            {
                snapshot.Restore();
                return Result();
            }
            return Result(results);
        };
    }

    template<typename R1, typename R2, typename U> 
    auto Until(
        function<Result<R1>(State<U>)> parser, 
        function<Result<R2>(State<U>)> endParser, 
        const Range& range = ZeroOrMore
        ) -> function<Result<vector<R1>>(State<U>)> 
    {
        typedef Result<vector<R1>> Result;
        return [parser, endParser, range] (State<U> state) -> Result
        {
            auto snapshot = state.Stream.GetSnapshot();
            vector<R1> results;
            auto result = parser(state);
            while (result.Code == ResultCode::Success) 
            {
                if (InRange(range, results.size()))
                {
                    snapshot.Restore();
                    return Result();
                }
                results.push_back(result.Value);
                result = parser(state);
            }
            auto endResult = endParser(state);
            if (endResult.Code == ResultCode::Failure || InRange(range, results.size()))
            {
                snapshot.Restore();
                return Result();
            }
            return Result(results);
        };
    }

    template<typename R, typename U> 
    auto Choice(
        function<Result<R>(State<U>)> parser1, 
        function<Result<R>(State<U>)> parser2
        ) -> function<Result<R>(State<U>)>
    {
        return [parser1, parser2] (State<U> state) -> Result<R>
        {
            auto result1 = parser1(state);
            if (result1.Code == ResultCode::Success)
                return result1;
            return parser2(state);
        };
    }

    template<typename R1, typename R2, typename R3, typename U> 
    auto Between(
        function<Result<R1>(State<U>)> parser1, 
        function<Result<R2>(State<U>)> parser2, 
        function<Result<R3>(State<U>)> parser3
        ) -> function<Result<R2>(State<U>)> 
    {
        return [parser1, parser2, parser3] (State<U> state) -> Result<R2>
        {
            auto snapshot = state.Stream.GetSnapshot();
            auto result1 = parser1(state);
            if (result1.Code == ResultCode::Failure)
                return Result<R2>();
            auto result2 = parser2(state);
            if (result2.Code == ResultCode::Failure)
            {
                snapshot.Restore();
                return Result<R2>();
            }
            auto result3 = parser3(state);
            if (result3.Code == ResultCode::Failure)
            {
                snapshot.Restore();
                return Result<R2>();
            }
            return result2;
        };
    }

    // Char Parsers

    template<typename U>
    auto Match(uchar value) -> ParserType(uchar, U)
    {
        typedef Result<uchar> Result;
        return [value] (State<U> state) -> Result
        {
            uchar c;
            if (state.Stream.Next(&c) == 0u)
            {
                return Result();
            }
            if (c != value)
            {
                state.Stream.Back(1);
                return Result();
            }
            return Result(c);
        };
    }

    template<typename U>
    auto Match(const string& value) -> ParserType(string, U)
    {
        typedef Result<string> Result;
        return [value] (State<U> state) -> Result
        {
            auto snapshot = state.Stream.GetSnapshot();
            auto length = value.length();
            vector<uchar> buffer(length);

            auto readCount = state.Stream.Next(&buffer[0], length);
            if (readCount != length)
            {
                snapshot.Restore();
                return Result();
            }

            for (auto i = 0u; i < length; i++)
            {
                if (buffer[i] != value[i])
                {
                    snapshot.Restore();
                    return Result();
                }
            }
            return Result(value);
        };
    }

    template<typename U>
    auto Satisfy(function<bool(uchar)> predicate) -> ParserType(uchar, U)
    {
        typedef Result<uchar> Result;
        return [predicate] (State<U> state) -> Result
        {
            uchar c;
            if (state.Stream.Next(&c) == 0u)
            {
                return Result();
            }
            if (!predicate(c))
            {
                state.Stream.Back(1);
                return Result();
            }
            return Result(c);
        };
    }
    
    template<typename U>
    auto MatchDecimalDigit(State<U> state) -> Result<uchar>
    {
        uchar c;
        if (state.Stream.Next(&c, 1) != 1 || !(c >= '0' && c <= '9'))
        {
            state.Stream.Back(1);
            return Result<uchar>();
        }
        return Result<uchar>(c);
    }
}