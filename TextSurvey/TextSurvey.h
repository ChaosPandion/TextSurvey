#include <functional>
#include <memory>
#include <vector>

using namespace std;

namespace TextSurvey
{
    struct Ignore {

    };

    template<typename T> 
    struct Result {
        enum : char {
            Success,
            Failure
        } Type;
        T Value;

        inline bool IsSuccess() {
            return Type == Success;
        }

        inline bool IsFailure() {
            return Type == Failure;
        }
    };

    struct State {
        char* Current;
        size_t Remaining;
        size_t Length;
        inline auto Remember() -> tuple<char*, size_t> {
            return make_tuple(Current, Remaining);
        }
    };

    template<typename T> 
    inline auto pSuccess(T value) -> Result<T> {
        Result<T> r;
        r.Type = Result<T>::Success;
        r.Value = value;
        return r;
    }

    template<typename T> 
    inline auto pFailure() -> Result<T> {
        Result<T> r;
        r.Type = Result<T>::Failure;
        return r;
    }
    
    template<typename T> 
    auto pZero() -> function<Result<T>(State*)> {
        return [] (State* s) -> Result<T> {
            return pFailure<T>();
        };
    }
    
    template<typename T> 
    auto pReturn(T value) -> function<Result<T>(State*)> {
        return [value] (State* s) -> Result<T> {
            return Success(value);
        };
    }
    
    template<typename T, typename U> 
    auto pBind(function<Result<T>(State*)> p, function<function<Result<U>(State*)>(T)> f) -> function<Result<U>(State*)> {
        return [p, f] (State* s) -> Result<T> {
            Result<T> r = p(s);
            if (r.IsFailure())
                return pFailure<U>();
            auto n = f(r.Value);
            return n(s);
        };
    }  
    
    template<typename T, typename U> 
    auto pMap(function<Result<T>(State*)> p, function<U(T)> f) -> function<Result<U>(State*)> {
        return [p, f] (State* s) -> Result<U> {
            Result<T> r = p(s);
            if (r.IsFailure())
                return pFailure<U>();
            U mapped = f(r.Value);
            return pSuccess<U>(mapped);
        };
    }  
    
    template<typename T> 
    auto pChoice(
        function<Result<T>(State*)> choice1, 
        function<Result<T>(State*)> choice2) -> function<Result<T>(State*)> {
        return [choice1, choice2] (State* s) -> Result<T> {
            auto r = choice1(s);
            if (r.IsSuccess())
                return r;
            return choice2(s);
        };
    }
    
    template<typename T> 
    auto pChoice(
        function<Result<T>(State*)> choice1, 
        function<Result<T>(State*)> choice2, 
        function<Result<T>(State*)> choice3) -> function<Result<T>(State*)> {
        return [choice1, choice2, choice3] (State* s) -> Result<T> {
            auto result1 = choice1(s);
            if (result1.IsSuccess())
                return result1;
            auto result2 = choice2(s);
            if (result2.IsSuccess())
                return result2;
            return choice3(s);
        };
    }   
    
    template<typename T> 
    auto pChoice(
        function<Result<T>(State*)> choice1, 
        function<Result<T>(State*)> choice2, 
        function<Result<T>(State*)> choice3, 
        function<Result<T>(State*)> choice4) -> function<Result<T>(State*)> {
        return [choice1, choice2, choice3, choice4] (State* s) -> Result<T> {
            auto result1 = choice1(s);
            if (result1.IsSuccess())
                return result1;
            auto result2 = choice2(s);
            if (result2.IsSuccess())
                return result2;
            auto result3 = choice3(s);
            if (result3.IsSuccess())
                return result3;
            return choice4(s);
        };
    } 

