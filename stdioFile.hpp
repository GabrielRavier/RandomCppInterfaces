#pragma once
#include <cstdio>
#include <cstdarg>
#include <string>

namespace ravier
{

enum class SeekOrigin
{
    cur = SEEK_CUR,
    end = SEEK_END,
    set = SEEK_SET,
};

enum class FileMode
{
    // Random read
    read,

    // Sequential read
    scan,

    // Random write to new or truncated file. File is either created or truncated on opening
    write,

    // Random write to new file. If the file exists, an exception is thrown
    writeNew,

    // Random write to existing file. If the file does not exist, an exception is thrown
    writeExisting,

    // Append to new or truncated file. The file pos is set to the end of the file before each write. File is either created or truncated on opening
    append,

    // Append to a new file only. If the file exists, an exception is thrown.
    appendNew,

    // Append to existing file. If the file does not exist, an exception is thrown.
    appendExisting,
};

class StdioFailureException;
class StdioReadWriteFailureException;

class StdioFile
{
private:
    FILE *m_file = nullptr;
    std::string m_name = "";

public:
    using nativeHandle_t = FILE *;  // The type of the underlying file handle. Platform-specific

    // Default constructor. There is no open file initially
    StdioFile() = default;

    // Move-constructor. The moved-from object is resetted to default state - as if default constructed
    StdioFile(StdioFile&& other) : m_file(other.m_file)
    {
        other.m_file = nullptr;
    }

    // Constructor. Opens file immediatly
    StdioFile(const std::string& name, FileMode mode)
    {
        this->open(name, mode);
    }

    // Move-assignment. The moved-from object is resettted to default state - as if default constructed
    StdioFile& operator=(StdioFile&& other)
    {
        if (m_file)
            this->close();
    }

    // Get filename
    std::string name() const
    {
        return m_name;
    }

    // Get native handle associated with the file
    nativeHandle_t nativeHandle() const
    {
        return m_file;
    }

    void throwStdioFailure(const std::string& str) const;
    void throwStdioReadWriteFailure(const std::string& str, size_t numRead) const;

    void checkForValidOpenedFile(const std::string& operationName) const
    {
        if (!m_file)
            this->throwStdioFailure("No file opened when " + operationName);
    }

    void throwStdioFailureWithErrorStr(const std::string& operationName) const
    {
        this->throwStdioFailure(errorStr + operationName);
    }

    // Set native handle associated with the file
    void nativeHandle(nativeHandle_t newFile)
    {
        if (m_file)
            this->close();
        m_file = newFile;
    }

    // Set native handle and filename
    void nativeHandle(nativeHandle_t newFile, const std::string& name)
    {
        this->nativeHandle(newFile);
        m_name = name;
    }

    // Destructor. Automatically closes file
    ~StdioFile()
    {
        this->close();
    }

    // Checks whether a file is open
    bool isOpen() const
    {
        return m_file != nullptr;
    }

    // Close the file if open
    void close()
    {
        if (m_file)
        {
            auto failed = fclose(m_file);
            m_file = nullptr;
            m_name = "";
            if (failed != 0)
                this->throwStdioFailureWithErrorStr("closing file");
        }
    }

    const char *fileModeToStr(FileMode mode) const
    {
        const char *s;
        switch (mode)
        {
            case FileMode::read:
            case FileMode::scan:
                s = "rb";
                break;

            case FileMode::write:
            case FileMode::writeExisting:
                s = "wb";
                break;

            case FileMode::writeNew:
                s = "wbx";
                break;

            case FileMode::append:
            case FileMode::appendExisting:
                s = "ab";
                break;

            case FileMode::appendNew:
                s = "abx";
                break;
        }
        return s;
    }

    // Open a file at the given path with the specified mode.
    void open(const std::string& path, FileMode mode)
    {
        if (m_file)
            this->close();

        const char * s = this->fileModeToStr(mode);

        m_file = fopen(path.c_str(), s);
        m_name = path;
        if (!m_file)
            this->throwStdioFailureWithErrorStr("opening file");
    }

    void setbuf(char *buf) const
    {
        this->checkForValidOpenedFile("setting buf");

       ::setbuf(m_file, buf);
    }

    void setvbuf(char *buf, int mode, size_t size) const
    {
        this->checkForValidOpenedFile("changing buffering mode");

        auto result = ::setvbuf(m_file, buf, mode, size);
    }

    // Return the size of the open file
    int size() const
    {
        this->checkForValidOpenedFile("getting size");

        auto pos = this->tell();
        auto result = this->seek(0, SeekOrigin::end);
        auto size = this->tell();
        this->seek(pos, SeekOrigin::set);
        return size;
    }

    long tell() const
    {
        this->checkForValidOpenedFile("getting position");

        auto pos = ftell(m_file);
        if (pos == -1L)
            throwStdioFailureWithErrorStr("getting position");
    }

    // Return the current position in the open file
    long pos() const
    {
        return this->tell();
    }

    // Change the current position in the open file
    int seek(long offset, SeekOrigin origin) const
    {
        this->checkForValidOpenedFile("seeking");

        auto result = fseek(m_file, offset, (int)origin);
        if (result != 0)
            this->throwStdioFailureWithErrorStr("seeking");
        return result;
    }

