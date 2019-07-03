#include "main.h"
#include "task.h"
#include "net.h"
#include "sync_task.h"
#include "db_frsit.h"
#include "record.h"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <curl/curl.h>

size_t writeToString(void *ptr, size_t size, size_t count, void *stream) {
    ((std::string*)stream)->append((char*)ptr, 0, size* count);
    return size* count;
}

void sync_task::run() {}

//二维码用来标识单次游戏，内容包括:场地编号，设备编号，游戏编号，座位编号，二维码生成时间（精确到分）。
//	内容	长度	示例	说明
//1	场地编号	4	0001	编号为0001的项目/场地
//2	设备编号	2	01	编号为01的设备
//3	游戏编号	3	001	编号为001的游戏
//4	座位编号	3	033	编号为033的座位，即33号座椅
//5	场次编号	10	0000000001	编号为1的场次
//以上示例生成的数字字符串如下：
//0001010010330000000001
bool sync_task::get_record_sn(const int i_session_id,
                              const int i_seat_id,
                              std::string& s_record_sn) {

    s_record_sn.clear() ;

    int i_place_id = -1;
    int i_project_id = -1;
    int i_game_id = -1;

    if ( !db_frsit::_instance().get_session_info(i_session_id, i_place_id, i_project_id, i_game_id) ) {
        rlog << "db_frsit::get_session_info() error ." << std::endl ;
        return false ;
    }
    char sn[100] = { 0x00 } ;
    sprintf( sn, "%04d%02d%03d%03d%010d",
             i_place_id,
             i_project_id,
             i_game_id,
             i_seat_id,
             i_session_id) ;
    s_record_sn=sn;

    return true ;
}
