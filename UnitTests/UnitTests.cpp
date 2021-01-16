#include "pch.h"
#include "CppUnitTest.h"

#include <TinyBasic.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			TinyBasic basic;

			basic.parseLine("10 LET A=33");
			basic.parseLine("20 PRINT A");

			basic.run();
		}

		TEST_METHOD(TestMethod2)
		{
			TinyBasic basic;

			basic.parseLine("10 LET A=0");
			basic.parseLine("20 LET A=A+1");
			basic.parseLine("30 IF A<1000 THEN GOTO 20");
			basic.parseLine("40 PRINT 33");

			basic.run();
		}

		TEST_METHOD(TestMethod3)
		{
			TinyBasic basic;

			basic.parseLine("10 PRINT 10");
			basic.parseLine("20 GOSUB 40");
			basic.parseLine("30 GOTO 60");
			basic.parseLine("40 PRINT 40");
			basic.parseLine("50 RETURN");
			basic.parseLine("60 PRINT 60");
			basic.parseLine("70 CLEAR");

			basic.run();
		}
	};
}
