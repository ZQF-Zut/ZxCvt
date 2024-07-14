#include <ZxCvt/ZxCvt.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif __linux__
#include <cstdio>
#include <iconv.h>
#include <memory.h>
#endif


namespace ZQF
{
    auto ZxCvt::ReSize(size_t nBytes, bool isSlotB) -> uint8_t*
    {
        if (isSlotB)
        {
            if (m_nMemBBytes < nBytes)
            {
                m_upMemB = std::make_unique_for_overwrite<uint8_t[]>(nBytes);
                m_nMemBBytes = nBytes;
            }

            return m_upMemB.get();
        }
        else
        {
            if (m_nMemABytes < nBytes)
            {
                m_upMemA = std::make_unique_for_overwrite<uint8_t[]>(nBytes);
                m_nMemABytes = nBytes;
            }

            return m_upMemA.get();
        }
    }

#ifdef _WIN32
    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB) -> std::string_view
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;
        if (u16Str.empty()) { return std::string_view{ "", 0 }; }
        BOOL not_all_cvt = TRUE;
        const size_t buffer_bytes = ((u16Str.size() * sizeof(char16_t)) * 2) + 1;
        auto cvt_buffer_ptr = reinterpret_cast<char*>(this->ReSize(buffer_bytes, isSlotB));
        const auto bytes_real = static_cast<size_t>(::WideCharToMultiByte(static_cast<UINT>(nCodePage), 0, reinterpret_cast<const wchar_t*>(u16Str.data()), static_cast<int>(u16Str.size()), cvt_buffer_ptr, static_cast<int>(buffer_bytes), nullptr, &not_all_cvt));
        if (not_all_cvt == TRUE) { m_eError = ZxCvt::LastError::ERROR_NOT_ALL_CVT; }
        cvt_buffer_ptr[bytes_real] = {};
        return std::string_view{ cvt_buffer_ptr, bytes_real };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB) -> std::u16string_view
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;
        if (msStr.empty()) { return std::u16string_view{ u"", 0 }; }
        const auto buffer_size = ((msStr.size() * sizeof(char)) * 2) + 2;
        auto cvt_buffer_ptr = reinterpret_cast<char16_t*>(this->ReSize(buffer_size, isSlotB));
        const auto char_count_real = static_cast<size_t>(::MultiByteToWideChar(static_cast<UINT>(nCodePage), MB_ERR_INVALID_CHARS, msStr.data(), static_cast<int>(msStr.size()), reinterpret_cast<wchar_t*>(cvt_buffer_ptr), static_cast<int>(buffer_size)));
        cvt_buffer_ptr[char_count_real] = {};
        return std::u16string_view{ cvt_buffer_ptr, char_count_real };
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStrA, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view
    {
        return this->UTF16LEToMBCS(this->MBCSToUTF16LE(msStrA, nCodePageA, true), nCodePageB, false);
    }
