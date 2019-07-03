#include "main.h"
#include "task.h"
#include "net.h"
#include "parse_task.h"
#include "main_service.h"
#include "avatar_task.h"
#include "score_task.h"

void parse_task::run( )
{
    rui::mutex(mp_task.get()->m_lock);

    if ( mp_task.get()->mb_closed )
        goto quit;

    std::cout << mp_task.get()->mv_data << std::endl ;

    //解析http头
    {
        std::string s_vpath;
        rui::http::get_vpath( s_vpath, std::string( mp_task.get()->mv_data.begin() , mp_task.get()->mv_data.end() ), rui::http::GET );

        mp_task.get()->mv_data.assign( s_vpath.begin(), s_vpath.end() ) ;

        URLContainer map_url_arg;
        rui::http::parse_url( map_url_arg, s_vpath );
        std::string s_do( get_map_value( map_url_arg, rui::http::VPATH ) );

        if( s_do == "avatar.do" ) {
            std::string s_session_id = get_map_value( map_url_arg, std::string( "session_id" ) );
            std::string s_seat_id = get_map_value( map_url_arg, std::string( "seat_id" ) );
            std::string s_player_avatar = get_map_value( map_url_arg, std::string( "player_avatar" ) );

            if( s_session_id.empty() ||
                    s_seat_id.empty() ||
                    s_player_avatar.empty())
            {                
                char msg[1000] = {0x00};
                sprintf(msg,"parse_task(%d) http protocol error", mp_task.get()->mi_fd);
                rlog << msg << std::endl ;
                goto quit;
            }

            rui::threadpool::_instance().add_task(
                        std::unique_ptr<rui::threadtask>(
                            new avatar_task(atoi(s_session_id.c_str()), atoi(s_seat_id.c_str()), s_player_avatar))) ;
            goto quit;
        }

        if( s_do == "score.do" ) {
            std::string s_session_id = get_map_value( map_url_arg, std::string( "session_id" ) );
            std::string s_seat_id = get_map_value( map_url_arg, std::string( "seat_id" ) );
            std::string s_player_score = get_map_value( map_url_arg, std::string( "player_score" ) );

            if( s_session_id.empty() ||
                    s_seat_id.empty() ||
                    s_player_score.empty())
            {
                char msg[1000] = {0x00};
                sprintf(msg,"parse_task(%d) http protocol error", mp_task.get()->mi_fd);
                rlog << msg << std::endl ;
                goto quit;
            }

            rui::threadpool::_instance().add_task(
                        std::unique_ptr<rui::threadtask>(
                            new score_task(atoi(s_session_id.c_str()), atoi(s_seat_id.c_str()), s_player_score))) ;
            goto quit;
        }
    }

quit:
    main_service::_instance().del_task(mp_task);
}


