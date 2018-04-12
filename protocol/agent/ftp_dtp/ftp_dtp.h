#pragma once

#ifndef _NINI_FTP_DTP_H_
#define _NINI_FTP_DTP_H_

namespace ftp_dtp {
    enum STATUS {

    };

    enum STURCTURE_TYPE {
        FILE,
        R,
        PAGE
    };

    enum DATA_TYPE {
        STREAM,
        BLOCK,
        C
    };

    class ftp_dtp {
    public:
        ftp_dtp();

        void get_status();
        enum STURCTURE_TYPE get_structure_type();
        enum DATA_TYPE get_data_type();
        void set_structure_type(enum STURCTURE_TYPE sturcture_type);
        void set_data_type(enum DATA_TYPE data_type);

        ~ftp_dtp();
    private:
        STURCTURE_TYPE m_sturcture_type;
        DATA_TYPE m_data_type;
    };
}

#endif // _NINI_FTP_DTP_H_
