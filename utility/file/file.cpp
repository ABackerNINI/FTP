#include "file.h"

inline int file::remove(const char *path) {
    return ::remove(path);
}

inline int file::rename(const char *old_path, const char *new_path) {
    return ::rename(old_path, new_path);
}

inline FILE *file::fopen(const char *path, const char *mode) {
    return ::fopen(path, mode);
}

inline int file::fclose(FILE *stream) {
    return ::fclose(stream);
}

inline size_t file::fread(void *buffer, size_t size, size_t count, FILE *stream) {
    return ::fread(buffer, size, count, stream);
}

inline size_t file::fwrite(const void *buffer, size_t size, size_t count, FILE *stream) {
    return ::fwrite(buffer, size, count, stream);
}

inline int file::feof(FILE *stream) {
    return ::feof(stream);
}

inline int file::ferror(FILE *stream) {
    return ::ferror(stream);
}

inline int file::fseek(FILE *stream, long offset, int origin) {
    return ::fseek(stream, offset, origin);
}

inline long file::ftell(FILE *stream) {
    return ::ftell(stream);
}

inline void file::rewind(FILE *stream) {
    ::rewind(stream);
}

/*
 *  file_reader
 */
file::file_reader::file_reader() :m_File(NULL) {
}

FILE *file::file_reader::open(const char *path, const char *mode) {
    if (m_File) {
        close();
    }

    m_File = ::fopen(path, mode);

    return m_File;
}

size_t file::file_reader::size() {
    ::fseek(m_File, 0, SEEK_END);
    size_t size = ::ftell(m_File);
    ::rewind(m_File);

    return size;
}

bool file::file_reader::jump(size_t bytes) {
    return false;
}

size_t file::file_reader::read(void *buffer, size_t size, size_t count) {
    return ::fread(buffer, size, count, m_File);
}

int file::file_reader::feof() {
    return ::feof(m_File);
}

int file::file_reader::ferror() {
    return ::ferror(m_File);
}

int file::file_reader::close() {
    int ret = 0;
    if (m_File) {
        ret = ::fclose(m_File);
        m_File = NULL;
    }
    return ret;
}

/*
 *  file_writer
 */
file::file_writer::file_writer() :m_File(NULL) {
}

FILE *file::file_writer::open(const char *path, const char *mode) {
    if (m_File) {
        ::fclose(m_File);
    }
    m_File = ::fopen(path, mode);

    return m_File;
}

size_t file::file_writer::write(const void *buffer, size_t size, size_t count) {
    return ::fwrite(buffer, size, count, m_File);
}

int file::file_writer::close() {
    int ret = 0;
    if (m_File) {
        ret = ::fclose(m_File);
        m_File = NULL;
    }
    return ret;
}