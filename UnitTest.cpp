//#include "pch.h"
#include "StdAfx.h"
#include <CppUnitTest.h>
//#include "CppUnitTest.h"
//#include "SysLatData.h"



using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Assert::AreEqual(1, 1);
			//CSysLatData newSLD;// = new CSysLatData;
			//int test1 = newSLD->GetCounter();
			//int test2 = 0;
			//Assert::AreEqual(test1, test2);
		}
	};
}
