#include <ZxCvt/ZxCvt.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#elif __linux__
#include <iconv.h>
#include <cstring>
#endif


namespace ZQF
{
#ifdef _WIN32
    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB) -> std::string_view
    {
        // if empty just return
        if (u16Str.empty()) { return std::string_view{ "", 0 }; }

        // resiz buffer
        const auto buffer_bytes = ((u16Str.size() * sizeof(char16_t)) * 2) + 1;
        char* buffer_ptr = this->ReSize<char*>(buffer_bytes, isSlotB);

        // cvt
        BOOL not_all_cvt = TRUE;
        const auto bytes_written = static_cast<size_t>(::WideCharToMultiByte(static_cast<UINT>(nCodePage), 0, reinterpret_cast<const wchar_t*>(u16Str.data()), static_cast<int>(u16Str.size()), buffer_ptr, static_cast<int>(buffer_bytes), nullptr, &not_all_cvt));

        // check error or not
        this->ErrorClear();
        if (!bytes_written) { this->ErrorSet(ZxCvt::LastError::ERROR_CVT_FAILED); }
        if (not_all_cvt) { this->ErrorSet(ZxCvt::LastError::ERROR_NOT_ALL_CVT); }
        if (m_isPrintError)
        {
            if (this->NotError() == false)
            {
                std::u16string error_msg;
                error_msg
                    .append(u"ZxCvt::UTF16LEToMBCS(): Warning!!\n\t")
                    .append(u"Source: UTF-16   -> Str: ")
                    .append(1, u'"')
                    .append(u16Str)
                    .append(1, u'"')
                    .append(u" -> Chars: ")
                    .append(reinterpret_cast<const char16_t*>(std::to_wstring(u16Str.size()).c_str()))
                    .append(u"\n\t")
                    .append(u"Target: CodePage -> ")
                    .append(reinterpret_cast<const char16_t*>(std::to_wstring(nCodePage).c_str()))
                    .append(u"\n\t")
                    .append(u"Reason: ");

                if (!bytes_written) { error_msg.append(u"failed to convert string!\n\n"); }
                if (not_all_cvt) { error_msg.append(u"not all characters are converted!\n\n"); }

                DWORD written{};
                ::WriteConsoleW(::GetStdHandle(STD_ERROR_HANDLE), error_msg.data(), static_cast<DWORD>(error_msg.size()), &written, nullptr);
            }
        }

        // make sure end with null char
        buffer_ptr[bytes_written] = {};

        // return string_view
        return std::string_view{ buffer_ptr, bytes_written };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB) -> std::u16string_view
    {
        // if empty just return
        if (msStr.empty()) { return std::u16string_view{ u"", 0 }; }

        // resiz buffer
        const auto buffer_size = ((msStr.size() * sizeof(char)) * 2) + 2;
        char16_t* buffer_ptr = this->ReSize<char16_t*>(buffer_size, isSlotB);

        // cvt
        const auto chars_written = static_cast<size_t>(::MultiByteToWideChar(static_cast<UINT>(nCodePage), MB_ERR_INVALID_CHARS, msStr.data(), static_cast<int>(msStr.size()), reinterpret_cast<wchar_t*>(buffer_ptr), static_cast<int>(buffer_size)));

        // check error or not
        this->ErrorClear();
        if (!chars_written) { this->ErrorSet(ZxCvt::LastError::ERROR_CVT_FAILED); }
        if (m_isPrintError)
        {
            if (this->NotError() == false)
            {
                std::string error_msg;
                error_msg
                    .append("ZxCvt::MBCSToUTF16LE(): Warning!!\n\t")
                    .append("Source: MBCS     -> Str: ")
                    .append(1, '"')
                    .append(msStr)
                    .append(1, '"')
                    .append(" -> Bytes:")
                    .append(std::to_string(msStr.size()))
                    .append("\n\t")
                    .append("Target: CodePage -> ")
                    .append(std::to_string(nCodePage))
                    .append("\n\t")
                    .append("Reason: ");

                if (!chars_written) { error_msg.append("failed to convert string!\n\n"); }

                DWORD written{};
                ::WriteConsoleA(::GetStdHandle(STD_ERROR_HANDLE), error_msg.data(), static_cast<DWORD>(error_msg.size()), &written, nullptr);
            }
        }

        // make sure end with null char
        buffer_ptr[chars_written] = {};

        // return string_view
        return std::u16string_view{ buffer_ptr, chars_written };
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStrA, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view
    {
        return this->UTF16LEToMBCS(this->MBCSToUTF16LE(msStrA, nCodePageA, true), nCodePageB, false);
    }
#elif __linux__
    auto ZxCvt::IConvConv(const void* cpSrc, const size_t nSrcBytes, const size_t nSrcCodePage, const size_t nDestCodePage, const size_t nDestEleSize, bool isSlotB) -> size_t
    {
        m_eError = ZxCvt::LastError::NOT_ERROR;

        if (nSrcBytes == 0) { return 0; }

        auto code_page_to_str = [](size_t nCodePage, char* pBuffer) -> void
            {
                switch (nCodePage)
                {
                case 65001: std::memcpy(pBuffer, "UTF-8", 6); break;
                case 12000: std::memcpy(pBuffer, "UTF-16LE", 9); break;
                default: ::snprintf(pBuffer, 15, "CP%d", static_cast<int>(nCodePage));
                }
            };

        char src_code_page_str_buffer[16];
        char dest_code_page_str_buffer[16];
        code_page_to_str(nDestCodePage, src_code_page_str_buffer);
        code_page_to_str(nSrcCodePage, dest_code_page_str_buffer);
        const auto  iconv_handle = ::iconv_open(src_code_page_str_buffer, dest_code_page_str_buffer);
        if (iconv_handle == iconv_t(-1)) { m_eError = ZxCvt::LastError::ERROR_INVALID_ENCODING; return 0; }

        const size_t temp_buffer_bytes = (nSrcBytes * 4) + 2;
        uint8_t* temp_buffer_ptr = this->ReSize<uint8_t*>(temp_buffer_bytes, isSlotB);

        char* out_buffer = reinterpret_cast<char*>(temp_buffer_ptr);
        size_t out_bytes_remain = temp_buffer_bytes;
        char* input_buffer = reinterpret_cast<char*>(const_cast<void*>(cpSrc));
        size_t input_bytes_remain = nSrcBytes;
        const size_t stastu = ::iconv(iconv_handle, &input_buffer, &input_bytes_remain, &out_buffer, &out_bytes_remain);
        ::iconv_close(iconv_handle);
        if (stastu == static_cast<size_t>(-1)) { m_eError = ZxCvt::LastError::ERROR_CVT_FAILED; return 0; }
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
        case ZQF::ZxCvt::ERROR_NOT_ALL_CVT: return "ZxCvt: not all characters are converted";
        case ZQF::ZxCvt::ERROR_OUT_OF_MEMORY: return "ZxCvt: out of memory";
        case ZQF::ZxCvt::ERROR_INVALID_ENCODING: return "ZxCvt: invalid encoding";
        case ZQF::ZxCvt::ERROR_CVT_FAILED: return "ZxCvt: failed to convert string";
        case ZQF::ZxCvt::NOT_ERROR: return "";
        default: return "";
        }
    }

    auto ZxCvt::ErrorClear() -> void
    {
        this->ErrorSet(ZxCvt::LastError::NOT_ERROR);
    }

    auto ZxCvt::ErrorSet(ZxCvt::LastError eError) -> void
    {
        m_eError = eError;
    }

    auto ZxCvt::DisablePrintError() -> void
    {
        m_isPrintError = false;
    }
}