    template<typename Left, typename Center, typename Right> 
    auto pBetween(
        function<Result<Left>(State*)> left, 
        function<Result<Center>(State*)> center, 
        function<Result<Right>(State*)> right) -> function<Result<Center>(State*)> {
        return [left, center, right] (State* s) -> Result<Center> {
            auto initialState = s->Remember();
            Result<Left> lResult = left(s);
            if (lResult.IsSuccess())
            {
                Result<Center> cResult = center(s);
                if (cResult.IsSuccess())
                {
                    Result<Right> rResult = right(s);
                    if (rResult.IsSuccess())
                        return pSuccess<Center>(cResult.Value);
                }
                s->Current = get<0>(initialState);
                s->Remaining = get<1>(initialState);
            }
            return pFailure<Center>();
        };
    }

    template<typename T> 
    auto pMany(function<Result<T>(State*)> parser) -> function<Result<vector<T>>(State*)> {
        return [parser] (State* s) -> Result<vector<T>> {
            vector<T> results;
            do {
                Result<T> r1 = parser(s);
                if (r1.IsFailure()) {
                    return pSuccess(results);
                }
                results.push_back(r1.Value);
            } while (true);
        };
    }
    
    template<typename T> 
    auto pSkipMany(function<Result<T>(State*)> parser) -> function<Result<Ignore>(State*)> {
        return [parser] (State* s) -> Result<Ignore> {
            Ignore ig;
            while (true) {
                auto r = parser(s);
                if (r.IsFailure()) {
                    return pSuccess(ig);
                }
            }
        };
    }

    template<typename T, int min> 
    auto pMany(function<Result<T>(State*)> parser) -> function<Result<vector<T>>(State*)> {
        return [parser] (State* s) -> Result<vector<T>> {
            auto cp = s->Current;
            auto cr = s->Remaining;
            vector<T> results;
            do {
                auto r1 = parser(s);
                if (r1.IsFailure()) {
                    if (results.size() < min) {
                        s->Current = cp;
                        s->Remaining = cr;
                        return pFailure<vector<T>>();
                    }
                    return pSuccess(results);
                }
                results.push_back(r1.Value);
            } while (true);
        };
    }
    
    template<typename T, typename U> 
    auto pManySeparated(
        function<Result<T>(State*)> parser, 
        function<Result<U>(State*)> separator) -> function<Result<vector<T>>(State*)> {
        return [parser, separator] (State* s) -> Result<vector<T>> {
            vector<T> results;
            do {
                auto r1 = parser(s);
                if (r1.IsFailure()) {
                    return pSuccess(results);
                }
                results.push_back(r1.Value);
                auto r2 = separator(s);
                if (r2.IsFailure()) {
                    return pSuccess(results);
                }
            } while (true);
        };
    }
    
    template<char c>
    auto pChar() -> function<Result<char>(State*)> {
        return [] (State* s) -> Result<char> {
            if (s->Remaining == 0 || *s->Current != c)
                return pFailure<char>();
            s->Current++;
            s->Remaining--;
            return pSuccess(c);
        };
    }
    
    template<char minInclusive, char maxInclusive>
    auto pChar() -> function<Result<char>(State*)> {
        static_assert(minInclusive < maxInclusive, "minInclusive < maxInclusive");
        return [] (State* s) -> Result<char> {
            if (s->Remaining == 0)
                return pFailure<char>();
            char c = *s->Current;
            if (c < minInclusive || c < maxInclusive)
                return pFailure<char>();
            s->Current++;
            s->Remaining--;
            return pSuccess(c);
        };
    }

    auto pChar(const string& values) -> function<Result<char>(State*)> {
        return [values] (State* s) -> Result<char> {
            for (auto c : values) {
                if (*s->Current == c) {
                    s->Current++;
                    s->Remaining--;
                    return pSuccess<char>(c);
                }
            }
            return pFailure<char>();
        };
    } 

    auto pString(const string& test) -> function<Result<string>(State*)> {
        return [test] (State* s) -> Result<string> {
            if (s->Remaining < test.length())
                return pFailure<string>();
            auto cp = s->Current;
            for (auto c : test) {
                if (*cp++ != c) {
                    return pFailure<string>();
                }
            }
            s->Remaining -= test.length();
            s->Current = cp;
            return pSuccess<string>(test);
        };
    } 
    
