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

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

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
    le
};


enum class type
{
    integer,
    token
};

class variant
{
public:
    union
    {
        int number;
        token token;
    };

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

class stack : private vector<variant>
{
public:
    void push(const variant& variant) { push_back(variant); }
    void pop() { pop_back(); }
    variant& top() { return back(); }
    bool empty() const { return size() == 0; }
    void clear() { vector<variant>::clear(); }

    variant& operator[](size_t i) { return vector<variant>::operator[](size() - (i + 1)); }
};

class TinyBasic
{
    friend tokenizer;
private:

    map<size_t, string> program;

    stack stack;

    map<size_t, string>::iterator current;
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
                            { "END", token::end, &TinyBasic::parseEnd},

                            { "+", token::plus, &TinyBasic::add},
                            { "-", token::minus, &TinyBasic::sub},
                            { "*", token::mult, &TinyBasic::mult},
                            { "/", token::div, &TinyBasic::div},
                            { "(", token::lparen, &TinyBasic::nop},
                            { ")", token::rparen, &TinyBasic::nop},

                            { "=", token::eq, &TinyBasic::nop},
                            { "<>", token::ne, &TinyBasic::nop},
                            { ">", token::gt, &TinyBasic::nop},
                            { ">=", token::ge, &TinyBasic::nop},
                            { "<", token::lt, &TinyBasic::nop},
                            { "<=", token::le, &TinyBasic::nop},
    };
    
    variant variables[26];

public:

    TinyBasic()
    {
        program[0] = empty;
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
            if (stack.top().number != 0)
                program[stack.top().number] = &line[seek];
            stack.pop();

            return stack.empty();
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
            switch (c)
            {
            case '+':
                stack.push(token::plus);
                break;
            case '-':
                stack.push(token::minus);
                break;
            case '*':
                stack.push(token::mult);
                break;
            case '/':
                stack.push(token::div);
                break;
            case '(':
                stack.push(token::lparen);
                break;
            case ')':
                stack.push(token::rparen);
                break;
            default:
                return false;
            }

            seek++;
            return true;
        }

        return false;
    }
    bool parseNumber()
    {
        if (!eol() && line[seek] >= '0' && line[seek] <= '9')
        {
            stack.push(line[seek] - '0');

            seek++;
            while (!eol() && line[seek] >= '0' && line[seek] <= '9')
            {
                stack.top() *= 10;
                stack.top() += line[seek] - '0';

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
            auto function = tokens[stack.top().token];
            stack.pop();

            return function(*this);
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
            stack.push(token);

            eatBlank();

            return token != token::null;
        }

        seek = i;
        return false;
    }

    bool parseToken(token token)
    {
        if (parseToken() && stack.top().token == token)
        {
            stack.pop();
            return true;
        }

        return false;
    }

    bool parsePrint()
    {
        if (parseExpression())
        {
            cout << stack.top().number << endl;
            stack.pop();

            return true;
        }
        return false;
    }

    bool parseIf()
    {
        if (parseExpression() && parseRelop())
        {
            auto function = tokens[stack.top().token];
            stack.pop();

            if (parseExpression() && parseToken(token::then))
            {
                function(*this);

                if (stack.top().number)
                    return parseStatement();

                return true;
            }
        }

        return false;
    }


    bool parseGoto()
    {
        if (parseExpression())
        {
            current = program.find(stack.top().number);
            current--;
            stack.pop();

            return true;
        }
        return false;
    }

    bool parseGosub()
    {
        if (parseExpression())
        {
            size_t ret = current->first;

            current = program.find(stack.top().number);
            current--;

            stack.pop();
            stack.push(ret);

            return true;
        }
        return false;
    }

    bool parseReturn()
    {
        current = program.find(stack.top().number);
        stack.pop();

        return true;
    }

    bool parseLet()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t i = line[seek] - 'A';
            seek++;

            eatBlank();
            if (!eol() && line[seek] == '=')
            {
                seek++;
                eatBlank();

                if (parseExpression())
                {
                    variables[i] = stack.top();
                    stack.pop();

                    return true;
                }
            }

            return false;
        }

        if (parseExpression())
        {
            current = program.find(stack.top().number);
            current--;
            stack.pop();

            return true;
        }
        return false;
    }

    bool parseList()
    {
        for (auto i : program)
            cout << i.first << ' ' << i.second << endl;
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
        stack.clear();
        current = program.begin();
        current++;

        while (current != program.end())
        {
            
            seek = 0;

            line = current->second;
            if (!parseStatement())
                return false;

            current++;
        }
        return true;
    }

    bool parseEnd()
    {
        current = program.end();
        current--;

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
                auto function = tokens[stack.top().token];
                stack.pop();
                
                if (!parseTerm())
                {
                    return false;
                }

                function(*this);
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
                auto function = tokens[stack.top().token];
                stack.pop();

                if (!parseFactor())
                {
                    return false;
                }

                function(*this);
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
            stack.pop();
            if (parseExpression() && parse(')'))
            {
                stack.pop();
                return true;
            }
        }

        return false;
    }

    bool parseVariable()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            stack.push(variables[line[seek] - 'A']);
            seek++;

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

                stack.push(token::eq);

                eatBlank();

                return true;

            case '>':
                seek++;

                if (!eol() && line[seek] == '=')
                {
                    seek++;

                    stack.push(token::ge);

                    eatBlank();

                    return true;
                }

                eatBlank();

                stack.push(token::gt);

                return true;

            case '<':
                seek++;

                if (!eol())
                {
                    switch (line[seek])
                    {
                    case '=':
                        seek++;

                        stack.push(token::le);

                        eatBlank();

                        return true;

                    case '>':
                        seek++;
                        stack.push(token::ne);

                        eatBlank();

                        return true;
                    }
                }

                stack.push(token::lt);

                eatBlank();

                return true;
            }
        }

        return false;
    }

    bool add()
    {
        stack[1] += stack[0];
        stack.pop();
        return true;
    }

    bool sub()
    {
        stack[1] -= stack[0];
        stack.pop();
        return true;
    }
    bool mult()
    {
        stack[1] *= stack[0];
        stack.pop();
        return true;
    }

    bool div()
    {
        stack[1] /= stack[0];
        stack.pop();
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
    return basic.loop();
}
