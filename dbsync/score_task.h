#ifndef _SCORE_TASK_H
#define _SCORE_TASK_H

#include <memory>
#include <string>

#include "threadpool.h"
#include "task.h"
#include "db.h"
#include "sync_task.h"

#define SCORE_UPLOAD_URL "https://frs.chinahost.net.cn/api.php/Public/gameRecordAdd"

class score_task : public sync_task
{
public:
    score_task(const int i_session_id,
                const int i_seat_id,
                const std::string& s_player_score) :
        mi_session_id(i_session_id),
        mi_seat_id(i_seat_id),
        ms_player_score(s_player_score) {}

    virtual void run();

    bool upload(const std::string& s_record_sn,
                     const std::string& s_player_score);

private:
    int mi_session_id;
    int mi_seat_id;
    std::string ms_player_score;
};

#endif

