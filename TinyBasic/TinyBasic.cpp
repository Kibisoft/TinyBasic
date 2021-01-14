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
#include <variant>
#include <vector>

using namespace std;

const unsigned char i_nop = 0;
const unsigned char i_push = 1;
const unsigned char i_pop = 2;
const unsigned char i_jne = 3;

const unsigned char i_plus = 4;
const unsigned char i_minus = 5;
const unsigned char i_mult = 6;
const unsigned char i_div = 7;

const unsigned char i_setvar = 8;
const unsigned char i_getvar = 9;

const unsigned char i_goto = 10;
const unsigned char i_gosub = 11;
const unsigned char i_return = 12;

const unsigned char i_eq = 13;
const unsigned char i_ne = 14;
const unsigned char i_gt = 15;
const unsigned char i_lt = 16;
const unsigned char i_ge = 17;
const unsigned char i_le = 18;

const unsigned char i_print = 19;


template<class T> class stack : private vector<T>
{
public:
    void push() { vector<T>::push_back(); }
    void push(T t) { vector<T>::push_back(t); }
    void pop() { vector<T>::pop_back(); }
    const T& top() { return vector<T>::back(); }

    const T& operator[](size_t i) const { return vector<T>::operator[](vector<T>::size() - (i + 1)); }
    T& operator[](size_t i) { return vector<T>::operator[](vector<T>::size() - (i + 1)); }
};

class InstructionSet : private vector<size_t>
{
public:
    size_t size() const { return vector<size_t>::size(); }

    void push_instruction(size_t instruction) { vector<size_t>::push_back(instruction); }
    void push_value(double value) { vector<size_t>::push_back(*(size_t*)&value); }
    void push_value(size_t value) { vector<size_t>::push_back(value); }
    void push_variable(unsigned char variable) { vector<size_t>::push_back((size_t)(variable - 'A')); }

    void operator+=(const InstructionSet& set)
    { 
        if (size() > 0)
            vector<size_t>::insert(end(), set.begin(), set.end());
        else
            this->operator=(set);
    }

    size_t operator[](size_t i) const { return vector<size_t>::operator[](i); }
    size_t& operator[](size_t i) { return vector<size_t>::operator[](i); }
};

class VirtualMachine
{
private:

    map<size_t, InstructionSet> program;

    stack<double> stack;
    double variables[26];

    size_t current_instruction;
    map<size_t, InstructionSet>::iterator current_line;

    vector<function<void(VirtualMachine&)>> instructions = {
        &VirtualMachine::i_nop,
        &VirtualMachine::i_push, 
        &VirtualMachine::i_pop,
        &VirtualMachine::i_jne, // jump not equal

        &VirtualMachine::i_plus,
        &VirtualMachine::i_minus,
        &VirtualMachine::i_mult,
        &VirtualMachine::i_div,

        &VirtualMachine::i_setvar,
        &VirtualMachine::i_getvar,

        &VirtualMachine::i_goto,
        &VirtualMachine::i_gosub,
        &VirtualMachine::i_return,

        &VirtualMachine::i_eq,
        &VirtualMachine::i_ne,
        &VirtualMachine::i_gt,
        &VirtualMachine::i_lt,
        &VirtualMachine::i_ge,
        &VirtualMachine::i_le,
    
        &VirtualMachine::i_print,
    };

private:

    void nop() { current_instruction++; }

    void i_push()
    {
        stack.push(*(double*)(&current_line->second[current_instruction + 1])); // var name
        current_instruction += 2;
    }

    void i_pop()
    {
        current_instruction++;
        stack.pop();
    }

    void i_jne()
    {
        current_instruction++;

        if (!stack.top())
        {
            size_t j = current_line->second[current_instruction + 1];
            current_instruction += j;
        }
        else
        {
            current_instruction++;
        }
        stack.pop();
    }

