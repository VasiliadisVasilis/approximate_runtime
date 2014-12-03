#ifndef __CONFIG__
#define __CONFIG__

#ifdef ENABLE_CONTEXT
#error Do not set this manually
#endif

#ifdef ENABLE_SIGNALS
#error Do not set this manually
#endif

#ifdef DUAL_TASKS
#error Do not set this manually
#endif

#ifdef RAZOR
#error Do not set this manually
#endif

#ifndef NOPROTECT
  #define NOPROTECT 0
#else
  #warning NOPROTECT --- Enabled
  #undef NOPROTECT
  #define NOPROTECT 1
#endif

#ifndef SOFTWARE
  #define SOFTWARE 0
#else
  #warning SOFTWARE --- Enabled
  #undef SOFTWARE
  #define SOFTWARE 1
#endif

#ifndef ALLFEATURES
  #define ALLFEATURES 0
#else
  #warning ALLFEATURES --- Enabled
  #undef ALLFEATURES
  #define ALLFEATURES 1
#endif

#if (SOFTWARE + NOPROTECT + ALLFEATURES) != 1
#error Set ONLY one of the following NOPROTECT, SOFTWARE, ALLFEATURES
#endif

#if SOFTWARE == 1 || ALLFEATURES == 1
  #define ENABLE_SIGNALS 1
  #define ENABLE_CONTEXT 1
  #define DUAL_TASKS
#endif

#if ALLFEATURES == 1
  #define RAZOR
#endif

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