    // Read from the open file
    size_t read(void *buf, size_t n) const
    {
        this->checkForValidOpenedFile("reading");

        auto numRead = fread(buf, n, 1, m_file);

        if (this->error())
            this->throwStdioReadWriteFailure(errorStr + "reading", numRead);
    }

    // Write to the open file
    size_t write(const void *buffer, size_t n) const
    {
        this->checkForValidOpenedFile("writing");

        auto numWritten = fwrite(buffer, n, 1, m_file);

        if (this->error())
            this->throwStdioReadWriteFailure(errorStr + "writing", numWritten);
    }

    int vscanf(const std::string& format, va_list args) const
    {
        this->checkForValidOpenedFile("using formatted scan");

        return vfscanf(m_file, format.c_str(), args);
    }

    int scanf(const std::string& format, ...) const
    {
        va_list args;
        va_start(args, format);

        auto result = this->vscanf(format, args);
        
        va_end(args);
        return result;
    }

    int vprintf(const std::string& format, va_list args) const
    {
        this->checkForValidOpenedFile("using formatted print");

        return vfprintf(m_file, format.c_str(), args);
    }

    int printf(const std::string& format, ...) const
    {
        va_list args;
        va_start(args, format);

        auto result = this->vprintf(format, args);

        va_end(args);
        return result;
    }

    bool eof() const
    {
        this->checkForValidOpenedFile("checking for eof");

        return feof(m_file);
    }

    bool error() const
    {
        this->checkForValidOpenedFile("checking for error");

        return ferror(m_file);
    }

    void flush() const
    {
        this->checkForValidOpenedFile("flushing buffer");

        auto ret = fflush(m_file);

        if (ret)
            this->throwStdioFailureWithErrorStr("flushing buffer");
    }

    void clearerr() const
    {
        this->checkForValidOpenedFile("clearing errors");

        ::clearerr(m_file);
    }

    int getc() const
    {
        this->checkForValidOpenedFile("getting single character");

        int result = fgetc(m_file);

        if (result == EOF)
            this->throwStdioFailureWithErrorStr("getting single character");
        return result;
    }

    void getpos(fpos_t *pos) const
    {
        this->checkForValidOpenedFile("getting position indicator");

        auto result = fgetpos(m_file, pos);

        if (result)
            this->throwStdioFailureWithErrorStr("getting position indicator");
    }

    void gets(char *buf, int maxCount) const
    {
        this->checkForValidOpenedFile("getting characters from stream");

        auto result = fgets(buf, maxCount, m_file);

        if (!result)
            this->throwStdioFailureWithErrorStr("getting characters from stream");
    }

    void putc(int ch) const
    {
        this->checkForValidOpenedFile("writing single character");

        auto result = fputc(ch, m_file);

        if (result != ch)
            this->throwStdioFailureWithErrorStr("writing single character");
    }

    void puts(const char *str) const
    {
        this->checkForValidOpenedFile("writing string");

        auto result = fputs(str, m_file);

        if (result == EOF)
           this->throwStdioFailureWithErrorStr("writing single character");
    }

    void ungetc(int ch) const
    {
        this->checkForValidOpenedFile("ungetting character");
        auto result = ::ungetc(ch, m_file);

        if (result != ch)
            this->throwStdioFailureWithErrorStr("ungetting character");
    }

    void setpos(const fpos_t *pos)
    {
        this->checkForValidOpenedFile("setting position indicator");
        auto result = fsetpos(m_file, pos);

        if (result != 0)
            this->throwStdioFailureWithErrorStr("setting position indicator");
    }

    void rewind()
    {
        ::rewind(m_file);
    }

    static constexpr size_t bufferSize = BUFSIZ;
    static constexpr int endOfFile = -1;
    static constexpr size_t filenameMax = FILENAME_MAX;
    static constexpr size_t openMax = FOPEN_MAX;
    static constexpr size_t tmpMax = TMP_MAX;
    const std::string errorStr = "An error occured while ";
};

class StdioFailureException : std::exception
{
private:
    std::string m_info;
    std::string m_name;

protected:
    StdioFailureException()
    {

    }

public:
    StdioFailureException(const std::string& info, const StdioFile& file) : m_info(info), m_name(file.name())
    {

    }

    StdioFailureException(const std::string& info) : m_info(info)
    {

    }

    const char *what() const noexcept
    {
        return (m_info + ". Filename : " + m_name).c_str();
    }

    std::string filename() const noexcept
    {
        return m_name;
    }
};



class StdioReadWriteFailureException : StdioFailureException
{
private:
    size_t m_numRead = 0;
    std::string m_info;
    std::string m_name;

public:
    StdioReadWriteFailureException(const std::string& info, const StdioFile& file, size_t numRead) : m_info(info), m_name(file.name()), m_numRead(numRead)
    {

    }

    const char *what() const noexcept
    {
        return (m_info + ". Filename : \"" + m_name + "\". Number of written characters : " + std::to_string(m_numRead)).c_str();
    }

    std::string filename() const noexcept
    {
        return m_name;
    }

    size_t numRead() const noexcept
    {
        return m_numRead;
    }
};

inline void StdioFile::throwStdioFailure(const std::string& str) const
{
    throw StdioFailureException(str, *this);
}

inline void StdioFile::throwStdioReadWriteFailure(const std::string& str, size_t numRead) const
{
    throw StdioReadWriteFailureException(str, *this, numRead);
}

}