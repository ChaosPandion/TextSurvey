#include "Common.h"
#include "../TextSurvey/TextSurvey.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TextSurvey;

namespace TextSurveyTests
{	

	TEST_CLASS(TextStreamTests)
	{
    private:
	public:		
		TEST_METHOD(RunTestWithAsciiTextStream)
		{
            const uint charCount = 3;
            string s("abc");
            TextStream& ts = AsciiTextStream((uint8*)"abc", 3);
            Assert::AreEqual(charCount, ts.GetLength());
            Assert::AreEqual(0u, ts.GetOffset());
            Assert::AreEqual(0u, ts.GetCharOffset());
            uchar data[charCount];
            Assert::AreEqual(charCount, ts.Next(data, charCount));
            Assert::AreEqual((uchar)'a', data[0]);
            Assert::AreEqual((uchar)'b', data[1]);
            Assert::AreEqual((uchar)'c', data[2]);
            Assert::AreEqual(charCount, ts.Back(charCount));
            Assert::AreEqual(0u, ts.GetOffset());
            Assert::AreEqual(0u, ts.GetCharOffset());
		}	    

        auto EncodeUtf8Char(uint c, uint8* buffer) -> void
        {
            uint count = 1;

            if (c > 0x3FFFFFF)
                count = 6;
            else if (c > 0x1FFFFF)
                count = 5;
            else if (c > 0xFFFF)
                count = 4;
            else if (c > 0x07FF)
                count = 3;
            else if (c > 0x007F)
                count = 2;

            if (count == 1)
            {
                *buffer = (c & 0x0000007F);
                buffer++;
                return;
            }

            uint v = c;
            for (uint i = 1; i < count; i++)
            {
                *buffer = 0x80 | (uint8)(v & 0x3F);
                buffer++;
                v >>= 6;
            }

            uint8 lower = 0;
            for (uint8 i = 0; i < count; i++)
                lower |= (0x80 >> i);
            uint8 upper = 0xFF;
            for (uint8 i = 0; i < count + 1; i++)
                upper ^= (0x80 >> i);
            *buffer = lower | (v & upper);
            buffer++;
            v >>= (8 - (count + 1));
        }

		TEST_METHOD(RunTestWithUtf8TextStream)
		{
            const uint charCount = 3;
            const uint char1 = 0x24;
            const uint char2 = 0xA2;
            const uint char3 = 0x20AC;
            const uint byteCount = 6;
            uint8 cs[byteCount];
            EncodeUtf8Char(char1, cs);
            EncodeUtf8Char(char2, cs + 1);
            EncodeUtf8Char(char3, cs + 3);
            TextStream& ts = Utf8TextStream(cs, byteCount);
            Assert::AreEqual(byteCount, ts.GetLength());
            Assert::AreEqual(0u, ts.GetOffset());
            Assert::AreEqual(0u, ts.GetCharOffset());
            uchar data[charCount];
            Assert::AreEqual(charCount, ts.Next(data, charCount));
            Assert::AreEqual(char1, data[0]);
            Assert::AreEqual(char2, data[1]);
            Assert::AreEqual(char3, data[2]);
            Assert::AreEqual(charCount, ts.Back(charCount));
            Assert::AreEqual(0u, ts.GetOffset());
            Assert::AreEqual(0u, ts.GetCharOffset());
		}
	};
}