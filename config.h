#ifndef __CONFIG__
#define __CONFIG__

#if defined(ENABLE_SIGNALS) && defined(ENABLE_CONTEXT)==0
  #error Signals enabled without Context
#endif

#ifdef DOUBLE_QUEUES
      #define MAY_FAIL (whoami->reliable == NON_RELIABLE)
#else
      #define MAY_FAIL (exec_task->significance == NON_SIGNIFICANT)
#endif

#ifdef DUAL_TASKS
  #define TASK_SIGNIFICANCE exec_task->significance
#else
  #define TASK_SIGNIFICANCE NON_SIGNIFICANT
#endif

#ifdef FAKE_SETCONTEXT
  #ifdef ENABLE_CONTEXT
    #define GET_CONTEXT_LABEL get_context_label:
    #define setcontext(X) goto get_context_label
  #else
    #define GET_CONTEXT_LABEL  void
    #define setcontext(X) void
  #endif
  #else
    #define GET_CONTEXT_LABEL
#endif

#ifdef GEMFI 						
#include "m5op.h"
#ifdef DUAL_TASKS 
#define ENABLE_FI(val)					\
	if ( val -> significance == NON_SIGNIFICANT )		\
		fi_activate_inst(val->task_id, START); 	\

#define DISABLE_FI(val) 				\
	if( val -> significance == NON_SIGNIFICANT )		\
		fi_activate_inst(val->task_id, PAUSE); 	\

#else

#define ENABLE_FI(val) 	fi_activate_inst(val->task_id, START);
#define DISABLE_FI(val) fi_activate_inst(val->task_id, PAUSE); 

#endif

#define INIT_FI(a,b) fi_read_init_all((unsigned long)&a, (unsigned long)&b);	
#define STOP_FI() fi_activate_inst(0,DUMP);

#else

#define ENABLE_FI(val) ;
#define DISABLE_FI(val) ;
#define STOP_FI() ;
#define INIT_FI(a,b) ; printf(" 0x%lx 0x%lx\n",(unsigned long)&a, (unsigned long) &b);

#endif

#endif
