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
// best is 0471557

#include <Windows.h>

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <variant>
#include <vector>

using namespace std;

enum class instruction : size_t
{
    nop = 0,
    push = 1,
    pop = 2,
    jne = 3,

    plus = 4,
    minus = 5,
    mult = 6,
    div = 7,

    setvar = 8,
    getvar = 9,

    got = 10,
    gosub = 11,
    ret = 12,
    end = 13,

    eq = 14,
    ne = 15,
    gt = 16,
    lt = 17,
    ge = 18,
    le = 19,

    print = 20,
    input = 21
};

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

    void push(instruction instruction) { vector<size_t>::push_back((size_t)instruction); }
    void push_value(double value) { vector<size_t>::push_back(*(size_t*)&value); }
    void push_value(size_t value) { vector<size_t>::push_back(value); }

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

        &VirtualMachine::i_end,

        &VirtualMachine::i_eq,
        &VirtualMachine::i_ne,
        &VirtualMachine::i_gt,
        &VirtualMachine::i_lt,
        &VirtualMachine::i_ge,
        &VirtualMachine::i_le,
    
        &VirtualMachine::i_print,
        &VirtualMachine::i_input,
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

        cout << stack.top() << endl;

        stack.pop();
    }

    void i_input()
    {
        current_instruction++;
        size_t variable = current_line->second[current_instruction];
        current_instruction++;

        cout << "? ";
        cin >> variables[variable];
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

        stack.push((double)current_line->first);

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_return()
    {
        current_instruction++;
        size_t line = (size_t)stack.top();

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_end()
    {
        current_line = program.end();
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

class ParserResult
{
private:
    bool valid;
    variant<size_t, double, string, InstructionSet> value;
public:

    ParserResult(bool b) { valid = b; }
    operator bool() { return valid; }

    ParserResult(double d) { value = d; valid = true; }
    operator double() { return get<double>(value); }

    ParserResult(size_t s) { value = s; valid = true; }
    operator size_t() { return get<size_t>(value); }

    ParserResult(InstructionSet is) { valid = true; value = is; }
    operator InstructionSet() { return get<InstructionSet>(value); }
};

class TinyBasic;

class TinyBasic
{
private:

    map<size_t, InstructionSet> program;
    map<size_t, string> source;

    string empty;
    string& line = empty;
    size_t seek = 0;

    map<string, function<ParserResult(TinyBasic&)>> functions = {
        { "PRINT", &TinyBasic::parsePrint},
        { "INPUT", &TinyBasic::parseInput},
        { "IF",&TinyBasic::parseIf},
        { "GOTO",&TinyBasic::parseGoto},
        { "GOSUB",&TinyBasic::parseGosub},
        { "RETURN",&TinyBasic::parseReturn},
        { "LET", &TinyBasic::parseLet},
        { "CLEAR", &TinyBasic::parseClear},
        { "LIST", &TinyBasic::parseList},
        { "RUN", &TinyBasic::parseRun},
        { "END", &TinyBasic::parseEnd}
    };
 
public:

    void parseLine(const string& aline)
    {
        line = aline;
        parse();
    }

    int loop()
    {
        cout << "Tiny Basic v0.1 by Fred Morales" << endl;

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
            size_t s = seek;
            if (ParserResult set = parseStatement())
            {
                double n = number;
                program[(size_t)n] = set;
                source[(size_t)n] = &line[s];
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

    bool parse(const string& string)
    {
        size_t i = 0;
        while (line[seek] == string[i])
        {
            seek++;
            i++;
        }

        eatBlank();

        return i == string.size();
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

            auto function = functions[line.substr(i, seek - i)];

            eatBlank();

            return  function(*this);
        }

        seek = i;
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
            set.push(instruction::print);
            set += expression;

            return set;
        }
        return false;
    }

    ParserResult parseInput()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t variable = (size_t)(line[seek] - 'A');

            seek++;

            eatBlank();

            InstructionSet set;

            set.push(instruction::input);
            set.push_value(variable);

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
                    if (parse("THEN"))
                    {
                        if (ParserResult statement = parseStatement())
                        {

                            InstructionSet set;
                            set += op;
                            set += exp1;
                            set += exp2;
                            set.push(instruction::jne);
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

            set.push(instruction::got);
            set += expression;

            return set;
        }
        return false;
    }

    ParserResult parseGosub()
    {
        if (ParserResult expression = parseExpression())
        {
            InstructionSet set;

            set.push(instruction::gosub);
            set += expression;

            return set;
        }
        return false;
    }

    ParserResult parseReturn()
    {
        InstructionSet set;

        set.push(instruction::ret);

        return set;
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

                    set.push(instruction::setvar);
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
        for (auto l : source)
        {
            cout << l.first << ' ' << l.second << endl;
        }
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
        InstructionSet set;
        set.push(instruction::end);

        return set;
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

                        plus.push(instruction::plus);
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

                        minus.push(instruction::minus);
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

                        mult.push(instruction::mult);
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

                        div.push(instruction::div);
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

            set.push(instruction::push);
            set.push_value((double)number);

            return set;
        }
        else if (ParserResult variable = parseVariable())
        {
            InstructionSet set;

            set.push(instruction::getvar);
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

                set.push(instruction::eq);

                return set;

            case '>':
                seek++;

                if (!eol() && line[seek] == '=')
                {
                    seek++;

                    eatBlank();

                    set.push(instruction::ge);

                    return set;

                }

                eatBlank();

                set.push(instruction::gt);

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

                        set.push(instruction::le);

                        return set;

                    case '>':
                        seek++;

                        eatBlank();

                        set.push(instruction::ne);

                        return set;
                    }
                }

                eatBlank();

                set.push(instruction::lt);

                return set;
            }
        }

        return false;
    }
};

int main()
{
    TinyBasic basic;

    basic.parseLine("10 INPUT A");
    basic.parseLine("20 PRINT A");
    basic.parseLine("30 GOTO 20");

    return basic.loop();
}
