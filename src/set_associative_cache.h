#ifndef _SET_CACHE_
#define _SET_CACHE_
#include<vector>
#include<tr1/functional>
#include<algorithm>

// This class implements a fully associative cache.
// LRU replacement is implemented with the clock algorithm.
class SetAssociativeCache
{
    //private forward declarations
    struct line_t;
    struct set_t;
    static void null_evict(uint,uint){}
public:
    SetAssociativeCache()
    :m_evictfn(null_evict)
    {}

    ~SetAssociativeCache()
    {
    }

    void setDimentions(uint sets, uint associativity=1)
    {
        using namespace std;
        m_storage.clear(); //clear out cache before resize
        m_storage.resize(sets);
        for(std::vector<set_t>::iterator i = m_storage.begin(); 
            i != m_storage.end(); ++i)
        {
            i->m_counter = 0;
            i->m_storage.resize(associativity);
        }
    }

    // A good use for this is to use bind to have the evict call into
    // a second level caches add. e.g.
    // cacheobject1.setEvictCallBack(
    //     std::tr1::bind(&Class::memfun, 
    //     &cacheobject2, _1, _2));
    // where memfun is a function that takes (uint, uint)
    void setEvictCallBack(std::tr1::function<void (uint, uint)> fn)
    {
        m_evictfn = fn;
    }

    // e.g. on use
    // cache.additem(0x123456, data, evicted_addr, evicted_data);
    void additem(uint addr, uint data)
    {
       using namespace std;
       uint tag = addrtotag(addr);

       vector<line_t>& set = m_storage[addrtoindex(addr)].m_storage; 
       uint& counter = m_storage[addrtoindex(addr)].m_counter;

       uint evaddress = 0;
       uint evdata = 0;

       vector<line_t>::iterator it; 
       it = find(set.begin(), set.end(), tag);
       if(it != set.end())
       {
           it->used = true;
           it->value = data;
           return;
       }
       else //find an element to evict
       {
           while(true)
           {
               //increment counter and get element reference
               counter = (counter + 1) % set.size();
               line_t& element = set[counter];

               //flip used bits
               element.used = ~element.used;
               if(element.used == true) // value to be evicted
               {
                   element.tag = tag;
                   evaddress = (addr & (tag << log2(m_storage.size())));
                   evdata = element.value;
                   if(evaddress != 0)
                       m_evictfn(evaddress, evdata);

                   element.value = data;
                   return;
               }
           }
       }
    }

    //data = cache.getitem(0x12345679);
    uint getitem(uint addr)
    {
       using namespace std;
       uint tag = addrtotag(addr);

       vector<line_t>& set = m_storage[addrtoindex(addr)].m_storage; 

       vector<line_t>::iterator it; 
       it = find(set.begin(), set.end(), tag);
       if(it != set.end()) // if tag is present just return a reference
       {   
           it->used = true;
           return it->value;
       }
        return 0;
    }
private: //helper functions
    //return the Ceil(log2(x))
    static uint log2(uint x)
    {
        uint i;
        for(i = 0; x >= ((unsigned)1<<i); ++i){}
        return i;
    }

    virtual uint addrtotag(uint addr)
    {
       //calc index bit size
        uint indexbits = log2(m_storage.size());
       return addr >> indexbits;
    }

    virtual uint addrtoindex(uint addr)
    {
        //calc index bit size and make mask
        uint indexbits = log2(m_storage.size());
        uint mask = ~0 << indexbits;
        mask = ~mask;
        //mask out all other bits
        return addr & mask;
    }

private:
    struct set_t //set size = sizeof(line_t) + log2(ways)
    {
        std::vector<line_t> m_storage;
        uint m_counter;
    };

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

    std::vector<set_t> m_storage;
    std::tr1::function<void (uint,uint)> m_evictfn;
};

class PcRelCache: public SetAssociativeCache 
{
    virtual uint addrtotag(uint addr)
    {
        return addr;
    }

};

#endif
