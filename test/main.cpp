#include <print>
#include <cassert>
#include <iostream>
#include <ZxCvt/ZxCvt.h>


auto main(void) -> int
{
    try
    {
        ZQF::ZxCvt cvt;

        auto try_cvt = [&cvt](const std::string_view msStr, const size_t nCodePage, const std::u8string_view u8Str, const std::u16string_view u16Str)
            {
                assert(cvt.MBCSToUTF16LE(msStr, nCodePage) == u16Str);
                assert(cvt.NotError() == true);
                assert(cvt.UTF16LEToMBCS(u16Str, nCodePage) == msStr);
                assert(cvt.NotError() == true);
                assert(cvt.MBCSToUTF8(msStr, nCodePage) == reinterpret_cast<const char*>(u8Str.data()));
                assert(cvt.NotError() == true);
                assert(cvt.UTF8ToMBCS(u8Str, nCodePage) == msStr);
                assert(cvt.NotError() == true);
                assert(cvt.UTF8ToUTF16LE(u8Str) == u16Str);
                assert(cvt.NotError() == true);
                assert(cvt.UTF16LEToUTF8(u16Str) == reinterpret_cast<const char*>(u8Str.data()));
                assert(cvt.NotError() == true);
            };

        try_cvt(reinterpret_cast<const char*>("\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB\xB4\xED"), 936, u8"今天天气不错", u"今天天气不错");
        try_cvt(reinterpret_cast<const char*>("\x82\xB7\x82\xDD\x82\xDC\x82\xB9\x82\xF1"), 932, u8"すみません", u"すみません");

        cvt.MBCSToMBCS(reinterpret_cast<const char*>("\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB\xB4\xED"), 936, 932);
        assert(cvt.NotError() == false);
        [[maybe_unused]] auto error = cvt.GetError();

        cvt.MBCSToMBCS(reinterpret_cast<const char*>("\x82\xB7\x82\xDD\x82\xDC\x82\xB9\x82\xF1"), 932, 936);
        assert(cvt.NotError() == true);

        
        [[maybe_unused]]int x = 0;

    }
    catch (const std::exception& err)
    {
        std::println(std::cerr, "std::exception: {}", err.what());
    }
}
