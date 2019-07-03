#include "datetime.h"
#include "db_frsit.h"
#include "main.h"
#include "net.h"

#define OTL_STL

db_frsit::db_frsit()
{
	pthread_mutex_init( &m_lock, NULL ) ;
}

db_frsit::~db_frsit( )
{
	m_db.logoff( );
	pthread_mutex_destroy( &m_lock ) ;
}

string db_frsit::get_login_string( const string &s_db_name, const string &s_ip )
{
    map<string, map<string, string> > mp;
    map<string, map<string, string> >::iterator curiter = mp.end( );

    std::ifstream in( "/etc/odbc.ini", ios::binary );
    if( !in ){
        rlog << "/etc/odbc.ini does not exist!" << std::endl ;
		return "";
	}

    string sLine;
	while( getline( in, sLine ) )
	{
        string sOdbcName;
        string::size_type pos = Substr( sOdbcName, sLine, "[", "]" );
        if( pos != string::npos )
		{
            std::pair<map<string, map<string, string> >::iterator, bool> pr = mp.insert( make_pair( sOdbcName, map<string, string>( ) ) );
			if( pr.second )
				curiter = pr.first;
			else
				curiter = mp.end( );
		}
		else
		{
			while( true )
			{
				string::size_type pos = 0;
				if( ( pos = sLine.find( '\r' ) ) != string::npos )
					sLine.erase( pos, 1 );
				else if( ( pos = sLine.find( ' ' ) ) != string::npos )
					sLine.erase( pos, 1 );
				else if( ( pos = sLine.find( '\t' ) ) != string::npos ) 
					sLine.erase( pos, 1 );
				else
					break;
			}

			if( curiter != mp.end( ) )
			{
				pair<string, string> pr = divideString( sLine, "=" );

				if( pr.first.empty( ) == false ) 
					curiter->second.insert( pr );
			}
		}
	}

	for( map<string, map<string, string> >::iterator iter = mp.begin( ); iter != mp.end( ); ++iter )
	{
		map<string, string> &submp = iter->second;
		string sServerIp = submp["Server"];
		if( sServerIp != s_ip )
			continue;

		string sDatabase = submp["Database"];
		if( sDatabase != s_db_name ) 
			continue;

		return submp["USER"] + "/" + submp["PASSWORD"] + "@" + iter->first;
	}

	return "";
}

bool db_frsit::db_connect( const string &s_db_name, const string &s_ip )
{
	ms_login = db_frsit::get_login_string( s_db_name, s_ip );
	if( ms_login.empty( ) )	{
        rlog << "ms_login empty" << std::endl ;
		return false;	
	}

    std::cout << ms_login << std::endl ;

	try	{
		otl_connect::otl_initialize( 1 ); 

        m_db.rlogon( ms_login.c_str( ) );
        rlog << "DB connect done!" << std::endl ;

		return true;
	}
	catch( otl_exception &err )	{ 
		show_sql_error( err, cout );

        rlog << "intermediato db connect error" << std::endl ;
		return false;
	}

	return true ;
}

void *db_frsit::routine_keepalive(void* arg)
{
	pthread_detach(pthread_self()) ;

    if ( arg == NULL ){
        rlog << "arg = NULL" << std::endl ;
		return NULL ;
	}

	try	{
		db_frsit* p_this = reinterpret_cast<db_frsit*>( arg );
		p_this->entry_keepalive(arg);
	}
    catch( ... ) {
        rlog << "exception occurs" << std::endl ;
	}

	return NULL;	
}

void db_frsit::entry_keepalive(  void* arg ) 
{
    if ( arg == NULL ) {
        rlog << "arg = NULL" << std::endl ;
		return ;
	}

    rlog << "frsit db keepalive thread running ..." << std::endl ;
	
    while( true ) {
        try {
			string s_sql = "select count( * ) from mysql.user";
            rui::mutex lock( m_lock );
			otl_cursor::direct_exec( m_db, s_sql.c_str( ), otl_exception::enabled );
		}
        catch( otl_exception &err ) {
			show_sql_error( err, cout );

			db_frsit* p_this = reinterpret_cast<db_frsit*>( arg );
			p_this->m_db.logoff( );
            p_this->m_db.rlogon( ms_login.c_str( ) );

			sleepx( 1 );
			continue;
		}

		sleepx( 60 * 60 );	
	}
}

bool db_frsit::background_routines( void )
{
	pthread_t tid0 ;
	if ( pthread_create( &tid0, NULL, routine_keepalive, this ) < 0 )
	{
        char msg[1000] = {0x00};
        sprintf(msg,"pthread_create() failed: %s", strerror(errno) );
        rlog << msg << std::endl ;
		return false ;
	}

	return true;
}

