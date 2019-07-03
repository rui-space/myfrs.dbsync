#include "task.h"
#include "net.h"
#include "db_frsit.h"
#include "record.h"
#include "main.h"

#include "avatar_loop_task.h"
#include "avatar_task.h"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <curl/curl.h>

void avatar_loop_task::run() {
    bool isFirstTime = true ;
    //循环发送头像和分数
    while( true ) {
        if ( !isFirstTime )
            sleep(5);
        isFirstTime = false ;

        //获取所有没有上传的完成的列表
        std::vector<record> v_record;
        if ( !db_frsit::_instance().get_avatar_unupload(v_record) ) {
            rlog << "db_frsit::get_avatar_unupload() error" << std::endl ;
            continue ;
        }

        if (v_record.size()==0) {
            rlog << "all avatar uploaded ." << std::endl ;
            continue ;
        }

        for (auto r: v_record){
//            rlog << "r.mi_session_id=" << r.mi_session_id ;
//            rlog << ", r.mi_seat_id=" << r.mi_session_id ;
//            rlog << ", r.ms_player_avatar=" << r.ms_player_avatar ;
//            rlog << ", r.ms_player_score=" << r.ms_player_score ;

            avatar_task task(r.mi_session_id, r.mi_seat_id, r.ms_player_avatar);
            task.run();
        }
    }

}
