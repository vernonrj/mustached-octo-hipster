#ifndef _HASH_TABLE_
#define _HASH_TABLE_

#include<fstream>
#include<algorithm>
#include<tr1/functional>
#include<string>
#include<map>

/* This class uses a hash function to index into an internal table 
The maximum size of the table depends on the image of the hash function.
This could also be called a closed domain hash table.*/
template<typename T, typename R>
class HashTable
{
public:
    HashTable(T (*hashfn) (T), std::string logname="")
    {
        m_hashfn = hashfn;
        if (logname != "")
            m_filestream.open(logname.c_str(), std::fstream::out);
    }

    virtual ~HashTable()
    {
        using namespace std::tr1;
        using namespace std::tr1::placeholders;

        // save data
        if (m_filestream.is_open())
        {
            for_each(m_data.begin(), m_data.end(), bind(&HashTable::save_pair, this, _1));
            m_filestream.close();
        }
        m_hashfn = 0;
    }

    void setHashFn(T (*hashfn) (T))
    {
        m_hashfn = hashfn;
    }

    R& operator[](const T index)
    {
        return m_data[m_hashfn(index)];
    }

    void save_pair(std::pair<const T,R> element)
    {
        m_filestream << element.first << "," << element.second << "\n";
    }

private:
    T (*m_hashfn) (T);
    std::map<T, R> m_data;
    std::fstream m_filestream;

};

#endif

