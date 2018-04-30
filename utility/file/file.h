#pragma once

#ifndef _NINI_FILE_H_
#define _NINI_FILE_H_

#include <stdio.h>

namespace file {

    //Returns 0 if the file is successfully deleted.
    //Otherwise, it returns - 1 and sets errno either to
    //1.EACCES to indicate that the path specifies a read - only file or the file is open, 
    //2.ENOENT to indicate that the filename or path was not found or that the path specifies a directory.
    //https://msdn.microsoft.com/en-us/library/2da4hk1d.aspx
    int remove(const char *path);

    //Returns 0 if it is successful.
    //On an error, the function returns a nonzero value and sets errno to one of the following values :
    //1.EACCES,File or directory specified by newname already exists or could not be created(invalid path); or oldname is a directory and newname specifies a different path.
    //2.ENOENT,File or path specified by oldname not found.
    //3.EINVAL,Name contains invalid characters.
    //https://msdn.microsoft.com/en-us/library/zw5t957f.aspx
    int rename(const char *old_path, const char *new_path);

    //Returns a pointer to the open file.A null pointer value indicates an error.
    //If filename or mode is NULL or an empty string, these functions trigger the invalid parameter handler, which is described in Parameter Validation.
    //If execution is allowed to continue, these functions return NULL and set errno to EINVAL.
    //The character string mode specifies the kind of access that is requested for the file, as follows.
    //"r",Opens for reading.If the file does not exist or cannot be found, the fopen call fails.
    //"w",Opens an empty file for writing.If the given file exists, its contents are destroyed.
    //"a",Opens for writing at the end of the file(appending) without removing the end - of - file(EOF) marker before new data is written to the file.Creates the file if it does not exist.
    //"r+",Opens for both reading and writing.The file must exist.
    //"w+",Opens an empty file for both reading and writing.If the file exists, its contents are destroyed.
    //"a+",Opens for reading and appending.The appending operation includes the removal of the EOF marker before new data is written to the file.The EOF marker is not restored after writing is completed.Creates the file if it does not exist.*/
    //When a file is opened by using the "a" access type or the "a+" access type, all write operations occur at the end of the file.The file pointer can be repositioned by using fseek or rewind, but is always moved back to the end of the file before any write operation is performed.Therefore, existing data cannot be overwritten.
    //The "a" mode does not remove the EOF marker before it appends to the file.After appending has occurred, the MS - DOS TYPE command only shows data up to the original EOF marker and not any data appended to the file.Before it appends to the file, the "a+" mode does remove the EOF marker.After appending, the MS - DOS TYPE command shows all data in the file.The "a+" mode is required for appending to a stream file that is terminated with the CTRL + Z EOF marker.
    //When the "r+", "w+", or "a+" access type is specified, both reading and writing are enabled(the file is said to be open for "update").However, when you switch from reading to writing, the input operation must encounter an EOF marker.If there is no EOF, you must use an intervening call to a file positioning function.The file positioning functions are fsetpos, fseek, and rewind.When you switch from writing to reading, you must use an intervening call to either fflush or to a file positioning function.
    //In addition to the earlier values, the following characters can be appended to mode to specify the translation mode for newline characters.
    //t
    //Open in text(translated) mode.In this mode, CTRL + Z is interpreted as an EOF character on input.In files that are opened for reading / writing by using "a+", fopen checks for a CTRL + Z at the end of the file and removes it, if it is possible.This is done because using fseek and ftell to move within a file that ends with CTRL + Z may cause fseek to behave incorrectly near the end of the file.
    //In text mode, carriage return每linefeed combinations are translated into single linefeeds on input, and linefeed characters are translated to carriage return每linefeed combinations on output.When a Unicode stream - I / O function operates in text mode(the default), the source or destination stream is assumed to be a sequence of multibyte characters.Therefore, the Unicode stream - input functions convert multibyte characters to wide characters(as if by a call to the mbtowc function).For the same reason, the Unicode stream - output functions convert wide characters to multibyte characters(as if by a call to the wctomb function).
    //b
    //Open in binary(untranslated) mode; translations involving carriage - return and linefeed characters are suppressed.
    //If t or b is not given in mode, the default translation mode is defined by the global variable _fmode.If t or b is prefixed to the argument, the function fails and returns NULL.
    //For more information about how to use text and binary modes in Unicode and multibyte stream - I / O, see Text and Binary Mode File I / O and Unicode Stream I / O in Text and Binary Modes.
    //There are more modes(c,n,N,S,R,T,D,css) not listed here.
    //https://msdn.microsoft.com/en-us/library/yeby3zcb.aspx
    FILE *fopen(const char *path, const char *mode);

    //fclose returns 0 if the stream is successfully closed, return EOF to indicate an error.
    //If stream is NULL, the invalid parameter handler is invoked.If execution is allowed to continue, fclose sets errno to EINVAL and returns EOF.
    //It is recommended that the stream pointer always be checked prior to calling this function.
    //https://msdn.microsoft.com/en-us/library/fxfsw25t.aspx
    int fclose(FILE *stream);

