#include "ZxCvt.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>


namespace ZQF::Zut
{
    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const std::size_t nCodePage, const bool isSlotB) -> std::string_view
    {
        auto fn_print_error = [this, &u16Str, &nCodePage](std::u16string_view u16Msg)
        {
                if (this->GetPrintErrorStatus() == false) { return; }

                std::u16string error_msg;
                error_msg
                    .append(u"ZxCvt::UTF16LEToMBCS(): Warning!\n\t")
                    .append(u"Source: UTF-16   -> Str: ").append(u16Str).append(u" -> Chars: ")
                    .append(reinterpret_cast<const char16_t*>(std::to_wstring(u16Str.size()).c_str())).append(u"\n\t")
                    .append(u"Target: CodePage -> ")
                    .append(reinterpret_cast<const char16_t*>(std::to_wstring(nCodePage).c_str())).append(u"\n\t")
                    .append(u"Reason: ").append(u16Msg);

                const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
                if (handle == INVALID_HANDLE_VALUE) { return; }
                DWORD written{};
                ::WriteConsoleW(handle, error_msg.data(), static_cast<DWORD>(error_msg.size()), &written, nullptr);
        };

        // if empty just return
        if (u16Str.empty()) { return std::string_view{ "", 0 }; }

        // resiz buffer
        const auto buffer_bytes = ((u16Str.size() * sizeof(char16_t)) * 2) + 1;
        char* buffer_ptr = this->ReSize<char*>(buffer_bytes, isSlotB);

        // cvt
        BOOL not_all_cvt = TRUE;
        const auto bytes_written = static_cast<size_t>(::WideCharToMultiByte(static_cast<UINT>(nCodePage), 0, reinterpret_cast<const wchar_t*>(u16Str.data()), static_cast<int>(u16Str.size()), buffer_ptr, static_cast<int>(buffer_bytes), nullptr, &not_all_cvt));

        // check error or not
        this->ClsError();
        if (!bytes_written) { this->SetError(ZxCvt::LastError::ERROR_CVT_FAILED); fn_print_error(u"failed to convert string!\n\n"); }
        if (not_all_cvt) { this->SetError(ZxCvt::LastError::ERROR_NOT_ALL_CVT); fn_print_error(u"not all characters are converted!\n\n"); }

        // make sure end with null char
        buffer_ptr[bytes_written] = {};

        // return string_view
        return std::string_view{ buffer_ptr, bytes_written };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const std::size_t nCodePage, const bool isSlotB) -> std::u16string_view
    {
        auto fn_print_error = [this, &msStr, &nCodePage](std::string_view msMsg)
            {
                if (this->GetPrintErrorStatus() == false) { return; }

                std::string error_msg;
                error_msg
                    .append("ZxCvt::MBCSToUTF16LE(): Warning!\n\t")
                    .append("Source: MBCS     -> Str: ").append(msStr).append(" -> Bytes:")
                    .append(std::to_string(msStr.size()).c_str()).append("\n\t")
                    .append("Target: CodePage -> ")
                    .append(std::to_string(nCodePage).c_str()).append("\n\t")
                    .append("Reason: ").append(msMsg);

                const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
                if (handle == INVALID_HANDLE_VALUE) { return; }
                DWORD written{};
                ::WriteConsoleA(handle, error_msg.data(), static_cast<DWORD>(error_msg.size()), &written, nullptr);
            };

        // if empty just return
        if (msStr.empty()) { return std::u16string_view{ u"", 0 }; }

        // resiz buffer
        const auto buffer_size = ((msStr.size() * sizeof(char)) * 2) + 2;
        auto buffer_ptr = this->ReSize<char16_t*>(buffer_size, isSlotB);

        // cvt
        const auto chars_written = static_cast<std::size_t>(::MultiByteToWideChar(static_cast<UINT>(nCodePage), MB_ERR_INVALID_CHARS, msStr.data(), static_cast<int>(msStr.size()), reinterpret_cast<wchar_t*>(buffer_ptr), static_cast<int>(buffer_size)));

        // check error or not
        this->ClsError();
        if (!chars_written) { this->SetError(ZxCvt::LastError::ERROR_CVT_FAILED); fn_print_error("failed to convert string!\n\n"); }

        // make sure end with null char
        buffer_ptr[chars_written] = {};

        // return string_view
        return std::u16string_view{ buffer_ptr, chars_written };
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStrA, const std::size_t nCodePageA, const std::size_t nCodePageB) -> std::string_view
    {
        return this->UTF16LEToMBCS(this->MBCSToUTF16LE(msStrA, nCodePageA, true), nCodePageB, false);
    }
}
#elif __linux__
#include <iconv.h>
#include <cstring>
#include <string>
#include <charconv>


