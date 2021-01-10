 // implementation of Tiny BASIC by Fred Morales
// https://github.com/Kibisoft/TinyBasic
// https://en.wikipedia.org/wiki/Tiny_BASIC
//
//MIT License
//
//Copyright(c) 2021 Fred Morales
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this softwareand associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright noticeand this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

// best is 3898762

#include <Windows.h>

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <stack>

using namespace std;

const size_t add_instruction = 0;
const size_t sub_instruction = 1;
const size_t mult_instruction = 2;
const size_t div_instruction = 3;
const size_t lparen_instruction = 4;
const size_t rparen_instruction = 5;

const size_t i_eq = 6;
const size_t i_ne = 7;
const size_t i_gt = 8;
const size_t i_ge = 9;
const size_t i_lt = 10;
const size_t i_le = 11;
const size_t i_move = 12;
const size_t i_push = 13;

class VirtualMachine
{
private:

    map<size_t, int> program;

    stack<int> stack;
    map<size_t, int>::iterator current;

    vector<function<void(VirtualMachine&)>> instructions = { &VirtualMachine::nop, &VirtualMachine::push, &VirtualMachine::pop };

    void nop() {}
    void push() {}
    void pop() {}

public:

    void run()
    {
        current = program.begin();
        current++;

        LARGE_INTEGER s, e;
        QueryPerformanceCounter(&s);

        try
        {
            while (current != program.end())
            {
                current++;
            }
        }
        catch (...) {}
        QueryPerformanceCounter(&e);
        cout << e.QuadPart - s.QuadPart << endl;
    }
};


enum class token
{
    null,
    value,
    run,
    end,
    list,
    clear,
    print,
    _if,
    then,
    _else,
    _goto,
    gosub,
    _return,
    let,
    plus,
    minus,
    mult,
    div,
    lparen,
    rparen,
    eq,
    ne,
    gt,
    lt,
    ge,
    le,
    var,
    assign
};


enum class type
{
    integer,
    token
};

class variant
{
public:
//    union
    //{
    int number;
    token token;
    vector<size_t> bytecode;
    //};

    type type;

    variant() : type(type::integer), number(0) {};
    variant(enum token t) : type(type::token), token(t) {};
    variant(int n) : type(type::integer), number(n) {};
    
    void operator+=(int i) { number += i; }
    void operator-=(int i) { number -= i; }
    void operator*=(int i) { number *= i; }
    void operator/=(int i) { number /= i; }

    void operator+=(const variant& i) { number += i.number; }
    void operator-=(const variant& i) { number -= i.number; }
    void operator*=(const variant& i) { number *= i.number; }
    void operator/=(const variant& i) { number /= i.number; }

    variant operator==(const variant& i) { return variant(number == i.number); }
    variant operator!=(const variant& i) { return variant(number != i.number); }
    variant operator>(const variant& i) { return variant(number > i.number); }
    variant operator<(const variant& i) { return variant(number < i.number); }
    variant operator>=(const variant& i) { return variant(number >= i.number); }
    variant operator<=(const variant& i) { return variant(number <= i.number); }
};

class TinyBasic;

class tokenizer
{
private:
    map<string, token> tokens;
    map<token, function<bool(TinyBasic&)>> functions;

public:
    tokenizer(std::initializer_list<tuple<string, token, function<bool(TinyBasic&)>>> initializer);

    token operator[](const string& str) const
    {
        auto token = tokens.find(str);
        if (token != tokens.end())
            return token->second;
        return token::null;
    }

    function<bool(TinyBasic&)> operator[](token token) const
    {
        return functions.find(token)->second;
    }
};

class stack_item : public vector<size_t>
{
public:
    size_t number;
};


class Line
{
public:
    string source;
    vector<size_t> bytecode;
};

class TinyBasic
{
    friend tokenizer;
private:

    map<size_t, Line>::iterator current;
    string empty;
    string& line = empty;
    size_t seek = 0;
    tokenizer tokens = {    { "PRINT", token::print, &TinyBasic::parsePrint} ,
                            { "IF", token::_if,&TinyBasic::parseIf},
                            { "THEN", token::then, &TinyBasic::nop},
                            { "ELSE", token::_else, &TinyBasic::nop},
                            { "GOTO", token::_goto,&TinyBasic::parseGoto},
                            { "GOSUB", token::gosub,&TinyBasic::parseGosub},
                            { "RETURN", token::_return,&TinyBasic::parseReturn},
                            { "LET", token::let, &TinyBasic::parseLet},
                            { "CLEAR", token::clear, &TinyBasic::parseClear},
                            { "LIST", token::list, &TinyBasic::parseList},
                            { "RUN", token::run, &TinyBasic::parseRun},
                            { "END", token::end, &TinyBasic::parseEnd}
    };
    

                 
    vector<function<bool(TinyBasic&)>> instructions =
    {
        &TinyBasic::add,
        &TinyBasic::sub,
        &TinyBasic::mult,
        &TinyBasic::div,
        &TinyBasic::nop,
        &TinyBasic::nop,

        &TinyBasic::eq,
        &TinyBasic::ne,
        &TinyBasic::gt,
        &TinyBasic::ge,
        &TinyBasic::lt,
        &TinyBasic::le,

        &TinyBasic::move,
        &TinyBasic::push
    };

    int variables[26];

public:

