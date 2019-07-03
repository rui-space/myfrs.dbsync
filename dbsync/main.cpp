#include "rui.h"
#include "main_service.h"
#include "threadpool.h"
#include "avatar_loop_task.h"
#include "score_loop_task.h"
#include "db_frsit.h"
#include "main.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/wait.h>

#include <string>
#include <iostream>

#include <curl/curl.h>

pthread_mutex_t m_avatar_lock;
pthread_mutex_t m_score_lock;

void init_lock() {
    pthread_mutex_init( &m_avatar_lock, NULL ) ;
    pthread_mutex_init( &m_score_lock, NULL ) ;
}

void destroy_lock(){
    pthread_mutex_destroy( &m_avatar_lock ) ;
    pthread_mutex_destroy( &m_score_lock ) ;
}


int main( void )
{
    srand( time( NULL ) );

    init_lock();

    if ( !db_frsit::_instance().init() ) {
        rlog << "db_frsit.init() error" << std::endl ;
        destroy_lock();
        return -1 ;
    }
    rlog << "frsit DB init success" << std::endl ;

    if ( !rui::threadpool::_instance().start(100) ) {
        rlog << "rui::threadpool::_instance().start(100) error" << std::endl ;
        destroy_lock();
        return -1 ;
    }
    rlog << "threadpool start success" << std::endl ;

    if ( !main_service::_instance().start() ) {
        rlog << "main_service::_instance().start() error" << std::endl ;
        destroy_lock();
        return -1;
    }
    rlog << "main_service start success" << std::endl ;

    rui::threadpool::_instance().add_task(std::unique_ptr<rui::threadtask>(new avatar_loop_task())) ;
    rui::threadpool::_instance().add_task(std::unique_ptr<rui::threadtask>(new score_loop_task())) ;

    while(true)
        sleep(5) ;

    destroy_lock();

    return 0 ;
}
