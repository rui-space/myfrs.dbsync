#include "task.h"
#include "net.h"
#include "score_task.h"
#include "db_frsit.h"
#include "record.h"
#include "main.h"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <curl/curl.h>


void score_task::run() {
    rui::mutex lock( m_score_lock );

    //rlog << "score_task : mi_session_id=" << mi_session_id << ", mi_seat_id=" << mi_seat_id << ", ms_player_score=" << ms_player_score << std::endl ;

    std::string s_record_sn;

    if ( !get_record_sn(mi_session_id, mi_seat_id, s_record_sn) ) {
        rlog << "get_record_sn error" << std::endl ;
        return ;
    }

    if ( !upload(s_record_sn, ms_player_score) ) {
        rlog << "score upload error: session_id=" << mi_session_id << ", seat_id=" << mi_seat_id << ", score=" << ms_player_score << std::endl ;
        return ;
    }

    if ( !db_frsit::_instance().set_score_uploaded(mi_session_id, mi_seat_id) ) {
        rlog << "db_frsit::set_score_uploaded() error" << std::endl ;
        return ;
    }

    rlog << "score upload success : session_id=" << mi_session_id << ", seat_id=" << mi_seat_id << ", score=" << ms_player_score << std::endl ;
}

//    GET 	https://frs.chinahost.net.cn/api.php/Public/gameRecordAdd
//    请求参数（GET传参）
//    名称	类型	说明
//    record_sn	varchar(30)	游戏记录唯一ID，也就是贵公司之前提供的二维码说明文档里生成的唯一ID,例如（0001010010331904151243）
//    score	int(10)	游戏得分
//
//    返回值（JSON）
//    status	tinyint(1)	1添加成功 0添加失败
//    msg	varchar(50)	提示信息

bool score_task::upload(const std::string& s_record_sn,
                        const std::string& s_player_score)
{
    std::string s_url = SCORE_UPLOAD_URL ;
    s_url += "?record_sn=" ;
    s_url += s_record_sn ;
    s_url += "&score=";
    s_url += s_player_score;

    CURL* curl_handle = curl_easy_init();    // 初始化
    if ( curl_handle == nullptr ) {
        rlog << "curl_easy_init() error" << std::endl ;
        return false ;
    }

    struct curl_slist *headers = nullptr ;
    headers = curl_slist_append(headers, "Accept: FRS-007");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_URL, s_url.c_str());

    std::string data;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeToString);

    rlog << "score http get request: " << ", record_sn=" << s_record_sn << ", score=" << s_player_score << ", url=" << s_url << std::endl ;

    CURLcode ret = curl_easy_perform(curl_handle);
    if (ret != CURLE_OK) {
        {
            char msg[1000] = {0x00};
            sprintf(msg,"curl_easy_perform() failed: %s", curl_easy_strerror(ret) );
            rlog << msg << std::endl ;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_handle);
        return false ;
    }

    if ( data != "{\"status\":1,\"msg\":\"success\"}" ) {
        rlog << "score http get error: "<< data << ", record_sn=" << s_record_sn << ", score=" << s_player_score << ", url=" << s_url << std::endl ;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_handle);
        return false ;
    }

//    rlog << "score http get success: "<< data << ", record_sn=" << s_record_sn << ", score=" << s_player_score << ", url=" << s_url << std::endl ;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);
    return true;
}