    TinyBasic()
    {
    }

    void parseLine(const string& aline)
    {
        line = aline;
        parse();
    }

    int loop()
    {
        while (true)
        {
            getline(cin, line);

            parse();
        }

        return true;
    }

private:

    bool eol()
    {
        return seek >= line.size();
    }

    void eatBlank()
    {
        while (!eol() && (line[seek] == ' ' || line[seek] == '\t'))
            seek++;
    }

    bool parse()
    {
        seek = 0;

        if (parseNumber())
        {
            if (parseStatement())
            {
            }
            return true;
        }
        else
        {
            return parseStatement();
        }

        return false;
    }

    bool parse(char c)
    {
        if (line[seek] == c)
        {
            seek++;
            return true;
        }

        return false;
    }

    bool parseNumber()
    {
        if (!eol() && line[seek] >= '0' && line[seek] <= '9')
        {
            int number = line[seek] - '0';

            seek++;
            while (!eol() && line[seek] >= '0' && line[seek] <= '9')
            {
                number = number * 10 + (line[seek] - '0');

                seek++;
            }

            eatBlank();

            return true;
        }

        return false;
    }

    bool parseStatement()
    {
        if (parseToken())
        {
            return true;
        }

        return false;
    }

    bool parseToken()
    {
        size_t i = seek;

        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            seek++;
            while (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
            {
                seek++;
            }

            token token = tokens[line.substr(i, seek - i)];

            eatBlank();

            return token != token::null;
        }

        seek = i;
        return false;
    }

    bool parseToken(token token)
    {
        if (parseToken())
        {
            return true;
        }

        return false;
    }

    bool parsePrint()
    {
        if (parseExpression())
        {
            return true;
        }
        return false;
    }

    bool parseIf()
    {
        if (parseExpression() && parseRelop())
        {
            if (parseExpression() && parseToken(token::then))
            {
                return parseStatement();
            }
        }

        return false;
    }


    bool parseGoto()
    {
        if (parseExpression())
        {
            return true;
        }
        return false;
    }

    bool parseGosub()
    {
        if (parseExpression())
        {
            return true;
        }
        return false;
    }

    bool parseReturn()
    {
        return true;
    }

    bool parseLet()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            seek++;

            eatBlank();
            if (!eol() && line[seek] == '=')
            {
                seek++;
                eatBlank();

                if (parseExpression())
                {
                    return true;
                }
            }

            return false;
        }

        if (parseExpression())
        {
            return true;
        }
        return false;
    }

    bool parseList()
    {
        return true;
    }

    bool parseClear()
    {
        for (size_t i = 0; i < 100; i++)
            cout << endl;
        return true;
    }

    bool parseRun()
    {
        return true;
    }

    bool parseEnd()
    {
        return true;
    }

    bool parseNull()
    {
        return false;
    }

    bool parseExpression()
    {
        if (parseTerm())
        {
            while (parse('+') || parse('-'))
            {
                if (!parseTerm())
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    bool parseTerm()
    {
        if (parseFactor())
        {
            while (parse('*') || parse('/'))
            {
                if (!parseFactor())
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    bool parseFactor()
    {
        if (parseNumber() || parseVariable())
            return true;
        else if (parse('('))
        {
            if (parseExpression() && parse(')'))
            {
                return true;
            }
        }

        return false;
    }

    bool parseVariable()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            eatBlank();

            return true;
        }

        return false;
    }

    bool parseRelop()
    {
        if (!eol())
        {
            switch (line[seek])
            {
            case '=':
                seek++;

                eatBlank();

                return true;

            case '>':
                seek++;

                if (!eol() && line[seek] == '=')
                {
                    seek++;

                    eatBlank();

                    return true;
                }

                eatBlank();

                return true;

            case '<':
                seek++;

                if (!eol())
                {
                    switch (line[seek])
                    {
                    case '=':
                        seek++;

                        eatBlank();

                        return true;

                    case '>':
                        seek++;

                        eatBlank();

                        return true;
                    }
                }

                eatBlank();

                return true;
            }
        }

        return false;
    }

    bool add()
    {
        return true;
    }

    bool sub()
    {
        return true;
    }

    bool mult()
    {
        return true;
    }

    bool div()
    {
        return true;
    }


    bool eq()
    {
        return true;
    }

    bool ne()
    {
        return true;
    }

    bool gt()
    {
        return true;
    }

    bool lt()
    {
        return true;
    }

    bool ge()
    {
        return true;
    }

    bool le()
    {
        return true;
    }

    bool move()
    {
        return true;
    }

    bool push()
    {
        return true;
    }

    bool nop()
    {
        return true;
    }
};

tokenizer::tokenizer(std::initializer_list<tuple<string, token, function<bool(TinyBasic&)>>> initializer)
{
    functions[token::null] = &TinyBasic::parseNull;
    for (auto item : initializer)
    {
        tokens[get<0>(item)] = get<1>(item);
        functions[get<1>(item)] = get<2>(item);
    }
}

int main()
{
    TinyBasic basic;

    basic.parseLine("10 LET A=1");
    basic.parseLine("20 LET A=A+1");
    basic.parseLine("30 IF A>1000000 THEN GOTO 50");
    basic.parseLine("40 GOTO 20");
    basic.parseLine("50 PRINT 0");

    return basic.loop();
}