    template<typename T>
    auto PowerOfTen(T power) -> T {
        const int cacheSize = 100;
        static vector<T> cache(cacheSize);
        if (power <= 0)
            return 1;
        T v;
        if (power > cacheSize) {
            v = 1;
            for (int i = 0; i < power; i++)
                v *= 10;
            return v;
        } else {
            v = cache[power];
            if (v > 0)
                return v;
            v = 1;
            for (int i = 0; i < power; i++)
                v *= 10;
            return cache[power] = v;
        }
    }

    template<typename T>
    auto pUIntBase10() -> function<Result<T>(State*)> {
        return [] (State* s) -> Result<T> {
            auto cp = s->Current;
            auto remain = s->Remaining;
            int count = 0;
            T result = 0;
            while (true) {
                if (remain == 0) {
                    break;
                }
                char c = *cp;
                if (c < '0' || c > '9') {
                    break;
                }
                result *= 10;
                result += (c - '0');
                cp++;
                count++;
                remain--;
            }
            if (count > 0) {
                s->Current = cp;
                s->Remaining -= count;
                return pSuccess(result);
            }
            return pFailure<T>();
        };
    }  
    
    template<typename T> 
    auto RunParser(string text, function<Result<T>(State*)> parser) -> Result<T> {
        State s;
        s.Current = (char*)text.c_str();
        s.Length = s.Remaining = text.length();
        return parser(&s);
    };
}



