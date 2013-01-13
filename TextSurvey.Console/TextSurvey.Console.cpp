// TextSurvey.Console.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "../TextSurvey/TextSurvey.h"
#include "../TextSurvey/Json.h"
//using namespace TextSurvey;
//
//
//namespace TLisp
//{
//
//    struct Node {
//        enum {
//            Invalid,
//            Bool,
//            Int32,
//            Name,
//            List
//        } Type;
//        union {
//            bool BoolValue;   
//            uint32_t Int32Value;
//            string* NameValue;
//            vector<Node*>* ListValue;
//        } Value;
//
//        ~Node() {
//            if (Type == Name && Value.NameValue != nullptr)
//                delete Value.NameValue;
//            if (Type != List || Value.ListValue == nullptr)
//                return;
//            for (auto p = Value.ListValue->begin(); p < Value.ListValue->end(); p++)
//                delete *p;
//            delete Value.ListValue;
//        }
//
//        static auto Create(bool v) -> Node* {
//            Node* n = new Node;
//            n->Type = Bool;
//            n->Value.BoolValue = v;
//            return n;
//        }
//
//        static auto Create(uint32_t v) -> Node* {
//            Node* n = new Node;
//            n->Type = Int32;
//            n->Value.Int32Value = v;
//            return n;
//        }
//
//        static auto Create(vector<char> v) -> Node* {
//            Node* n = new Node;
//            n->Type = Name;
//            n->Value.NameValue = new string(v.begin(), v.end());
//            return n;
//        }
//
//        static auto Create(vector<Node*> v) -> Node* {
//            Node* n = new Node;
//            n->Type = List;
//            n->Value.ListValue = new vector<Node*>(v);
//            return n;
//        }
//    };
//
//    
//    static function<Result<Ignore>(State*)> WhiteSpaceParser = pSkipMany<char>(pChar(" \t\r\n"));  
//    
//    typedef function<Result<Node*>(State*)> NodeParserType;
//
//    static NodeParserType BoolParser = 
//        pChoice<Node*>(
//            pMap<string, Node*>(
//                pString("true"), 
//                [] (string s) { return Node::Create(true); }), 
//            pMap<string, Node*>(
//                pString("false"), 
//                [] (string s) { return Node::Create(false); })); 
//
//    static NodeParserType Int32Parser = 
//        pMap<uint32_t, Node*>(
//            pUIntBase10<uint32_t>(), 
//            [] (uint32_t v) { return Node::Create(v); });
//    
//    static const Range<char> nameRanges[2] = { { 'a', 'z' }, { 'A', 'Z' } };
//    static NodeParserType NameParser = 
//        pMap<vector<char>, Node*>(
//            pMany<char, 1>(pChar(nameRanges, 2)), 
//            [] (vector<char> v) { return Node::Create(v); });
//
//    auto GetListParser() -> NodeParserType;
//
//    auto GetWrappedListParser(State* s) -> Result<Node*> {
//        return GetListParser()(s);
//    }
//
//    static const NodeParserType nodeParsers[4] = { 
//        BoolParser, 
//        Int32Parser, 
//        NameParser, 
//        GetWrappedListParser 
//    };
//
//    static NodeParserType NodeParser = pChoice(nodeParsers, 4);
//
//    auto GetListParser() -> NodeParserType
//    {
//        static NodeParserType ListParser = 
//            pMap<vector<Node*>, Node*>(
//                pBetween(pChar<'('>(), pManySeparated(NodeParser, WhiteSpaceParser), pChar<')'>()),
//                [] (vector<Node*> e) { return Node::Create(e); }
//            );
//
//        return ListParser;
//    }    
//
//
//    auto ParseText(string text) -> Node* {
//        Result<Node*> r = RunParser<Node*>(text, NodeParser);
//        if (r.IsFailure())
//            return nullptr;
//        return r.Value;
//    }
//}

void MeasureTime(size_t iterations, function<void(void)> f)
{
    clock_t ct;
    ct = clock();
    for (size_t i = 0; i < iterations; i++)
        f();
    ct = clock() - ct;
    float ctf = (float)ct;
    float ctfs = ctf / (float)CLOCKS_PER_SEC;
    ctfs *= 1000;
    ctfs /= iterations;
    cout << ctfs << "\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
 /*   using namespace TLisp;*/
    using namespace TextSurvey;
    using namespace Json;

    //auto s = State<unit>(AsciiTextStream("true"));
    //auto p = Match<unit>("true");

    //MeasureTime(1000000, [s,p] () {
    //    auto tParser = p(s);
    //    s.Stream.Back(4);
    //});

    getchar();
    return 0;
}