#elif __linux__
    static auto CodePageToStr(size_t nCodePage) -> const char*
    {
        switch (nCodePage)
        {
        case 932: return "CP932";
        case 936: return "CP936";
        case 65001: return "UTF-8";
        case 12000: return "UTF-16LE";
        }

        return "";
    }

    auto ZxCvt::IConvConv(const void* cpSrc, const size_t nSrcBytes, const size_t nSrcCodePage, const size_t nDestCodePage, const size_t nDestEleSize, bool isSlotB) -> size_t
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;

        if (nSrcBytes == 0) { return 0; }

        const auto iconv_handle = ::iconv_open(CodePageToStr(nDestCodePage), CodePageToStr(nSrcCodePage));
        if (iconv_handle == iconv_t(-1)) { m_eError = ZxCvt::LastError::ERROR_INVALID_ENCODING; return 0; }

        const size_t temp_buffer_bytes = (nSrcBytes * 2) + 2;
        uint8_t* temp_buffer_ptr = this->ReSize(temp_buffer_bytes, isSlotB);

        char* out_buffer = reinterpret_cast<char*>(temp_buffer_ptr);
        size_t out_bytes_remain = temp_buffer_bytes;
        char* input_buffer = reinterpret_cast<char*>(const_cast<void*>(cpSrc));
        size_t input_bytes_remain = nSrcBytes;
        const size_t stastu = ::iconv(iconv_handle, &input_buffer, &input_bytes_remain, &out_buffer, &out_bytes_remain);
        ::iconv_close(iconv_handle);
        if (stastu == static_cast<size_t>(-1)) { m_eError = ZxCvt::LastError::ERROR_NULL_CHARS; return 0; }
        if (input_bytes_remain != 0) { m_eError = ZxCvt::LastError::ERROR_NOT_ALL_CVT; }
        if (out_bytes_remain < 1) { m_eError = ZxCvt::LastError::ERROR_OUT_OF_MEMORY; return 0; }

        const size_t cvt_str_bytes = temp_buffer_bytes - out_bytes_remain;
        out_buffer[cvt_str_bytes + 0] = {};
        out_buffer[cvt_str_bytes + 1] = {};
        return cvt_str_bytes / nDestEleSize;
    }


    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB) -> std::string_view
    {
        size_t cvt_str_bytes = this->IConvConv(u16Str.data(), u16Str.size() * sizeof(char16_t), 12000, nCodePage, sizeof(char), isSlotB);
        return { this->Ptr<char*>(), cvt_str_bytes };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB) -> std::u16string_view
    {
        size_t cvt_str_bytes = this->IConvConv(msStr.data(), msStr.size() * sizeof(char), nCodePage, 12000, sizeof(char16_t), isSlotB);
        return { this->Ptr<char16_t*>(), cvt_str_bytes };
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStr, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view
    {
        size_t cvt_str_bytes = this->IConvConv(msStr.data(), msStr.size() * sizeof(char), nCodePageA, nCodePageB, sizeof(char));
        return { this->Ptr<char*>(), cvt_str_bytes };
    }
#endif
    auto ZxCvt::UTF8ToUTF16LE(const std::string_view u8Str) -> std::u16string_view
    {
        return this->MBCSToUTF16LE(u8Str, 65001);
    }

    auto ZxCvt::UTF8ToUTF16LE(const std::u8string_view u8Str) -> std::u16string_view
    {
        return this->UTF8ToUTF16LE(std::string_view{ reinterpret_cast<const char*>(u8Str.data()), u8Str.size() });
    }

    auto ZxCvt::UTF16LEToUTF8(const std::u16string_view u16Str) -> std::string_view
    {
        return this->UTF16LEToMBCS(u16Str, 65001);
    }

    auto ZxCvt::MBCSToUTF8(const std::string_view msStr, const size_t nCodePage) -> std::string_view
    {
        return this->MBCSToMBCS(msStr, nCodePage, 65001);
    }

    auto ZxCvt::UTF8ToMBCS(const std::string_view u8Str, const size_t nCodePage) -> std::string_view
    {
        return this->MBCSToMBCS(u8Str, 65001, nCodePage);
    }

    auto ZxCvt::UTF8ToMBCS(const std::u8string_view u8Str, const size_t nCodePage) -> std::string_view
    {
        return this->UTF8ToMBCS(std::string_view{ reinterpret_cast<const char*>(u8Str.data()), u8Str.size() }, nCodePage);
    }

    auto ZxCvt::NotError() const -> bool
    {
        return m_eError == ZxCvt::LastError::NOT_ERROR ? true : false;
    }

    auto ZxCvt::GetError() const->std::string_view
    {
        switch (m_eError)
        {
        case ZQF::ZxCvt::ERROR_NULL_CHARS: return "ZxCvt: converted length is empty";
        case ZQF::ZxCvt::ERROR_NOT_ALL_CVT: return "ZxCvt: not all characters are converted";
        case ZQF::ZxCvt::ERROR_INVALID_ENCODING: return "ZxCvt: invalid encoding";
        case ZQF::ZxCvt::ERROR_OUT_OF_MEMORY: return "ZxCvt: out of memory";
        }

        return "";
    }
}
