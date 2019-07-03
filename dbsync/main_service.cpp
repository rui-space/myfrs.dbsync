#include <map>

#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/json.h>

#include "task.h"
#include "net.h"
#include "main_service.h"
#include "read_task.h"
#include "main.h"

short gsh_main_service_listen_port = 9000 ;
int gi_client_idle = 20 ;

main_service::main_service()
{
}	

main_service::~main_service( )
{
//	对于智能指针，没有删除指针操作。因此std::queue，map，vector等，都不需要循环删除各个元素指针。
    rlog << "main_service quit ..." << std::endl ;
}


bool main_service::start(void)
{
    if ( !mservice_epoll.create(gsh_main_service_listen_port) ) {
		rlog << "mservice_epoll.create() error" << std::endl ;
		return false ;
	}

	pthread_t tid_epoll_clear ;
	if ( pthread_create(&tid_epoll_clear, NULL, epoll_clear_routine, this) != 0 )
	{

        char msg[1000] = {0x00};
        sprintf(msg,"pthread_create() : epoll_clear_routine : %s", strerror( errno ));
        rlog << msg << std::endl ;
		return false;
	}

	pthread_t tid_epoll_wait ;
	if ( pthread_create(&tid_epoll_wait, NULL, epoll_wait_routine, this) != 0 )
	{
        char msg[1000] = {0x00};
        sprintf(msg,"pthread_create() : epoll_wait_routine : %s", strerror( errno ));
        rlog << msg << std::endl ;
		return false;
	}	

	pthread_t tid_epoll_accept ;
	if ( pthread_create(&tid_epoll_accept, NULL, epoll_accept_routine, this) != 0 )
	{
        char msg[1000] = {0x00};
        sprintf(msg,"pthread_create() : epoll_accept_routine : %s", strerror( errno ));
        rlog << msg << std::endl ;
		return false;
	}

	return true ;
}

void main_service::add_task( const int i_socket )
{
    auto it = mm_task.find(i_socket) ;
    if ( it == mm_task.end() )
		mm_task[i_socket] = std::shared_ptr<rui::task>(new rui::task());

    auto p_task = mm_task.find(i_socket)->second ;
	rui::mutex(p_task.get()->m_lock) ;
	p_task.get()->init(i_socket);
	mservice_epoll.event_add(i_socket) ;
}

void main_service::del_task(std::shared_ptr<rui::task> p_task)
{
	mservice_epoll.event_del(p_task.get()->mi_fd);
	p_task.get()->clear();
}

void main_service::del_task( const int i_socket )
{
	if ( i_socket == -1 )
		return ;
		
    auto it = mm_task.find(i_socket) ;
    if ( it == mm_task.end() )
		return ;
		
	auto p_task = it->second ;
	rui::mutex(p_task.get()->m_lock) ;
	del_task(p_task);
}

void* main_service::epoll_accept_routine(void* arg)
{
	pthread_detach(pthread_self());

    if ( arg == NULL ) {
        rlog << "argument = NULL" << std::endl ;
		return NULL ;
	}
	
	main_service* p_this = reinterpret_cast<main_service*>( arg );
	p_this->epoll_accept_entry();

	return NULL;
}

void main_service::epoll_accept_entry(void)
{
    while( true ) {
		int i_socket = mservice_epoll.accept() ;
        if ( i_socket < 0 ) {
			rlog << "mservice_epoll.accept() error" << std::endl ;
			continue ;
		}

		add_task(i_socket) ;
	}
}

void* main_service::epoll_wait_routine(void* arg)
{
	pthread_detach(pthread_self());

    if ( arg == NULL ) {
        rlog << "argument = NULL" << std::endl ;
		return NULL ;
	}
	
	main_service* p_this = reinterpret_cast<main_service*>( arg );
	p_this->epoll_wait_entry();

	return NULL;
}

void main_service::epoll_wait_entry(void)
{
    while(true)  {
		int nfds = mservice_epoll.wait() ;

		if(nfds == 0) 
			continue;

        if(nfds < 0) {
            rlog << "mservice_epoll.wait() error" << std::endl ;
			continue ;
		}

        for(int i = 0; i < nfds; ++i) {
            int i_socket = mservice_epoll.mresult_event[i].data.fd ;

            {
                char msg[1000] = {0x00};
                sprintf(msg,"main-task(%d) event", i_socket);
                rlog << msg << std::endl ;
            }

            if ( mservice_epoll.mresult_event[i].events & EPOLLERR ) {
                {
                    char msg[1000] = {0x00};
                    sprintf(msg,"main-task(%d) error event", i_socket);
                    rlog << msg << std::endl ;
                }

				del_task(i_socket);
				continue ;
			}
			
			rui::threadpool::_instance().add_task(
						std::unique_ptr<rui::threadtask>(
							new read_task(mm_task[i_socket])
							)
						);
		}
	}
}

void* main_service::epoll_clear_routine(void* arg)
{
	pthread_detach(pthread_self());

    if ( arg == NULL ) {
        rlog << "argument = NULL" << std::endl ;
		return NULL ;
	}
	
	main_service* p_this = reinterpret_cast<main_service*>( arg );
	p_this->epoll_clear_entry();

	return NULL;
}

void main_service::epoll_clear_entry(void)
{
    while(true)  {

        for( auto it : mm_task ) {
			auto p_task = it.second ;
			rui::mutex(p_task->m_lock) ;
			if( p_task.get()->mi_fd == -1 )
				continue ;
				
            if( time(NULL) - p_task.get()->m_time > gi_client_idle ) {
                char msg[1000] = {0x00};
                sprintf(msg,"task(%d) idle timeout error", p_task.get()->mi_fd);
                rlog << msg << std::endl ;
				rui::json::write_failure( p_task.get()->mi_fd, "idle timeout error" ) ;

				del_task(p_task);
			}
		}

		sleep(gi_client_idle) ;
	}
}
