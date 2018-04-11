#include "File.h"

File::File() :m_File(NULL) {
}

bool File::Open(const char * _Path, const char *_Mode) {
    if (m_File) {
        Close();
    }

    m_File = fopen(_Path, _Mode);

    return m_File;
}

bool File::Jump(int _Bytes) {
    return false;
}

int File::Read() {
    return 0;
}

bool File::Close() {
    if (m_File) {
        return fclose(m_File) == 0;
    }
    return true;
}

int File::Delete(const char * _Path) {
    return remove(_Path);
}
