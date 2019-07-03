#ifndef _PARSE_TASK_H_
#define _PARSE_TASK_H_

#include <memory>
#include <string>

#include "threadpool.h"
#include "task.h"

class parse_task : public rui::threadtask
{
	public:
        parse_task( std::shared_ptr<rui::task> p_task) :
            mp_task(p_task) {}

		virtual void run( );
		
		std::shared_ptr<rui::task> mp_task ;
};

#endif

