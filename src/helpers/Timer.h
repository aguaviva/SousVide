class Timer
{
  public:
    enum Status
    {
      TIMER_Stopped = 0,
      TIMER_Running = 1,
      TIMER_TimedOut = 2
    };

  private:
    unsigned long m_LastTime = 0;
    unsigned long m_time = 0;
    unsigned long m_timeOut = 0;

    Status m_Status = TIMER_Stopped;

  public:
    void Start()
    {
      m_LastTime = millis();
      m_Status = TIMER_Running;
      m_time = 0;
    }

    void Stop()
    {
      m_Status = TIMER_Stopped;
    }

    int GetElapsedTime()
    {
      if ( m_Status == TIMER_Running)
      {
        unsigned long nowtime = millis();
        m_time += (nowtime - m_LastTime);
        m_LastTime = nowtime;
      }
      return m_time;
    }

    void SetTimeOut(unsigned long timeOut)
    {
      m_time = 0;
      m_timeOut = timeOut;
    }

    int GetTimeOut()
    {
      return m_timeOut;
    }



    int GetTimeLeft()
    {
      if ( m_Status == TIMER_Stopped)
        return -1;

      if ( m_Status == TIMER_TimedOut)
          return 0;

      return GetTimeOut() - GetElapsedTime();
    }

    Timer::Status GetStatus()
    {
      if ( m_Status == TIMER_Running)
      {
        if ( GetElapsedTime() >= GetTimeOut())
          m_Status = TIMER_TimedOut;
      }
      return m_Status;
    }

};
