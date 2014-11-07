#ifndef __CONFIG__
#define __CONFIG__


#ifdef ENABLE_CONTEXT
#warning Context_set/get mechanism enabled.
#endif

#ifdef ENABLE_SIGNALS
#warning Signal handler mechanism enabled.
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
