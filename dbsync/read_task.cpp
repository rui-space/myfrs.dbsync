#include "main.h"
#include "crypto.h"
#include "task.h"
#include "net.h"
#include "parse_task.h"
#include "read_task.h"
#include "main_service.h"

void read_task::run()
{
    rui::mutex(mp_task.get()->m_lock);

    mp_task.get()->m_time = time(NULL) ;

    int ret = 0 ;

    {
        char msg[1000] = {0x00};
        sprintf(msg,"read(%d)", mp_task.get()->mi_fd);
        rlog << msg << std::endl ;
    }

    while ( true )
    {
        if ( mp_task.get()->mb_closed )
            goto quit;

        char buffer[SOCKET_BUFFER_SIZE] = { 0x00 } ;
        ret = ::read( mp_task.get()->mi_fd, buffer, SOCKET_BUFFER_SIZE ) ;
        if( ret < 0 )
        {
            if( errno == EINTR )
                continue ;

            if ( errno == EAGAIN )
                break;

            {
                char msg[1000] = {0x00};
                sprintf(msg,"read(%d) error : %s", mp_task.get()->mi_fd, strerror( errno ));
                rlog << msg << std::endl ;
            }

            mp_task.get()->mb_closed = true ;
            break ;
        }

        if ( ret == 0 )
        {
            {
                char msg[1000] = {0x00};
                sprintf(msg,"read(%d) peer closed", mp_task.get()->mi_fd);
                rlog << msg << std::endl ;
            }

            mp_task.get()->mb_closed = true ;
            break ;
        }

        copy( buffer, buffer+ret, back_inserter( mp_task.get()->mv_data ) ) ;
    }


    //数据没有收完
    if ( (int)mp_task.get()->mv_data.size() < 7 )
    {
        {
            char msg[1000] = {0x00};
            sprintf(msg,"read(%d) task protocol not over", mp_task.get()->mi_fd);
            rlog << msg << std::endl ;
        }

        if ( mp_task.get()->mb_closed )
            goto quit;

        return;
    }

    //判断是否是http协议
    if ( mp_task.get()->mv_data[0] != 'G' ||
         mp_task.get()->mv_data[1] != 'E' ||
         mp_task.get()->mv_data[2] != 'T' )
    {
        {
            char msg[1000] = {0x00};
            sprintf(msg,"read(%d) task protocol error", mp_task.get()->mi_fd);
            rlog << msg << std::endl ;
        }

        rui::http::write_failure(mp_task.get()->mi_fd, "task protocol error");
        goto quit ;
    }

    {
        const char* s_key = "\r\n\r\n" ;
        std::vector<char> v_key(s_key, s_key+strlen(s_key)) ;
        std::vector<char>::iterator it = search(mp_task.get()->mv_data.begin(), mp_task.get()->mv_data.end(), v_key.begin(), v_key.end() ) ;
        if( it == mp_task.get()->mv_data.end() )
        {
            {
                char msg[1000] = {0x00};
                sprintf(msg,"main(%d) protocol error : main protocol error", mp_task.get()->mi_fd);
                rlog << msg << std::endl ;
            }

            rui::http::write_failure(mp_task.get()->mi_fd, "protocol error : main protocol error");
            goto quit ;
        }

        mp_task.get()->mv_data.erase( it+4, mp_task.get()->mv_data.end() ) ;
    }

    {
        char msg[1000] = {0x00};
        sprintf(msg,"read main(%d) data", mp_task.get()->mi_fd );
        rlog << msg << std::endl ;
    }

    rui::threadpool::_instance().add_task(std::unique_ptr<rui::threadtask>(new parse_task(mp_task))) ;
    return ;

quit:
    main_service::_instance().del_task(mp_task);
}

