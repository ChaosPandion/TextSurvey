// TextSurvey.Console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../TextSurvey/TextSurvey.h"

#include <time.h>
#include <stdint.h>
using namespace std;
using namespace TextSurvey;

typedef unsigned int uint;

namespace TLisp
{
    struct Node {
        enum {
            Invalid,
            Bool,
            Int32,
            Name,
            List
        } Type;
        union {
            bool BoolValue;   
            uint32_t Int32Value;
            string* NameValue;
            vector<Node*>* ListValue;
        } Value;

        ~Node() {
            if (Type == Name && Value.NameValue != nullptr)
                delete Value.NameValue;
            if (Type != List || Value.ListValue == nullptr)
                return;
            for (auto p = Value.ListValue->begin(); p < Value.ListValue->end(); p++)
                delete *p;
            delete Value.ListValue;
        }

        static auto Create(bool v) -> Node* {
            Node* n = new Node;
            n->Type = Bool;
            n->Value.BoolValue = v;
            return n;
        }

        static auto Create(uint32_t v) -> Node* {
            Node* n = new Node;
            n->Type = Int32;
            n->Value.Int32Value = v;
            return n;
        }

        static auto Create(vector<char> v) -> Node* {
            Node* n = new Node;
            n->Type = Name;
            n->Value.NameValue = new string(v.begin(), v.end());
            return n;
        }

        static auto Create(vector<Node*> v) -> Node* {
            Node* n = new Node;
            n->Type = List;
            n->Value.ListValue = new vector<Node*>(v);
            return n;
        }
    };

    static function<Result<Ignore>(State*)> WhiteSpaceParser = pSkipMany<char>(pChar(" \t\r\n"));  

    static function<Result<Node*>(State*)> BoolParser = 
        pChoice<Node*>(
            pMap<string, Node*>(
                pString("true"), 
                [] (string s) { return Node::Create(true); }), 
            pMap<string, Node*>(
                pString("false"), 
                [] (string s) { return Node::Create(false); })); 

    static function<Result<Node*>(State*)> Int32Parser = 
        pMap<uint32_t, Node*>(
            pUIntBase10<uint32_t>(), 
            [] (uint32_t v) { return Node::Create(v); });

    static function<Result<Node*>(State*)> NameParser =  
        pMap<vector<char>, Node*>(
            pMany<char, 1>(
                pChoice(
                    pChar<'a', 'z'>(), 
                    pChar<'A', 'Z'>())), 
            [] (vector<char> v) { return Node::Create(v); });

    auto GetListParser() -> function<Result<Node*>(State*)>  {
        typedef function<Result<Node*>(State*)> P;
        static P ListParser = 
            pMap<vector<Node*>, Node*>(
                pBetween(
                    pChar<'('>(), 
                    pManySeparated(
                        pChoice(
                            BoolParser,
                            Int32Parser,
                            NameParser,
                            static_cast<P>([] (State* s) { return GetListParser()(s); })
                        ),
                        WhiteSpaceParser
                    ), 
                    pChar<')'>()
                ),
                [] (vector<Node*> e) { return Node::Create(e); }
            );
        return ListParser;
    }

    static function<Result<Node*>(State*)> NodeParser =
        pChoice(
            BoolParser,
            Int32Parser,
            NameParser,
            GetListParser()
        );


    auto ParseText(string text) -> Node* {
        Result<Node*> r = RunParser<Node*>(text, NodeParser);
        if (r.IsFailure())
            return nullptr;
        return r.Value;
    }
}

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
    cout << ctfs << "\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
    using namespace TLisp;

    MeasureTime(1000, [] () {
        unique_ptr<Node> n = unique_ptr<Node>(ParseText("(hello (1 2 3) matthew (1 (hello (1 (hello (1 2 3) matthew (1 2 (hello (1 2 3) matthew (1 2 3) true false 123)) true false 123) 3) matthew (1 2 3) true false 123) 3) true false 123)"));
    });
    return 0;
}

