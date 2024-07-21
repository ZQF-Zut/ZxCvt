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
        try_cvt(reinterpret_cast<const char*>("123"), 932, u8"123", u"123");

        cvt.MBCSToMBCS(reinterpret_cast<const char*>("\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB\xB4\xED"), 936, 932);
        assert(cvt.NotError() == false);
        [[maybe_unused]] auto error = cvt.GetError();

        cvt.MBCSToMBCS(reinterpret_cast<const char*>("\x82\xB7\x82\xDD\x82\xDC\x82\xB9\x82\xF1"), 932, 936);
        assert(cvt.NotError() == true);


        [[maybe_unused]] auto cvtx = cvt.UTF8ToUTF16LE(u8"😈121😀哈哈哈，*(#Y*(藜");
        assert(cvt.NotError() == true);


        std::string_view xxc = "\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB\xB4\xED";
        cvt.MBCSToUTF8(xxc, 932);

        [[maybe_unused]] auto xx =cvt.MBCSToMBCS(reinterpret_cast<const char*>("\x82\xB7\x82\xDD\x82\xDC\x82\xB9\x82\xF1"), 10086, 1008611);

        [[maybe_unused]] int x = 0;

        cvt.MBCSToUTF8("\x81\x41\x00", 932);



    }
    catch (const std::exception& err)
    {
        std::println(std::cerr, "std::exception: {}", err.what());
    }
}


// #include <iconv.h>
// #include <iostream>
// #include <string.h>
// #include <malloc.h>

// int code_convert(const char* from_charset, const char* to_charset, char* inbuf, size_t inlen,
//     char* outbuf, size_t outlen) {
//     iconv_t cd;
//     char** pin = &inbuf;
//     char** pout = &outbuf;

//     cd = iconv_open(to_charset, from_charset);
//     if (cd == 0)
//         return -1;

//     memset(outbuf, 0, outlen);

//     if ((int)iconv(cd, pin, &inlen, pout, &outlen) == -1)
//     {
//         iconv_close(cd);
//         return -1;
//     }
//     iconv_close(cd);

//     return 0;
// }



// int main()
// {
//     char16_t u8_str[] = u"测试";
//     char gbk_str[100]{};

//     code_convert("UTF-16LE", "CP936", (char*)u8_str, sizeof(u8_str), gbk_str, sizeof(gbk_str));

//     int x = 0;
// }