bool db_frsit::init( const string &s_db_name, const string &s_ip )
{
    if ( !db_connect( s_db_name, s_ip ) ) {
        rlog << "db_frsit::db_connect() error" << std::endl ;
		return false ;
    }

    if ( !background_routines() ) {
        rlog << "db_frsit::background_routines() error" << std::endl ;
		return false ;
	}

	return true ;
}

bool db_frsit::get_avatar_unupload(std::vector<record>& v_record) {
    try {
        std::string s_sql = "select session_id, seat_id, player_avatar, player_score from record"
                " where player_avatar is not null and player_avatar!='0' and ( is_uploaded is null or is_uploaded&0x01!=0x01)";

        //rlog << s_sql << std::endl ;

        rui::mutex lock( m_lock );
        otl_stream os( 1000, s_sql.c_str( ), m_db );
        while( !os.eof( ) ) {
            record r ;
            os >> r.mi_session_id >> r.mi_seat_id >> r.ms_player_avatar >> r.ms_player_score ;
            v_record.push_back(r) ;
        }
        os.close( );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    return true;
}

bool db_frsit::get_score_unupload(std::vector<record>& v_record) {
    try {
        std::string s_sql = "select session_id, seat_id, player_avatar, player_score from record"
                " where player_score is not null and player_score!='0' and (is_uploaded is null or is_uploaded&0x10!=0x10)";

        //rlog << s_sql << std::endl ;

        rui::mutex lock( m_lock );
        otl_stream os( 1000, s_sql.c_str( ), m_db );
        while( !os.eof( ) ) {
            record r ;
            os >> r.mi_session_id >> r.mi_seat_id >> r.ms_player_avatar >> r.ms_player_score ;
            v_record.push_back(r) ;
        }
        os.close( );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    return true;
}

bool db_frsit::get_session_info( const int i_session_id,
                                 int& i_place_id,
                                 int& i_project_id,
                                 int& i_game_id){
    if ( i_session_id == -1 ) {
        rlog << "session_id = -1" << std::endl ;
        return false ;
    }

    i_place_id = -1;
    i_project_id = -1;
    i_game_id = -1;

    try {
        string s_sql = "select place_id, project_id, game_id from session where id=" + lexical_cast<std::string>(i_session_id) ;
        rui::mutex lock( m_lock );
        otl_stream os( 1, s_sql.c_str( ), m_db );
        os >> i_place_id >> i_project_id >> i_game_id ;
        os.close( );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    if ( i_place_id == -1 ||
         i_project_id == -1 ||
         i_game_id == -1 ) {
        rlog << "The data is incomplete in table session" << std::endl ;
        return false ;
    }

    return true;
}

bool db_frsit::set_score_uploaded(const int i_session_id,
                                 const int i_seat_id) {
    if ( i_session_id == -1 ||
         i_seat_id == -1 ) {
        rlog << "args error ." << std::endl ;
        return false ;
    }    

    try {
        string s_sql = "update record set is_uploaded=0x10 where is_uploaded is null";
        s_sql += " and session_id=" + lexical_cast<std::string>(i_session_id) ;
        s_sql += " and seat_id=" + lexical_cast<std::string>(i_seat_id) ;
        rui::mutex lock( m_lock );
        otl_cursor::direct_exec( m_db, s_sql.c_str( ), otl_exception::enabled );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    try {
        string s_sql = "update record set is_uploaded=(is_uploaded|0x10) where";
        s_sql += " session_id=" + lexical_cast<std::string>(i_session_id) ;
        s_sql += " and seat_id=" + lexical_cast<std::string>(i_seat_id) ;
        rui::mutex lock( m_lock );
        otl_cursor::direct_exec( m_db, s_sql.c_str( ), otl_exception::enabled );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    return true ;
}

bool db_frsit::set_avatar_uploaded(const int i_session_id,
                                 const int i_seat_id) {
    if ( i_session_id == -1 ||
         i_seat_id == -1) {
        rlog << "args error ." << std::endl ;
        return false ;
    }

    try {
        string s_sql = "update record set is_uploaded=0x01 where is_uploaded is null";
        s_sql += " and session_id=" + lexical_cast<std::string>(i_session_id) ;
        s_sql += " and seat_id=" + lexical_cast<std::string>(i_seat_id) ;
        rui::mutex lock( m_lock );
        otl_cursor::direct_exec( m_db, s_sql.c_str( ), otl_exception::enabled );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    try {
        string s_sql = "update record set is_uploaded=(is_uploaded|0x01) where";
        s_sql += " session_id=" + lexical_cast<std::string>(i_session_id) ;
        s_sql += " and seat_id=" + lexical_cast<std::string>(i_seat_id) ;
        rui::mutex lock( m_lock );
        otl_cursor::direct_exec( m_db, s_sql.c_str( ), otl_exception::enabled );
    }
    catch( otl_exception &err ) {
        show_sql_error( err, cout );
        return false ;
    }

    return true ;
}