namespace ZQF::Zut
{
    auto ZxCvt::IConvConv(const void* pSrc, const std::size_t nSrcBytes, const std::size_t nSrcCodePage, const std::size_t nDestCodePage, const std::size_t nDestEleBytes, const bool isSlotB) -> std::size_t
    {
        auto fn_print_error = [this, nSrcCodePage, nDestCodePage, pSrc, nSrcBytes]()
            {
                if (this->GetPrintErrorStatus() == false) { return; }

                std::string error;
                error
                    .append("ZxCvt::IConvConv(): Warning!\n\t")
                    .append("Source: CodePage: ").append(std::to_string(nSrcCodePage))
                    .append(", Data: ").append(reinterpret_cast<const char*>(pSrc), nSrcBytes)
                    .append(", Bytes: ").append(std::to_string(nSrcBytes)).append("\n\t")
                    .append("Target: CodePage: ").append(std::to_string(nDestCodePage)).append("\n\t")
                    .append("Reason: ").append(this->GetLastErrorAsStr());

                ::perror(error.c_str());
            };

        auto fn_code_page_to_str = [](std::size_t nCodePage, char* pBuffer) -> void
            {
                switch (nCodePage)
                {
                case 65001: std::memcpy(pBuffer, "UTF-8", 6); return;
                case 12000: std::memcpy(pBuffer, "UTF-16LE", 9); return;
                }

                pBuffer[0] = 'C';
                pBuffer[1] = 'P';
                const auto [end_ptr, err] = std::to_chars(pBuffer + 2, pBuffer + 15, nCodePage, 10);
                (err != std::errc()) ? pBuffer[0] = '\0' : *end_ptr = '\0';
            };


        this->ClsError();

        if (nSrcBytes == 0) { return 0; }

        // open iconv
        char src_code_page_str_buffer[16], dest_code_page_str_buffer[16];
        fn_code_page_to_str(nDestCodePage, src_code_page_str_buffer);
        fn_code_page_to_str(nSrcCodePage, dest_code_page_str_buffer);
        const auto iconv_handle{ ::iconv_open(src_code_page_str_buffer, dest_code_page_str_buffer) };
        if (iconv_handle == reinterpret_cast<::iconv_t>(-1))
        {
            this->SetError(ZxCvt::LastError::ERROR_INVALID_ENCODING);
            fn_print_error();
            return 0;
        }

        const auto tmp_buf_bytes{ (nSrcBytes * 4) + 2 };
        auto* tmp_buf_ptr{ this->ReSize<std::uint8_t*>(tmp_buf_bytes, isSlotB) };

        // cvt string
        auto cvt_buf{ reinterpret_cast<char*>(tmp_buf_ptr) };
        auto cvt_buf_bytes_remain{ tmp_buf_bytes };
        auto raw_str_buf{ reinterpret_cast<char*>(const_cast<void*>(pSrc)) };
        auto raw_str_bytes_remain{ nSrcBytes };
        const auto iconv_cvt_status{ ::iconv(iconv_handle, &raw_str_buf, &raw_str_bytes_remain, &cvt_buf, &cvt_buf_bytes_remain) };
        const auto iconv_close_status{ ::iconv_close(iconv_handle) };

        // error check
        if (iconv_close_status == -1)
        {
            this->SetError(ZxCvt::LastError::ERROR_CVT_FAILED);
            fn_print_error();
            return 0;
        }
        if (iconv_cvt_status == static_cast<size_t>(-1))
        {
            this->SetError(ZxCvt::LastError::ERROR_CVT_FAILED);
            fn_print_error();
            return 0;
        }
        if (raw_str_bytes_remain != 0)
        {
            this->SetError(ZxCvt::LastError::ERROR_NOT_ALL_CVT);
            fn_print_error();
        }
        if (cvt_buf_bytes_remain < 1)
        {
            this->SetError(ZxCvt::LastError::ERROR_OUT_OF_MEMORY);
            fn_print_error();
            return 0;
        }

        // write null char
        const auto cvted_str_bytes{ tmp_buf_bytes - cvt_buf_bytes_remain };
        cvt_buf[cvted_str_bytes + 0] = {};
        cvt_buf[cvted_str_bytes + 1] = {};

        // cnt elements
        const auto ele_cnt{ cvted_str_bytes / nDestEleBytes };
        return ele_cnt;
    }

