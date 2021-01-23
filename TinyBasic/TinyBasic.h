#pragma once

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

#ifdef _DEBUG
#include <Windows.h>
#endif

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
    input = 21,
    clear = 22,

    call = 23,
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

class VirtualMachine;

class Instruction : public function<void(VirtualMachine&)>
{
public:
    Instruction(function<void(VirtualMachine&)> f) : function<void(VirtualMachine&)>(f) {}
};

class VirtualMachine
{
private:
    map<size_t, InstructionSet> program;

    stack<double> stack;
    double variables[26];

    size_t current_instruction;
    map<size_t, InstructionSet>::iterator current_line;

    vector<Instruction> instructions = {
        Instruction(&VirtualMachine::i_nop),
        Instruction(&VirtualMachine::i_push),
        Instruction(&VirtualMachine::i_pop),
        Instruction(&VirtualMachine::i_jne), // jump not equal

        Instruction(&VirtualMachine::i_plus),
        Instruction(&VirtualMachine::i_minus),
        Instruction(&VirtualMachine::i_mult),
        Instruction(&VirtualMachine::i_div),

        Instruction(&VirtualMachine::i_setvar),
        Instruction(&VirtualMachine::i_getvar),

        Instruction(&VirtualMachine::i_goto),
        Instruction(&VirtualMachine::i_gosub),
        Instruction(&VirtualMachine::i_return),

        Instruction(&VirtualMachine::i_end),

        Instruction(&VirtualMachine::i_eq),
        Instruction(&VirtualMachine::i_ne),
        Instruction(&VirtualMachine::i_gt),
        Instruction(&VirtualMachine::i_lt),
        Instruction(&VirtualMachine::i_ge),
        Instruction(&VirtualMachine::i_le),

        Instruction(&VirtualMachine::i_print),
        Instruction(&VirtualMachine::i_input),
        Instruction(&VirtualMachine::i_clear),

        Instruction(&VirtualMachine::i_call),
    };

private:

    void i_push()
    {
        stack.push(*(double*)(&current_line->second[current_instruction])); // var name
        current_instruction++;
    }

    void i_pop()
    {
        stack.pop();
    }

    void i_jne()
    {
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
        execInstruction();
        execInstruction();

        stack[1] += stack[0];
        stack.pop();
    }

    void i_minus()
    {
        execInstruction();
        execInstruction();

        stack[1] -= stack[0];
        stack.pop();
    }

    void i_mult()
    {
        execInstruction();
        execInstruction();

        stack[1] *= stack[0];
        stack.pop();
    }

    void i_div()
    {
        execInstruction();
        execInstruction();

        stack[1] /= stack[0];
        stack.pop();
    }

