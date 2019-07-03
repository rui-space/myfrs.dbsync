#ifndef _SCORE_LOOP_TASK_H
#define _SCORE_LOOP_TASK_H

#include <memory>
#include <string>

#include "threadpool.h"
#include "task.h"
#include "db.h"
#include "sync_task.h"

class score_loop_task : public sync_task
{
public:
    score_loop_task() {}

    virtual void run();
};

#endif