    auto ZxCvt::UTF16LEToMBCS(const std::u16string_view u16Str, const std::size_t nCodePage, const bool isSlotB) -> std::string_view
    {
        const auto cvt_str_bytes{ this->IConvConv(u16Str.data(), u16Str.size() * sizeof(char16_t), 12000, nCodePage, sizeof(char), isSlotB) };
        return { this->Ptr<char*>(), cvt_str_bytes };
    }

    auto ZxCvt::MBCSToUTF16LE(const std::string_view msStr, const std::size_t nCodePage, const bool isSlotB) -> std::u16string_view
    {
        const auto cvt_str_bytes{ this->IConvConv(msStr.data(), msStr.size() * sizeof(char), nCodePage, 12000, sizeof(char16_t), isSlotB) };
        return { this->Ptr<char16_t*>(), cvt_str_bytes };
    }

    auto ZxCvt::MBCSToMBCS(const std::string_view msStr, const std::size_t nCodePageA, const std::size_t nCodePageB) -> std::string_view
    {
        const auto cvt_str_bytes{ this->IConvConv(msStr.data(), msStr.size() * sizeof(char), nCodePageA, nCodePageB, sizeof(char)) };
        return { this->Ptr<char*>(), cvt_str_bytes };
    }
}
#endif


namespace ZQF::Zut
{
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

    auto ZxCvt::GetErrorStatus() const -> bool
    {
        return m_eError == ZxCvt::LastError::NOT_ERROR ? false : true;
    }

    auto ZxCvt::GetLastErrorAsStr() const->std::string_view
    {
        switch (m_eError)
        {
        case ZxCvt::ERROR_NOT_ALL_CVT: return "not all characters are converted";
        case ZxCvt::ERROR_OUT_OF_MEMORY: return "out of memory";
        case ZxCvt::ERROR_INVALID_ENCODING: return "invalid encoding";
        case ZxCvt::ERROR_CVT_FAILED: return "convert failed";
        case ZxCvt::NOT_ERROR: return "";
        default: return "";
        }
    }

    auto ZxCvt::ClsError() -> void
    {
        this->SetError(ZxCvt::LastError::NOT_ERROR);
    }

    auto ZxCvt::SetError(const ZxCvt::LastError eError) -> void
    {
        m_eError = eError;
    }

    auto ZxCvt::GetPrintErrorStatus() const -> bool
    {
        return m_isPrintError;
    }

    auto ZxCvt::SetPrintErrorStatus(const bool isStatus) -> void
    {
        m_isPrintError = isStatus;
    }
}
