#ifndef _AVATAR_TASK_H
#define _AVATAR_TASK_H

#include <memory>
#include <string>

#include "threadpool.h"
#include "task.h"
#include "db.h"
#include "sync_task.h"

#define AVATAR_UPLOAD_URL "https://frs.chinahost.net.cn/api.php/Public/gameRecordUploadPic"

#define AVATAR_PATH "/usr/local/webcenter/public"

class avatar_task : public sync_task
{
public:
    avatar_task(const int i_session_id,
                const int i_seat_id,
                const std::string& s_player_avatar) :
        mi_session_id(i_session_id),
        mi_seat_id(i_seat_id),
        ms_player_avatar(s_player_avatar) {}

    virtual void run();

    bool upload(const std::string& s_record_sn,
                      const std::string& s_player_avatar_path,
                      const std::string& s_player_avatar_name);

private:
    int mi_session_id;
    int mi_seat_id;
    std::string ms_player_avatar;
};

#endif

