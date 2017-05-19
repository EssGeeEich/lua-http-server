#include "requestmanager.h"
#include <fstream>

struct FileReader {
    inline FileReader(char const* f, std::uint64_t off, std::uint64_t len)
        : m_file(f, std::ios_base::in | std::ios_base::binary),
          m_begin(off), m_end(off + len) {}
    
    std::ifstream m_file;
    
    std::uint64_t m_begin;
    std::uint64_t m_end;
};

static ssize_t FileRead(void* cls, uint64_t pos, char* buf, size_t len)
{
    FileReader* fr = static_cast<FileReader*>(cls);
    
    std::uint64_t maxlen = (fr->m_end - fr->m_begin) - pos;
    if(len > maxlen)
        len = static_cast<size_t>(maxlen);
    
    if(len > 0)
    {
        fr->m_file.seekg(pos);
        fr->m_file.read(buf, len);
        return fr->m_file.gcount();
    }
    return 0;
}

static void FileClose(void* cls)
{
    FileReader* fr = static_cast<FileReader*>(cls);
    if(fr)
        delete fr;
}

bool RequestManager::ResponseT::Cancel()
{
    if(Response())
    {
        MHD_destroy_response(Response());
        SetResponse(nullptr);
        return true;
    }
    return false;
}

bool RequestManager::ResponseT::SSendData(char const* data, uint64_t size)
{
    Cancel();
    return SetResponse(
        MHD_create_response_from_buffer(size,
        const_cast<void*>(static_cast<void const*>(data)), MHD_RESPMEM_MUST_COPY)
    );
}

bool RequestManager::ResponseT::SSendData(std::string const& str)
{
    Cancel();
    return SetResponse(
        MHD_create_response_from_buffer(str.size(),
        const_cast<void*>(static_cast<void const*>(str.data())), MHD_RESPMEM_MUST_COPY)
    );
}

bool RequestManager::ResponseT::SSendFile(char const* path, std::size_t from, std::size_t length)
{
    Cancel();
    return SetResponse(MHD_create_response_from_callback(-1, 65536, FileRead, (new FileReader(path, from, length)), FileClose));
}

bool RequestManager::ResponseT::AddHeader(char const* header, char const* content)
{
    if(!Response())
        return false;
    return MHD_add_response_header(Response(), header, content) == MHD_YES;
}

bool RequestManager::ResponseT::AddFooter(const char *footer, const char *content)
{
    if(!Response())
        return false;
    return MHD_add_response_footer(Response(), footer, content) == MHD_YES;
}

bool RequestManager::ResponseT::RemHeader(const char *header, const char *content)
{
    if(!Response())
        return false;
    return MHD_del_response_header(Response(), header, content) != MHD_NO;
}

bool RequestManager::ResponseT::QRespond(unsigned int code)
{
    if(!Response())
        return false;
    return MHD_queue_response(Connection(), code, Response()) == MHD_YES;
}

bool RequestManager::ResponseT::QBasicLoginDataRequired(char const* realm)
{
    if(!Response())
        return false;
    return MHD_queue_basic_auth_fail_response(Connection(), realm, Response()) == MHD_YES;
}

bool RequestManager::ResponseT::QDigestLoginDataRequired(char const* realm, char const* opaque, bool stale)
{
    if(!Response())
        return false;
    return MHD_queue_auth_fail_response(Connection(), realm, opaque, Response(), stale ? 1 : 0) == MHD_YES;
}
