#include <ZxCvt/ZxCvt.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif __linux__

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

    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB) -> std::string_view
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;
        if (u16Str.empty()) { return std::string_view{ "", 0 }; }
        BOOL not_all_cvt = TRUE;
        const auto bytes = static_cast<size_t>(::WideCharToMultiByte(static_cast<UINT>(nCodePage), 0, reinterpret_cast<const wchar_t*>(u16Str.data()), static_cast<int>(u16Str.size()), nullptr, 0, nullptr, &not_all_cvt));
        if (bytes == 0) { m_eError = ZxCvt::LastError::ERROR_NULL_CHARS; return std::string_view{ "", 0 }; }
        if (not_all_cvt == TRUE) { m_eError = ZxCvt::LastError::ERROR_NOT_ALL_CVT; }
        auto cvt_buffer_ptr = reinterpret_cast<char*>(this->ReSize(bytes + 1, isSlotB));
        const auto bytes_real = static_cast<size_t>(::WideCharToMultiByte(static_cast<UINT>(nCodePage), 0, reinterpret_cast<const wchar_t*>(u16Str.data()), static_cast<int>(u16Str.size()), cvt_buffer_ptr, static_cast<int>(bytes), nullptr, &not_all_cvt));
        cvt_buffer_ptr[bytes_real] = {};
        return std::string_view{ cvt_buffer_ptr, bytes_real };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB) -> std::u16string_view
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;
        if (msStr.empty()) { return std::u16string_view{ u"", 0 }; }
        const auto char_count = static_cast<size_t>(::MultiByteToWideChar(static_cast<UINT>(nCodePage), MB_ERR_INVALID_CHARS, msStr.data(), static_cast<int>(msStr.size()), nullptr, 0));
        if (char_count == 0) { m_eError = ZxCvt::LastError::ERROR_NULL_CHARS; return std::u16string_view{ u"", 0 }; }
        const auto expected_bytes = ((char_count + 1) * sizeof(char16_t));
        auto cvt_buffer_ptr = reinterpret_cast<char16_t*>(this->ReSize(expected_bytes, isSlotB));
        const auto char_count_real = static_cast<size_t>(::MultiByteToWideChar(static_cast<UINT>(nCodePage), MB_ERR_INVALID_CHARS, msStr.data(), static_cast<int>(msStr.size()), reinterpret_cast<wchar_t*>(cvt_buffer_ptr), static_cast<int>(char_count)));
        cvt_buffer_ptr[char_count_real] = {};
        return std::u16string_view{ cvt_buffer_ptr, char_count_real };
    }


    auto ZxCvt::UTF8ToUTF16LE(const std::string_view u8Str) -> std::u16string_view
    {
        return this->MBCSToUTF16LE(u8Str, CP_UTF8);
    }

    auto ZxCvt::UTF8ToUTF16LE(const std::u8string_view u8Str) -> std::u16string_view
    {
        return this->UTF8ToUTF16LE(std::string_view{ reinterpret_cast<const char*>(u8Str.data()), u8Str.size() });
    }

    auto ZxCvt::UTF16LEToUTF8(const std::u16string_view u16Str) -> std::string_view
    {
        return this->UTF16LEToMBCS(u16Str, CP_UTF8);
    }

    auto ZxCvt::MBCSToUTF8(const std::string_view msStr, const size_t nCodePage) -> std::string_view
    {
        return this->MBCSToMBCS(msStr, nCodePage, CP_UTF8);
    }

    auto ZxCvt::UTF8ToMBCS(const std::string_view u8Str, const size_t nCodePage) -> std::string_view
    {
        return this->MBCSToMBCS(u8Str, CP_UTF8, nCodePage);
    }

    auto ZxCvt::UTF8ToMBCS(const std::u8string_view u8Str, const size_t nCodePage) -> std::string_view
    {
        return this->UTF8ToMBCS(std::string_view{ reinterpret_cast<const char*>(u8Str.data()), u8Str.size() }, nCodePage);
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStrA, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view
    {
        return this->UTF16LEToMBCS(this->MBCSToUTF16LE(msStrA, nCodePageA, true), nCodePageB, false);
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
        }

        return "";
    }
}
