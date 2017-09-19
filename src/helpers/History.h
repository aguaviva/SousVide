
#include "config.h"
#define min(a,b) ((a>b)?b:a)

template<class TYPE>
class History
{
    int fin = 0;
    int len = 0;
    TYPE buff[HISTORY_SIZE];
  public:
    void Queue(TYPE t)
    {
      buff[fin] = t;

      if (len < HISTORY_SIZE)
        len++;

      fin++;
      fin %= HISTORY_SIZE;
    }

    int GetSize()
    {
      return len;
    }

    int GetIndex(int i)
    {
      int ii = fin - len + i;

      if (ii < 0)
        ii += HISTORY_SIZE;

      return ii;
    }

    TYPE *GetData(int i)
    {
      return &buff[GetIndex(i)];
    }

    int GetSize1()
    {
      return min(HISTORY_SIZE - GetIndex(0), len);
    }

    int GetSize2()
    {
      return len - GetSize1();
    }

};
