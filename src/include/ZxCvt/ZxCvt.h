#pragma once
#include <memory>
#include <string_view>


namespace ZQF
{
    class ZxCvt
    {
    private:
        enum LastError
        {
            NOT_ERROR,
            ERROR_NULL_CHARS,
            ERROR_NOT_ALL_CVT
        };
    private:
        size_t m_nMemABytes{};
        size_t m_nMemBBytes{};
        std::unique_ptr<uint8_t[]> m_upMemA;
        std::unique_ptr<uint8_t[]> m_upMemB;
        ZxCvt::LastError m_eError{};

    public:
        ZxCvt() {}

    private:
        auto ReSize(size_t nBytes, bool isSlotB) -> uint8_t*;

    public:
        auto UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB = false) -> std::string_view;
        auto MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB = false) -> std::u16string_view;

        auto UTF16LEToUTF8(const std::u16string_view u16Str) -> std::string_view;
        auto UTF8ToUTF16LE(const std::string_view u8Str) -> std::u16string_view;
        auto UTF8ToUTF16LE(const std::u8string_view u8Str) -> std::u16string_view;
        auto UTF8ToMBCS(const std::string_view u8Str, const size_t nCodePage) -> std::string_view;
        auto UTF8ToMBCS(const std::u8string_view u8Str, const size_t nCodePage) -> std::string_view;
        auto MBCSToUTF8(const std::string_view msStr, const size_t nCodePage) -> std::string_view;
        auto MBCSToMBCS(const std::string_view msStrA, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view;

    public:
        auto NotError() const -> bool;
        auto GetError() const->std::string_view;
    };
}
