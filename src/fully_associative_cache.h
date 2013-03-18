#ifndef _VICTIM_CACHE_
#define _VICTIM_CACHE_
#include<vector>
#include<algorithm>

// This class implements a fully associative cache.
// LRU replacement is implemented with the clock algorithm.
class FullyAssociativeCache
{
public:
    FullyAssociativeCache(uint entries)
    {
        m_counter = 0;
        m_storage.resize(entries);
    }

    ~FullyAssociativeCache()
    {

    }

    //cache.additem(0x12345679) = data;
    uint& additem(uint tag)
    {
       using namespace std;
       vector<line_t>::iterator it; 
       it = find(m_storage.begin(), m_storage.end(), tag);
       if(it != m_storage.end()) // if tag is present just return a reference
       {   it->used = true;
           return it->value;
       }
       else //find an element to evict
       {
           while(true)
           {
               //increment counter and get element reference
               m_counter = m_counter + 1 % m_storage.size();
               line_t& element = m_storage[m_counter];

               //flip used bits
               element.used = ~element.used;
               if(element.used == true) // value to be evicted
               {
                   element.tag = tag;
                   return element.value;
               }
           }
       }

    }

    //data = cache.getitem(0x12345679);
    uint getitem(uint tag)
    {
       using namespace std;
       vector<line_t>::iterator it; 
       it = find(m_storage.begin(), m_storage.end(), tag);

       return (it != m_storage.end())? it->value : 0;
    }

private:
struct line_t
    {
        uint tag;
        uint value;
        bool used;

        line_t() //line size = sizeof(value) + sizeof(tag) + 1
        {
            tag   = 0;
            value = 0;
            used  = false; //size 1
        }

        bool operator==(const uint x)
        {
           return (x == this->tag);
        }
    };

    std::vector<line_t> m_storage;
    uint m_counter; //size log ways
};

#endif
