#if 0
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
#ifdef RAZOR
  #warning [NoticeMe] Razor is ENABLED
#else
  #warning [NoticeMe] Razor is disabled
#endif
#endif
