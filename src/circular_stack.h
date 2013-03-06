#ifndef CIRCULAR_STACK_H
#define CIRCULAR_STACK_H

#include <vector>
#include <stdlib.h>

template <typename T>
class CircularStack
{
   //local definitions

public:
   virtual ~CircularStack() {}
   void resize(size_t newsize)
   {
       m_datavector.resize(newsize);
   }
private:
   std::vector<T> m_datavector;
   size_t bottom;
   size_t top;
};

#endif

