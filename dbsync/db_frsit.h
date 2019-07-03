#ifndef _DB_FRSIT_H_
#define _DB_FRSIT_H_

#include "lock.h"
#include "db.h"
#include "record.h"

#include <iostream>
#include <string>

using namespace std;

const string gs_db_name = "frsit";
const string gs_db_host = "10.10.10.13";

class db_frsit
{
public:
    db_frsit();
    ~db_frsit();

    db_frsit(const db_frsit&);
    db_frsit& operator=(const db_frsit&);

    string get_login_string( const string &s_db_name, const string &s_ip = gs_db_host );
    bool db_connect( const string &s_db_name, const string &s_ip  ) ;
    static void *routine_keepalive(void* arg);
    void entry_keepalive( void* arg ) ;
    bool background_routines( void ) ;
    bool init( const string &s_db_name = gs_db_name, const string &s_ip = gs_db_host );

    bool get_avatar_unupload(std::vector<record>& v_record);
    bool get_score_unupload(std::vector<record>& v_record);

    bool get_session_info( const int i_session_id,
                           int& i_place_id,
                           int& i_project_id,
                           int& i_game_id);
    bool set_avatar_uploaded(const int i_session_id,
                           const int i_seat_id);
    bool set_score_uploaded(const int i_session_id,
                           const int i_seat_id);

public:
    static db_frsit& _instance() {
        static db_frsit instance ;
        return instance ;
    }

public:
    otl_connect m_db;
    pthread_mutex_t m_lock;
    string ms_login ;
};

#endif
