# ZxCvt
Text Converter

## Example
```cpp
#include <print>
#include <cassert>
#include <iostream>
#include <Zut/ZxCvt.h>


auto main(void) -> int
{
    ZxCvt cvt;

    // "今天天气不错" -> encoding: CP936
    const std::string_view cp936_sv{ "\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB\xB4\xED" };

    // mbcs <-> utf16le
    const auto u16_sv = cvt.MBCSToUTF16LE(cp936_sv, 936);
    const auto u16_to_mbcs_sv = cvt.UTF16LEToMBCS(u16_sv, 936);
    assert(cp936_sv == u16_to_mbcs_sv);

    // mbcs <-> utf8
    const auto mbcs_to_u8_sv = cvt.MBCSToUTF8(cp936_sv, 936);
    const auto u8_to_mbcs_sv = cvt.UTF8ToMBCS(mbcs_to_u8_sv, 936);
    assert(cp936_sv == u8_to_mbcs_sv);

    // utf8 <-> utf16le
    const auto u8_to_u16_sv = cvt.UTF8ToUTF16LE(u8"今天天气不错");
    assert(std::u16string_view{ u"今天天气不错" } == u8_to_u16_sv);
    const auto u16_to_utf8_sv = cvt.UTF16LEToUTF8(u"今天天气不错");
    assert(std::string_view{ reinterpret_cast<const char*>(u8"今天天气不错") } == u16_to_utf8_sv);

    // mbcs <-> mbcs
    // "今天天气不" -> encoding: CP936
    const auto mbcs_to_mbcs_sv = cvt.MBCSToMBCS("\xBD\xF1\xCC\xEC\xCC\xEC\xC6\xF8\xB2\xBB", 936, 932);
    assert(std::string_view{ "\x8D\xA1\x93\x56\x93\x56\x9F\x83\x95\x73" } == mbcs_to_mbcs_sv);
}

```
