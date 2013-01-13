#pragma once

using namespace std;

namespace TextSurvey
{
    class TextStream 
    {
    protected:

        const uint8* _data;
        const uint _length;
        uint _offset;
        uint _charOffset;

        TextStream(const uint8* data, uint length) :
            _data(data), _length(length), _offset(0u), _charOffset(0u)
        {

        }

    public:

        class Snapshot {
        private:

            TextStream& _stream;
            uint _charOffset;

        public:

            Snapshot(TextStream& stream) : 
                _stream(stream), 
                _charOffset(stream._charOffset)
            {

            }

            void Restore()
            {
                uint diff = _stream._charOffset - _charOffset;
                uint result = _stream.Back(diff);                
                assert(result == diff);
            }
        };

        
        inline Snapshot GetSnapshot()
        {
            TextStream& r = *this;
            return Snapshot(r);
        }

        inline uint GetLength()
        {
            return _length;
        }
        
        inline uint GetOffset()
        {
            return _offset;
        }  
        
        inline uint GetCharOffset()
        {
            return _charOffset;
        }  

        inline uint Next(uchar* buffer)
        {
            return Next(buffer, 1);
        }

        virtual auto Next(uchar* buffer, uint count) -> uint = 0;

        virtual auto Back(uint count) -> uint = 0;
    };

    class AsciiTextStream : public TextStream
    {
    public:
        AsciiTextStream(const uint8* data, uint length) :
            TextStream(data, length)
        {

        }

        auto Next(uchar* buffer, uint count) -> uint
        {
            auto result = _length - _offset;
            if (result > count)
                result = count;
            for (auto i = 0ul; i < result; i++) 
                *(buffer + i) = _data[(unsigned int)(_offset + i)];
            _offset += result;
            _charOffset = _offset;
            return result;
        }

        auto Back(uint count) -> uint
        {
            if (count > _offset)
                count = _offset;
            _offset -= count;
            _charOffset = _offset;
            return count;
        }
    };

    class Utf8TextStream : public TextStream
    {
    public:
        Utf8TextStream(const uint8* data, uint length) :
            TextStream(data, length)
        {

        }

        auto Next(uchar* buffer, uint count) -> uint
        {
            auto offset = _offset;
            auto charOffset = 0;
            auto p = buffer;

            for (auto i = 0ul; i < count && _offset < _length; i++, charOffset++, p++) 
            {
                *p = 0;

                uint count = 0;
                while ((_data[offset] & 0xC0) == 0x80)
                    *p += ((uint)_data[offset++] & (uint)0x3F) << (uint)(6 * count++);

                if ((_data[offset] & 0xF8) == 0xF0)
                    *p += ((uint)_data[offset++] & (uint)0x07) << (uint)(6 * count); 
                else if ((_data[offset] & 0xF0) == 0xE0)
                    *p += ((uint)_data[offset++] & (uint)0x0F) << (uint)(6 * count); 
                else if ((_data[offset] & 0xE0) == 0xC0)
                    *p += ((uint)_data[offset++] & (uint)0x1F) << (uint)(6 * count); 
                else
                    *p += (uint)_data[offset++]; 
            }

            _offset = offset;
            _charOffset += charOffset;
            return charOffset;
        }

        auto Back(uint count) -> uint
        {
            if (count > _charOffset)
                count = _charOffset;
            uint offset = _offset - 1;
            for (uint i = 0; i < count; i++)
            {
                if ((_data[offset] & 0x80) == 0)
                {
                    continue;
                }
                while ((_data[--offset] & 0xC0) == 0x80);
            }
            _charOffset -= count;
            _offset = offset;
            return count;
        }
    };
}