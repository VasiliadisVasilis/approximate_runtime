#ifndef __CONFIG__
#define __CONFIG__


#ifdef ENABLE_CONTEXT
  #warning [NoticeMe] Context_set/get mechanism ENABLED
#else
  #warning [NoticeMe] Context_set/get mechanism disabled
#endif
#ifdef FAKE_SETCONTEXT
#warning [NoticeMe] setcontext will be replaced by goto and disabled in coordinator.c
#ifdef ENABLE_CONTEXT
	#define setcontext(X) goto GET_CONTEXT
#else
	#define setcontex(X) void
#endif
#endif

#ifdef ENABLE_SIGNALS
  #warning [NoticeMe] Signal handler mechanism ENABLED
#else
  #warning [NoticeMe] Signal handler mechanism disabled
#endif

#ifdef DUAL_TASKS
  #warning [NoticeMe] Only non-significant tasks are injected faults ENABLED
#else
  #warning [NoticeMe] Only non-significant tasks are injected faults disabled
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
