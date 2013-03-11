#ifndef _HASH_TABLE_
#define _HASH_TABLE_

#include<map>

/* This class uses a hash function to index into an internal table 
The maximum size of the table depends on the image of the hash function.
This could also be called a closed domain hash table.*/
template<typename T, typename R>
class HashTable
{
public:
    HashTable(T (*hashfn) (T))
    {
        m_hashfn = hashfn;
    }

    virtual ~HashTable()
    {
        m_hashfn = 0;
    }

    R& operator[](const T index)
    {
        return m_data[m_hashfn(index)];
    }

private:
    T (*m_hashfn) (T);
    std::map<T, R> m_data;
};

#endif