//template<typename TokenType, typename UserStateType> 
//class ParseState {
//public:
//    TokenType *current;
//    TokenType *end;
//    UserStateType *userState;
//    ParseState(TokenType *data, size_t length, UserStateType *userState_) 
//        : current(data), end(data + length), userState(userState_)
//    {
//
//    }
//    bool Peek(TokenType *token)
//    {
//        if (current == end)
//            return false;
//        *token = *current;
//        return true;
//    }
//    bool Next()
//    {
//        if (current == end)
//            return false;
//        current++;
//        return true;
//    }
//};
//
//enum ParseResultType {
//    Success,
//    Failure
//};
//
//template<typename ResultType, typename UserStateType> 
//class ParseResult {
//public:
//
//    ParseResultType type;
//    ResultType result;
//    UserStateType *userState;
//
//    ParseResult(ParseResultType type_, ResultType result_, UserStateType *userState_)
//        : type(type_), result(result_), userState(userState_)
//    {
//
//    }
//};
//
//template<typename ResultType, typename TokenType, typename UserStateType> 
//class Parse {
//public:
//
//    static auto Run(
//            TokenType *input, 
//            size_t length, 
//            function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> parser, 
//            UserStateType *userState
//        ) -> ParseResult<ResultType, UserStateType> 
//    {
//        ParseState<TokenType, UserStateType> s(input, length, userState);
//        return parser(&s);
//    }
//
//    static auto Delay(
//        function<function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)>()> f) 
//            -> function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> {
//        return [f] (ParseState<TokenType, UserStateType>* s) -> ParseResult<ResultType, UserStateType> { 
//            auto continuation = f();
//            return continuation(s);
//        };
//    }
//
//    static auto Zero() -> function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> {
//        return [] (ParseState<TokenType, UserStateType>* s) -> ParseResult<ResultType, UserStateType> { 
//            return ParseResult<ResultType, UserStateType>(Failure, nullptr, s->userState);
//        };
//    }
//
//    static auto Return(ResultType* v) -> function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> {
//        return [v] (ParseState<TokenType, UserStateType>* s) -> ParseResult<ResultType, UserStateType> { 
//            return ParseResult<ResultType, UserStateType>(Success, v, s->userState);
//        };
//    }
//
//    template<typename T> 
//    static auto Bind(
//            function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> parser, 
//            function<function<ParseResult<T, UserStateType>(ParseState<TokenType, UserStateType>*)>(ResultType)> continuation
//        ) -> function<ParseResult<T, UserStateType>(ParseState<TokenType, UserStateType>*)> 
//    {
//        return [parser, continuation] (ParseState<TokenType, UserStateType>* s) -> ParseResult<T, UserStateType> { 
//            ParseResult<ResultType, UserStateType> r = parser(s);
//            if (r.type == ParseResultType::Failure)
//                return ParseResult<T, UserStateType>(Failure, nullptr, r.userState);
//            auto next = continuation(r.result);
//            return next(s);
//        };
//    }
//    
//    template<TokenType test> 
//    static auto pToken() -> function<ParseResult<ResultType, UserStateType>(ParseState<TokenType, UserStateType>*)> {
//        return [] (ParseState<TokenType, UserStateType>* s) -> ParseResult<ResultType, UserStateType> { 
//            TokenType t;
//            if (!s->Peek(&t) || t != test) 
//            {
//                return ParseResult<ResultType, UserStateType>(Failure, nullptr, s->userState);
//            }
//            s->Next();
//            return ParseResult<ResultType, UserStateType>(Success, t, s->userState);
//        };
//    }
//};
//    
//template<typename T, typename S> 
//static auto pString(const T* test) 
//        -> function<ParseResult<const T*, S>(ParseState<T, S>*)> {
//    return [test] (ParseState<T, S>* s) -> ParseResult<const T*, S> {
//        auto sp = s->current;
//        T t;
//        for (T c : test) 
//        {
//            if (!s->Peek(&t) || t != c) 
//            {
//                s->current = sp;
//                return ParseResult<string, S>(Failure, nullptr, s->userState);
//            }
//            s->Next();
//        }
//        return ParseResult<string, S>(Success, test, s->userState);
//    };
//}
//
//template<typename R, typename T, typename S> 
//static auto pChoice(
//    function<ParseResult<R, S>(ParseState<T, S>*)> choice1,
//    function<ParseResult<R, S>(ParseState<T, S>*)> choice2) 
//        -> function<ParseResult<R, S>(ParseState<T, S>*)> {
//    return [choice1, choice2] (ParseState<T, S>* s) -> ParseResult<R, S> {
//        auto r = choice1(s);
//        if (r.type == Success)
//            return r;
//        return choice2(s);
//    };
//}