    //The fread function reads up to count items of size bytes from the input stream and stores them in buffer.
    //fread returns the number of full items actually read, which may be less than count if an error occurs or if the end of the file is encountered before reaching count.
    //Use the feof or ferror function to distinguish a read error from an end - of - file condition.If size or count is 0, fread returns 0 and the buffer contents are unchanged.
    //If stream or buffer is a null pointer, fread invokes the invalid parameter handler, as described in Parameter Validation.
    //If execution is allowed to continue, this function sets errno to EINVAL and returns 0.
    //https://msdn.microsoft.com/en-us/library/kt0etdcs.aspx
    size_t fread(void *buffer, size_t size, size_t count, FILE *stream);

    //The fwrite function writes up to count items, of size length each, from buffer to the output stream.
    //fwrite returns the number of full items actually written, which may be less than count if an error occurs. 
    //Also, if an error occurs, the file-position indicator cannot be determined. 
    //If either stream or buffer is a null pointer, or if an odd number of bytes to be written is specified in Unicode mode, the function invokes the invalid parameter handler, as described in Parameter Validation. 
    //If execution is allowed to continue, this function sets errno to EINVAL and returns 0.
    //https://msdn.microsoft.com/en-us/library/h9t88zwz.aspx
    size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream);

    //The feof function returns a nonzero value if a read operation has attempted to read past the end of the file; it returns 0 otherwise. 
    //If the stream pointer is NULL, the function invokes the invalid parameter handler, as described in Parameter Validation. 
    //If execution is allowed to continue, errno is set to EINVAL and the feof returns 0.
    //https://msdn.microsoft.com/en-us/library/xssktc6e.aspx
    int feof(FILE *stream);

    //If no error has occurred on stream, ferror returns 0. Otherwise, it returns a nonzero value. 
    //If stream is NULL, ferror invokes the invalid parameter handler, as described in Parameter Validation. 
    //If execution is allowed to continue, this function sets errno to EINVAL and returns 0.
    //https://msdn.microsoft.com/en-us/library/y2wc3w90.aspx
    int ferror(FILE *stream);

    //The fseek functions moves the file pointer (if any) associated with stream to a new location that is offset bytes from origin. The next operation on the stream takes place at the new location. On a stream open for update, the next operation can be either a read or a write. 
    //The argument origin must be one of the following constants, defined in STDIO.H:
    //SEEK_CUR, Current position of file pointer.
    //SEEK_END, End of file.
    //SEEK_SET, Beginning of file.
    //If successful, fseek returns 0. Otherwise, it returns a nonzero value. 
    //On devices incapable of seeking, the return value is undefined. If stream is a null pointer, or if origin is not one of allowed values described below, fseek invoke the invalid parameter handler, as described in Parameter Validation. 
    //If execution is allowed to continue, these functions set errno to EINVAL and return -1.
    //https://msdn.microsoft.com/en-us/library/75yw9bf3.aspx
    int fseek(FILE *stream,long offset,int origin);

    //ftell return the current file position.The value returned by ftell may not reflect the physical byte offset for streams opened in text mode, because text mode causes carriage return每linefeed translation.
    //Use ftell with fseek to return to file locations correctly.On error, ftell invoke the invalid parameter handler, as described in Parameter Validation.
    //If execution is allowed to continue, these function return 每1L and set errno to one of two constants, defined in ERRNO.H.The EBADF constant means the stream argument is not a valid file pointer value or does not refer to an open file.
    //EINVAL means an invalid stream argument was passed to the function.On devices incapable of seeking(such as terminals and printers), or when stream does not refer to an open file, the return value is undefined.
    //https://msdn.microsoft.com/en-us/library/0ys3hc0b.aspx
    long ftell(FILE *stream);

    //The rewind function repositions the file pointer associated with stream to the beginning of the file. A call to rewind is similar to (void)fseek(stream, 0L, SEEK_SET);
    //However, unlike fseek, rewind clears the error indicators for the stream as well as the end - of - file indicator.Also, unlike fseek, rewind does not return a value to indicate whether the pointer was successfully moved.
    //To clear the keyboard buffer, use rewind with the stream stdin, which is associated with the keyboard by default.
    //If stream is a NULL pointer, the invalid parameter handler is invoked, as described in Parameter Validation.If execution is allowed to continue, this function returns and errno is set to EINVAL.
    void rewind(FILE *stream);

    class File {
    public:
        File();

        FILE *open(const char *path, const char *mode);

        size_t size();

        bool jump(size_t bytes);

        size_t read(void *buffer, size_t size, size_t count);

        size_t write(const void *buffer, size_t size, size_t count);

        int feof();

        int ferror();

        int close();

    protected:
        FILE * m_file;
    };

    class file_reader {
    public:
        file_reader();

        FILE *open(const char *path, const char *mode = "rb");

        size_t size();

        bool jump(size_t bytes);

        size_t read(void *buffer, size_t size, size_t count);

        int feof();

        int ferror();

        int close();

    protected:
        FILE * m_File;
    };

    class file_writer {
    public:
        file_writer();

        FILE *open(const char *path, const char *mode = "wb");

        size_t write(const void *buffer, size_t size, size_t count);

        int close();

    protected:
        FILE * m_File;
    };

   
}

#endif //_NINI_FILE_H_