#ifdef ENABLE_SIGNALS
  #warning [NoticeMe] Signal handler mechanism ENABLED
#else
  #warning [NoticeMe] Signal handler mechanism disabled
#endif
#ifdef ENABLE_CONTEXT
  #warning [NoticeMe] Context_set/get mechanism ENABLED
#else
  #warning [NoticeMe] Context_set/get mechanism disabled
#endif
#ifdef FAKE_SETCONTEXT
  #warning [NoticeMe] setcontext will be replaced by goto and disabled in coordinator.c
#endif
#ifdef DUAL_TASKS
  #warning [NoticeMe] Only non-significant tasks are injected faults ENABLED
#else
  #warning [NoticeMe] Only non-significant tasks are injected faults disabled
#endif
#ifdef DOUBLE_QUEUES
  #warning [NoticeMe] Significant/NonSignificant tasks use different queues ENABLED
#else
  #warning [NoticeMe] Significant/NonSignificant tasks use different queues disabled
#endif

