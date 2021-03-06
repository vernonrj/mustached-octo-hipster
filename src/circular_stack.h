#ifndef CIRCULAR_STACK_H
#define CIRCULAR_STACK_H

#include <vector>
#include <stdlib.h>

// this is a dirty circular stack - it will return an old value or overwrite
// an old value if it runs out of bounds

template <typename T>
class CircularStack
{
   //local definitions

public:
   CircularStack()
   :m_top(0)
   {}

   virtual ~CircularStack() {}
   void resize(size_t newsize)
   {
       m_datavector.resize(newsize);
       m_top = (m_top < newsize) ? m_top: m_top % newsize;
   }

   /**
    * Push a new element onto the stack
    */
   void push(T newvalue)
   {
      size_t stacksize = m_datavector.size();
      m_top = (m_top >= stacksize) ? 0 : m_top + 1;
      m_datavector[m_top] = newvalue;
   }

   /**
    * Returns the element currently on the top of the stack.
    */
   T pop()
   {
      T topelement = m_datavector[m_top];
      m_top = (m_top == 0) ? m_datavector.size() : m_top - 1; 
      return topelement;
   }

   /** returns the memory budget of the stack in bits 
   *   (might want to refactor to allow for arbitrary bit sizes)
   */
   size_t memoryusage()
   {
      return sizeof(T)*8*m_datavector.size();
   }

private:
   std::vector<T> m_datavector;
   size_t m_top;
};

#endif