//template<typename T>
//class ForwardIterator : 
//    public iterator<forward_iterator_tag, T>
//{
//private:
//    const T* start;
//    const T* end;
//    T* current;
//public:
//    ForwardIterator(const T* start, const T* end) 
//        : start(start), 
//        end(end),
//        current((T*)start)
//    {
//
//    }
//
//    ForwardIterator(const ForwardIterator& other) 
//        : start(other.start), 
//        end(other.end),
//        current(other.current)
//    {
//
//    }
//
//    bool End()
//    {
//        return current == end;
//    }
//
//    ForwardIterator& operator++() 
//    {
//        if (current < end)
//            ++current;
//        return *this;
//    }
//
//    ForwardIterator operator++(int) {
//        ForwardIterator tmp(*this); 
//        operator++(); 
//        return tmp;
//    }
//
//    T& operator*() 
//    {
//        return *current;
//    }
//};
//
//
//template<typename TokenType, typename UserStateType>
//class State {
//public:
//    unique_ptr<ForwardIterator<TokenType>> Iterator;
//    unique_ptr<UserStateType> UserState;
//
//    State(unique_ptr<ForwardIterator<TokenType>> iterator, unique_ptr<UserStateType> userState) : 
//        Iterator(move(iterator)), 
//        UserState(move(userState))
//    {
//
//    }
//};
//
//enum ResultType {
//    Success,
//    Failure
//};
//
//template<typename OutputType, typename UserStateType>
//class Result {
//public:
//    ResultType Type;
//    OutputType* Output;
//    unique_ptr<UserStateType> UserState;
//
//    Result(ResultType type, OutputType* output, unique_ptr<UserStateType> userState)
//        : Type(type), Output(output), UserState(move(userState))
//    {
//
//    }
//};
//
//template<typename OutputType, typename Token, typename UserState>
//class pReturn
//{
//private:
//    OutputType* _output;
//public:
//
//    pReturn(OutputType* output)
//        : _output(output)
//    {
//
//    }
//
//    unique_ptr<Result<OutputType, UserState>> operator()(unique_ptr<State<Token, UserState>> state)
//    {
//        Result<OutputType, UserState>* r = new Result<OutputType, UserState>(Success, _output, move(state->UserState));
//        return unique_ptr<Result<OutputType, UserState>>(r);
//    }
//};
//
//
////template<typename Output, typename Token, typename UserState>
////auto pReturn(Output v) -> function<Result<Output, UserState>(State<Token, UserState>)>
////{
////    return [&v] (State<Token, UserState> s) -> Result<Output, UserState> { 
////        return Result<Output, UserState>(Success, &v, s.UserState);
////    };
////}
//
//
//template<typename Output, typename Token, typename UserState>
//auto pZero() -> function<Result<Output, UserState>(unique_ptr<State<Token, UserState>>)>
//{
//    return [] (unique_ptr<State<Token, UserState>> s) -> Result<Output, UserState> { 
//        return Result<Output, UserState>(Failure, nullptr, s.UserState);
//    };
//}
//
//
//template<typename R1, typename R2, typename Token, typename UserState>
//auto pBind(
//    const function<Result<R1, UserState>(unique_ptr<State<Token, UserState>>)>& parser,
//    const function<function<Result<R2, UserState>(unique_ptr<State<Token, UserState>>)>(R1*)>& continuation
//    ) -> function<Result<R2, UserState>(unique_ptr<State<Token, UserState>>)>
//{
//    return [&parser, &continuation] (unique_ptr<State<Token, UserState>> s) -> Result<R2, UserState> { 
//        Result<R1, UserState> r = parser(s);
//        if (r.Type == Failure)
//        {
//            return Result<R2, UserState>(Failure, nullptr, s.UserState);
//        }
//        R1 v = *r.Output;
//        function<Result<R2, UserState>(unique_ptr<State<Token, UserState>>)> next = continuation(&v);
//        Result<R2, UserState> va = next(s);
//        return va;
//    };
//}
//
//template<typename R, R test, typename UserState>
//auto One() -> function<Result<R, UserState>(State<R, UserState>)>
//{
//    return [] (unique_ptr<State<R, UserState>> s) -> Result<R, UserState> {
//        if (s.Iterator->End()) {
//            return Result<R, UserState>(Failure, nullptr, s.UserState);
//        }
//        R& v = **s.Iterator;
//        if (v != test) {
//            return Result<R, UserState>(Failure, nullptr, s.UserState);
//        }
//        s.Iterator++;
//        return Result<R, UserState>(Success, &v, s.UserState);
//    };
//}
//
//template<typename Input, typename Output, typename UserState>
//auto Run(
//    unique_ptr<ForwardIterator<Input>> input, 
//    unique_ptr<UserState> userState, 
//    function<unique_ptr<Result<Output, UserState>>(unique_ptr<State<Input, UserState>>)> parser)
//        -> unique_ptr<Result<Output, UserState>>
//{
//    State<Input, UserState>* state = new State<Input, UserState>(std::move(input), std::move(userState));
//    unique_ptr<State<Input, UserState>> statePtr(state);
//    auto result = parser(std::move(statePtr));
//    return result;
//}

//template<typename TChar, typename TResult> 
//class Parser {
//public:
//    virtual const Parser<TChar, TResult>& operator >>=(const Parser<TChar, TResult>& input);
//    virtual const ParseState<TChar, TResult>& operator ()(const ParseState<TChar, TResult>& input);
//};