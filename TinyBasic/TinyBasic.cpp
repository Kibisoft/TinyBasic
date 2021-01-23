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

// best is 0471557

#include <TinyBasic.h>

int main()
{
    TinyBasic basic;

    basic.commands["ACCEPT"] = { 0, [](VirtualMachine& vm) {} , false};

    basic.functions["ABS"] = { 1, [](VirtualMachine& vm) { vm[1] = abs(vm[0]); }, true };
    basic.functions["ACS"] = { 1, [](VirtualMachine& vm) { vm[1] = acos(vm[0]); }, true };
    //basic.functions["ASC"] = { 1, [](VirtualMachine& vm) { } };
    basic.functions["ASN"] = { 1, [](VirtualMachine& vm) { vm[1] = asin(vm[0]); }, true };
    basic.functions["ATN"] = { 1, [](VirtualMachine& vm) { vm[1] = atan(vm[0]); }, true };

    return basic.loop();
}
