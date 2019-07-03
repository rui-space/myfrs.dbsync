#include "task.h"
#include "net.h"
#include "avatar_task.h"
#include "db_frsit.h"
#include "record.h"
#include "main.h"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <curl/curl.h>

void avatar_task::run() {
    rui::mutex lock( m_avatar_lock );

    //rlog << "avatar_task : mi_session_id=" << mi_session_id << ", mi_seat_id=" << mi_seat_id << ", ms_player_avatar=" << ms_player_avatar << std::endl ;

    std::string s_record_sn;

    if ( !get_record_sn(mi_session_id, mi_seat_id, s_record_sn) ) {
        rlog << "get_record_sn error" << std::endl ;
        return ;
    }

    std::string s_player_avatar_path = AVATAR_PATH;
    s_player_avatar_path += ms_player_avatar;
    if(access(s_player_avatar_path.c_str(),F_OK) < 0) {
        rlog << "image file " << s_player_avatar_path << " is not exist ." << std::endl ;
        return ;
    }

    std::string s_player_avatar_name = ms_player_avatar.substr(ms_player_avatar.rfind('/')+1, ms_player_avatar.length());
    if ( !upload(s_record_sn, s_player_avatar_path, s_player_avatar_name) ) {
        rlog << "avatar upload error: session_id=" << mi_session_id << ", seat_id=" << mi_seat_id << ", avatar=" << ms_player_avatar << std::endl ;
        return ;
    }

    if ( !db_frsit::_instance().set_avatar_uploaded(mi_session_id, mi_seat_id) ) {
        rlog << "db_frsit::set_avatar_uploaded() error" << std::endl ;
        return ;
    }

    rlog << "avatar upload success : session_id=" << mi_session_id << ", seat_id=" << mi_seat_id << ", avatar=" << ms_player_avatar << std::endl ;
}

//    POST	https://frs.chinahost.net.cn/api.php/Public/gameRecordUploadPic
//    请求参数（POST传参）
//    名称	类型	说明
//    record_sn	varchar(30)	游戏记录唯一ID，也就是贵公司之前提供的二维码说明文档里生成的唯一ID,例如（0001010010331904151243）
//    file	data	图片数据
//    返回值（JSON）
//    status	tinyint(1)	1上传成功 0上传失败
//    msg	varchar(50)	提示信息

bool avatar_task::upload(const std::string& s_record_sn,
                         const std::string& s_player_avatar,
                         const std::string& s_player_avatar_name) {
    CURL* curl_handle = curl_easy_init();
    if ( curl_handle == nullptr ) {
        rlog << "curl_easy_init() error" << std::endl ;
        return false ;
    }

    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "record_sn",
                 CURLFORM_COPYCONTENTS, s_record_sn.c_str(),
                 CURLFORM_END) ;

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, s_player_avatar.c_str(),
                 CURLFORM_FILENAME, s_player_avatar_name.c_str(),
                 CURLFORM_END);

    struct curl_slist * headers = 0;
    headers = curl_slist_append(headers, "Expect:");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl_handle, CURLOPT_URL, AVATAR_UPLOAD_URL);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, post) ;

    std::string data;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeToString);

    rlog << "avatar http post request: " << ", record_sn=" << s_record_sn << ", file=" << s_player_avatar << std::endl ;

    CURLcode ret = curl_easy_perform(curl_handle);
    if (ret != CURLE_OK) {
        rlog << "curl_easy_perform() failed: " << curl_easy_strerror(ret) << ", path=" << s_player_avatar << std::endl ;
        curl_easy_cleanup(curl_handle);
        return false ;
    }

    if ( data != "{\"status\":1,\"msg\":\"success\"}" ) {
        rlog << "avatar http post error: "<< data << ", record_sn=" << s_record_sn << ", file=" << s_player_avatar << std::endl ;
        curl_easy_cleanup(curl_handle);
        return false ;
    }

//    rlog << "avatar http post success: "<< data << ", record_sn=" << s_record_sn << ", file=" << s_player_avatar << std::endl ;

    curl_easy_cleanup(curl_handle);
    return true ;
}


