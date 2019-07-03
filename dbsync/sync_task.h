#ifndef _SYNC_TASK_H_
#define _SYNC_TASK_H_

#include <memory>
#include <string>

#include "threadpool.h"
#include "task.h"
#include "db.h"

size_t writeToString(void *ptr, size_t size, size_t count, void *stream);

class sync_task : public rui::threadtask
{
public:
    sync_task() {}

    virtual void run();

    bool get_record_sn(const int i_session_id,
                       const int i_seat_id,
                       std::string& s_record_sn) ;
};

#endif