    void i_plus()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] += stack[0];
        stack.pop();
    }

    void i_minus()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] -= stack[0];
        stack.pop();
    }

    void i_mult()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] *= stack[0];
        stack.pop();
    }

    void i_div()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] /= stack[0];
        stack.pop();
    }

    void i_eq()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] == stack[0];
        stack.pop();
    }

    void i_ne()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] != stack[0];
        stack.pop();
    }

    void i_gt()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] >= stack[0];
        stack.pop();
    }

    void i_lt()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] < stack[0];
        stack.pop();
    }

    void i_ge()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] >= stack[0];
        stack.pop();
    }

    void i_le()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] = stack[1] <= stack[0];
        stack.pop();
    }

    void i_print()
    {
        current_instruction++;
        execInstruction();

        cout << stack.top();

        stack.pop();
    }

    void i_set()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] += stack[0];
        stack.pop();
    }

    void i_get()
    {
        current_instruction++;
        execInstruction();
        execInstruction();

        stack[1] += stack[0];
        stack.pop();
    }

    void i_setvar()
    {
        current_instruction++;
        size_t variable = current_line->second[current_instruction];
        current_instruction++;
        execInstruction();

        variables[variable] = stack[0];
        stack.pop();
    }

    void i_getvar()
    {
        current_instruction++;
        size_t variable = current_line->second[current_instruction];
        current_instruction++;

        stack.push(variables[variable]);
    }

    void i_goto()
    {
        current_instruction++;
        execInstruction();
        size_t line = (size_t)stack.top();
        stack.pop();

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_gosub()
    {
        current_instruction++;
        execInstruction();
        size_t line = (size_t)stack.top();
        stack.pop();

        current_line++;

        stack.push(*(double*)&current_line->first);

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_return()
    {
        current_instruction++;
        size_t variable = current_line->second[current_instruction];
        current_instruction++;

        stack.push(variables[variable]);
    }

    void i_nop()
    {
        current_instruction++;
    }

    void execInstruction()
    {
        instructions[current_line->second[current_instruction]](*this);
    }

    void exec()
    {
        current_instruction = 0;
        while (current_instruction < current_line->second.size())
        {
            execInstruction();
        }
    }

public:

    void run(const map<size_t, InstructionSet>& p)
    {
        program = p;
        program[0] = InstructionSet();

        current_line = program.begin();
        current_line++;

        LARGE_INTEGER s, e;
        QueryPerformanceCounter(&s);

        try
        {
            while (current_line != program.end())
            {
                exec();
                current_line++;
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
};

class ParserResult
{
private:
    bool valid;
    variant<size_t, double, string, InstructionSet> value;
public:

    ParserResult(bool b) { valid = b; }
    const ParserResult& operator=(bool b) { valid = b; return *this; }
    operator bool() { return valid; }

    ParserResult(double d) { value = d; valid = true; }
    operator double() { return get<double>(value); }

    ParserResult(size_t s) { value = s; valid = true; }
    operator size_t() { return get<size_t>(value); }

    ParserResult(InstructionSet is) { valid = true; value = is; }
    operator InstructionSet() { return get<InstructionSet>(value); }
};

class TinyBasic;

class tokenizer
{
private:
    map<string, token> tokens;
    map<token, function<ParserResult(TinyBasic&)>> functions;

public:
    tokenizer(std::initializer_list<tuple<string, token, function<ParserResult(TinyBasic&)>>> initializer);

    token operator[](const string& str) const
    {
        auto token = tokens.find(str);
        if (token != tokens.end())
            return token->second;
        return token::null;
    }

    function<ParserResult(TinyBasic&)> operator[](token token) const
    {
        return functions.find(token)->second;
    }
};

class TinyBasic
{
    friend tokenizer;
private:

    map<size_t, InstructionSet> program;

    string empty;
    string& line = empty;
    size_t seek = 0;
    tokenizer tokens = {    { "PRINT", token::print, &TinyBasic::parsePrint},
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
 
public:

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

    ParserResult parse()
    {
        seek = 0;

        if (ParserResult number = parseNumber())
        {
            if (ParserResult set = parseStatement())
            {
                double n = number;
                program[(size_t)n] = set;
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

    ParserResult parseNumber()
    {
        if (!eol() && line[seek] >= '0' && line[seek] <= '9')
        {
            double number = line[seek] - '0';

            seek++;
            while (!eol() && line[seek] >= '0' && line[seek] <= '9')
            {
                number = number * 10 + (line[seek] - '0');

                seek++;
            }

            eatBlank();

            return number;
        }

        return false;
    }

    ParserResult parseStatement()
    {
        if (ParserResult statement = parseToken())
        {
            return statement;
        }

        return false;
    }

    ParserResult parseToken()
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

            return  tokens[token](*this);
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

    ParserResult parseNull()
    {
        return false;
    }

    ParserResult parsePrint()
    {
        if (ParserResult expression = parseExpression())
        {
            InstructionSet set;
            set.push_instruction(i_print);
            set += expression;

            return set;
        }
        return false;
    }

    ParserResult parseIf()
    {
        if (ParserResult exp1 = parseExpression())
        {
            if (ParserResult op = parseRelop())
            {
                if (ParserResult exp2 = parseExpression())
                {
                    if (parseToken(token::then))
                    {
                        if (ParserResult statement = parseStatement())
                        {

                            InstructionSet set;
                            set += op;
                            set += exp1;
                            set += exp2;
                            set.push_instruction(i_jne);
                            set.push_value(((InstructionSet)statement).size());
                            set += statement;

                            return set;
                        }
                    }
                }
            }
        }

        return false;
    }


    ParserResult parseGoto()
    {
        if (ParserResult expression = parseExpression())
        {
            InstructionSet set;

            set.push_instruction(i_goto);
            set += expression;

            return set;
        }
        return false;
    }

    ParserResult parseGosub()
    {
        if (parseExpression())
        {
            return true;
        }
        return false;
    }

    ParserResult parseReturn()
    {
        return true;
    }

    ParserResult parseLet()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t variable = (size_t)(line[seek] - 'A');

            seek++;

            eatBlank();
            if (!eol() && line[seek] == '=')
            {
                seek++;
                eatBlank();

                if (ParserResult expression = parseExpression())
                {
                    InstructionSet set;

                    set.push_instruction(i_setvar);
                    set.push_value(variable);
                    set += expression;

                    return set;
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

    ParserResult parseList()
    {
        return true;
    }

    ParserResult parseClear()
    {
        for (size_t i = 0; i < 100; i++)
            cout << endl;
        return true;
    }

    ParserResult parseRun()
    {
        VirtualMachine vm;

        vm.run(program);

        return true;
    }

    ParserResult parseEnd()
    {
        return true;
    }

    ParserResult parseExpression()
    {
        if (ParserResult a = parseTerm())
        {
            InstructionSet set;
            set += a;

            bool loop = true;


            while (loop)
            {
                loop = false;
                if (parse('+'))
                {
                    if (ParserResult b = parseTerm())
                    {
                        loop = true;

                        InstructionSet plus;

                        plus.push_instruction(i_plus);
                        plus += set;
                        plus += b;

                        set = plus;
                    }
                    else
                        return false;
                }
                else if (parse('-'))
                {
                    if (ParserResult b = parseTerm())
                    {
                        loop = true;

                        InstructionSet minus;

                        minus.push_instruction(i_minus);
                        minus += set;
                        minus += b;

                        set = minus;
                    }
                    else
                        return false;
                }
            }

            return set;
        }

        return false;
    }

    ParserResult parseTerm()
    {
        if (ParserResult a = parseFactor())
        {
            InstructionSet set;
            set += a;

            bool loop = true;


            while (loop)
            {
                loop = false;
                if (parse('*'))
                {
                    if (ParserResult b = parseFactor())
                    {
                        loop = true;

                        InstructionSet mult;

                        mult.push_instruction(i_mult);
                        mult += set;
                        mult += b;

                        set = mult;
                    }
                    else
                        return false;
                }
                else if (parse('/'))
                {
                    if (ParserResult b = parseFactor())
                    {
                        loop = true;

                        InstructionSet div;

                        div.push_instruction(i_div);
                        div += set;
                        div += b;

                        set = div;
                    }
                    else
                        return false;
                }
            }

            return set;
        }

        return false;
    }

    ParserResult parseFactor()
    {
        if (ParserResult number = parseNumber())
        {
            InstructionSet set;

            set.push_instruction(i_push);
            set.push_value((double)number);

            return set;
        }
        else if (ParserResult variable = parseVariable())
        {
            InstructionSet set;

            set.push_instruction(i_getvar);
            set.push_value((size_t)variable);

            return set;
        }
        else if (parse('('))
        {
            if (ParserResult exp = parseExpression() && parse(')'))
            {
                return exp;
            }
        }

        return false;
    }

    ParserResult parseVariable()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t variable = (size_t)(line[seek] - 'A');
            seek++;

            eatBlank();

            return variable;
        }

        return false;
    }

    ParserResult parseRelop()
    {
        InstructionSet set;

        if (!eol())
        {
            switch (line[seek])
            {
            case '=':
                seek++;

                eatBlank();

                set.push_instruction(i_eq);

                return set;

            case '>':
                seek++;

                if (!eol() && line[seek] == '=')
                {
                    seek++;

                    eatBlank();

                    set.push_instruction(i_ge);

                    return set;

                }

                eatBlank();

                set.push_instruction(i_gt);

                return set;

            case '<':
                seek++;

                if (!eol())
                {
                    switch (line[seek])
                    {
                    case '=':
                        seek++;

                        eatBlank();

                        set.push_instruction(i_le);

                        return set;

                    case '>':
                        seek++;

                        eatBlank();

                        set.push_instruction(i_ne);

                        return set;
                    }
                }

                eatBlank();

                set.push_instruction(i_lt);

                return set;
            }
        }

        return false;
    }
    
    ParserResult nop()
    {
        return true;
    }
};

tokenizer::tokenizer(std::initializer_list<tuple<string, token, function<ParserResult(TinyBasic&)>>> initializer)
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
    basic.parseLine("30 IF A>100000000 THEN GOTO 50");
    basic.parseLine("40 GOTO 20");
    basic.parseLine("50 PRINT 0");

    return basic.loop();
}