    void i_eq()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] == stack[0];
        stack.pop();
    }

    void i_ne()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] != stack[0];
        stack.pop();
    }

    void i_gt()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] >= stack[0];
        stack.pop();
    }

    void i_lt()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] < stack[0];
        stack.pop();
    }

    void i_ge()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] >= stack[0];
        stack.pop();
    }

    void i_le()
    {
        execInstruction();
        execInstruction();

        stack[1] = stack[1] <= stack[0];
        stack.pop();
    }

    void i_print()
    {
        execInstruction();

        cout << stack.top() << endl;

        stack.pop();
    }

    void i_input()
    {
        cout << "? ";
        cin >> variables[current_line->second[current_instruction]];
    }

    void i_clear()
    {
        cout << "\x1B[2J\x1B[H";
    }

    void i_setvar()
    {
        size_t variable = current_line->second[current_instruction];
        current_instruction++;
        execInstruction();

        variables[variable] = stack.top();
        stack.pop();
    }

    void i_getvar()
    {
        stack.push(variables[current_line->second[current_instruction]]);
        current_instruction++;
    }

    void i_goto()
    {
        execInstruction();
        size_t line = (size_t)stack.top();
        stack.pop();

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_gosub()
    {
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
        size_t line = (size_t)stack.top();

        current_line = program.find(line);
        current_line--;

        current_instruction = current_line->second.size();
    }

    void i_end()
    {
        current_line = program.end();
    }

    void i_call()
    {
        size_t nb_of_params = current_line->second[current_instruction];
        current_instruction++;

        void(*callback)(VirtualMachine&) = (void(*)(VirtualMachine&))current_line->second[current_instruction];
        current_instruction++;

        for (size_t i = 0; i < nb_of_params; i++)
            execInstruction();

        callback(*this);

        for (size_t i = 0; i < nb_of_params; i++)
            stack.pop();
    }

    void i_nop() {}

    void execInstruction()
    {
        function<void(VirtualMachine&)> instruction = instructions[current_line->second[current_instruction]];
        current_instruction++;
        instruction(*this);
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

    double operator[](size_t i) const { return stack[i]; }
    double& operator[](size_t i) { return stack[i]; }

    double top()
    {
        return stack.top();
    }

    void run(const map<size_t, InstructionSet>& p)
    {
        program = p;
        program[0] = InstructionSet();

        current_line = program.begin();
        current_line++;

#ifdef _DEBUG
        LARGE_INTEGER s, e;
        QueryPerformanceCounter(&s);
#endif

        try
        {
            while (current_line != program.end())
            {
                exec();
                current_line++;
            }
        }
        catch (...) {}
        
#ifdef _DEBUG
        QueryPerformanceCounter(&e);
        cout << e.QuadPart - s.QuadPart << endl;
#endif
    }
};

class ParserResult
{
private:
    bool valid;
    variant<size_t, double, string, InstructionSet> value;
public:

    ParserResult(bool b) { valid = b; }
    operator bool() const { return valid; }

    ParserResult(double d) { value = d; valid = true; }
    operator double() const { return get<double>(value); }

    ParserResult(size_t s) { value = s; valid = true; }
    operator size_t() const { return get<size_t>(value); }

    ParserResult(instruction i) { InstructionSet set; set.push(i); value = set; valid = true; }

    ParserResult(InstructionSet is) { valid = true; value = is; }
    operator InstructionSet() const { return get<InstructionSet>(value); }

    ParserResult operator+(const ParserResult& b)
    {
        InstructionSet result = *this;
        result += (InstructionSet)b;

        return result;
    }

    ParserResult operator+(size_t s)
    {
        InstructionSet result = *this;
        result.push_value(s);

        return result;
    }

    ParserResult operator+(double d)
    {
        InstructionSet result = *this;
        result.push_value(d);

        return result;
    }
};

ParserResult operator+(instruction i, const ParserResult& r)
{
    InstructionSet result;
    result.push(i);
    result += (InstructionSet)r;

    return result;
}

class TinyBasic
{
private:

    map<size_t, InstructionSet> program;
    map<size_t, string> source;

    string empty;
    string& line = empty;
    size_t seek = 0;

    map<string, function<ParserResult(TinyBasic&)>> instructions = {
        { "CALL", &TinyBasic::parseCall},
        { "CLEAR", &TinyBasic::parseClear},
        { "END", &TinyBasic::parseEnd},
        { "GOSUB", &TinyBasic::parseGosub},
        { "GOTO", &TinyBasic::parseGoto},
        { "IF", &TinyBasic::parseIf},
        { "INPUT", &TinyBasic::parseInput},
        { "LET", &TinyBasic::parseLet},
        { "LIST", &TinyBasic::parseList},
        { "PRINT", &TinyBasic::parsePrint},
        { "RETURN", &TinyBasic::parseReturn},
        { "RUN", &TinyBasic::parseRun},
    };

public:

    map<string, tuple<size_t, void(*)(VirtualMachine&), bool>> functions;
    map<string, tuple<size_t, void(*)(VirtualMachine&), bool>> commands;

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

    int run()
    {
        cout << "Tiny Basic v0.1 by Fred Morales" << endl;

        VirtualMachine vm;
        vm.run(program);

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
            double number = (double)(line[seek] - (size_t)'0');

            seek++;
            while (!eol() && line[seek] >= '0' && line[seek] <= '9')
            {
                number = number * 10 + (line[seek] - (size_t)'0');

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

            string f = line.substr(i, seek - i);

            auto function = instructions.find(f);
            if (function != instructions.end())
            {
                eatBlank();
                return function->second(*this);
            }
            else
            {
                auto command = commands.find(f);
                if (command != commands.end())
                {
                    eatBlank();
                    return parseCommand(get<0>(command->second), get<1>(command->second), get<2>(command->second));
                }
             }
        }

        seek = i;
        return false;
    }

    ParserResult parseFunction()
    {
        size_t i = seek;

        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            seek++;
            while (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
            {
                seek++;
            }

            string f = line.substr(i, seek - i);

            auto function = functions.find(f);
            if (function != functions.end())
            {
                eatBlank();
                return parseFunction(get<0>(function->second), get<1>(function->second), get<2>(function->second));
            }
        }

        seek = i;
        return false;
    }
    ParserResult parsePrint()
    {
        if (ParserResult expression = parseExpression())
        {
            return instruction::print + expression;
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

            return instruction::input + variable;
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
                            return op + exp1 + exp2 + instruction::jne + ((InstructionSet)statement).size() + statement;
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
            return instruction::got + expression;
        }
        return false;
    }

    ParserResult parseGosub()
    {
        if (ParserResult expression = parseExpression())
        {
            return instruction::gosub + expression;
        }
        return false;
    }

    ParserResult parseReturn()
    {
        return instruction::ret;
    }

    ParserResult parseLet()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t variable = (size_t)line[seek] - (size_t)'A';

            seek++;

            eatBlank();
            if (!eol() && line[seek] == '=')
            {
                seek++;
                eatBlank();

                if (ParserResult expression = parseExpression())
                {
                    return ParserResult(instruction::setvar) + variable + expression;
                }
            }
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

    ParserResult parseCall()
    {
        return parseFunction();
    }

    ParserResult parseClear()
    {
        return instruction::clear;
    }

    ParserResult parseRun()
    {
        VirtualMachine vm;

        vm.run(program);

        return true;
    }

    ParserResult parseEnd()
    {
        return instruction::end;
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

                        return instruction::plus + set + b;
                    }
                    else
                        return false;
                }
                else if (parse('-'))
                {
                    if (ParserResult b = parseTerm())
                    {
                        loop = true;

                        return instruction::minus + set + b;
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

                        set = instruction::mult + set + b;

                    }
                    else
                        return false;
                }
                else if (parse('/'))
                {
                    if (ParserResult b = parseFactor())
                    {
                        loop = true;

                        return instruction::plus + set + b;
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
            return ParserResult(instruction::push) + (double)number;
        }
        else if (ParserResult variable = parseVariable())
        {
            return ParserResult(instruction::getvar) + (size_t)variable;
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

    ParserResult parseRelop()
    {
        if (!eol())
        {
            switch (line[seek])
            {
            case '=':
                seek++;

                eatBlank();

                return instruction::eq;

            case '>':
                seek++;

                if (!eol() && line[seek] == '=')
                {
                    seek++;

                    eatBlank();

                    return instruction::ge;

                }

                eatBlank();

                return instruction::gt;

            case '<':
                seek++;

                if (!eol())
                {
                    switch (line[seek])
                    {
                    case '=':
                        seek++;

                        eatBlank();

                        return instruction::le;

                    case '>':
                        seek++;

                        eatBlank();

                        return instruction::ne;
                    }
                }

                eatBlank();

                return instruction::lt;
            }
        }

        return false;
    }

    ParserResult parseVariable()
    {
        if (!eol() && line[seek] >= 'A' && line[seek] <= 'Z')
        {
            size_t variable = (size_t)line[seek] - (size_t)'A';
            seek++;

            eatBlank();

            return variable;
        }

        return false;
    }

    ParserResult parseCommand(size_t parameters, void(*f)(VirtualMachine&), bool parenthesis)
    {
        ParserResult set = ParserResult(instruction::call) + parameters + (size_t)f;

        eatBlank();

        if (!parenthesis || parse('('))
        {
            eatBlank();
            if (parameters > 0)
            {

                if (ParserResult exp = parseExpression())
                    set = set + exp;
                else
                    return false;

                parameters--;

                while (parameters > 0 && parse(','))
                {
                    eatBlank();

                    if (ParserResult exp = parseExpression())
                        set = set + exp;
                    else
                        return false;

                    parameters--;
                }
            }

            if (parenthesis && !parse(')'))
                return false;
        }
        return set;
    }

    ParserResult parseFunction(size_t parameters, void(*f)(VirtualMachine&), bool parenthesis)
    {
        return ParserResult(instruction::push) + 0.0 + parseCommand(parameters, f, parenthesis);
    }
};
