#ifndef PREDICTOR_STATISTICS_H
#define PREDICTOR_STATISTICS_H

#include<fstream>
#include<sstream>

template<typename T>
class TraceVar
{
public:
    TraceVar(T initial_value = 0):
        m_data(initial_value),
        m_accesses(0),
        m_matches(0)
    {
    }

    virtual ~TraceVar()
    {

    }

    void showStats(std::string& str)
    {
         std::stringstream ss;
         ss << "Last_Value: " << m_data << '\n';
         ss << "Matches   : " << m_matches << '\n';
         ss << "Accesses  : " << m_accesses << '\n';
         ss << "Max       : " << m_max << '\n';
         ss << "Min       : " << m_min << '\n';
         str = ss.str();
    }

    bool operator<(const TraceVar<T> right)
    {
        return this.m_data < right.m_data;
    }

    bool operator<(const T right)
    {
        return this.m_data < right;
    }

    bool operator>(const T right)
    {
        return this.m_data > right;
    }

    bool operator>(const TraceVar<T> right)
    {
        return this.m_data > right.m_data;
    }

    bool operator== (const T right)
    {
        return this.m_data == right;
    }

    bool operator== (const TraceVar<T> right)
    {
        return right.m_data == this.m_data;
    }

    T& operator= (const T right)
    {
        if(m_min > right)
            m_min = right;
        if(m_max < right)
            m_max = right;
        if(right == this.m_data)
            ++m_matches;
        ++m_accesses;
        this->m_data = right;
        return *this;
    }

    T& operator= (const TraceVar<T> right)
    {
        return *this = right.m_data;
    }

private:
    T m_data; 
    size_t m_accesses;
    size_t m_matches;
    T m_max;
    T m_min;
};


class PredictorStatistics
{
public:
    PredictorStatistics()
    {
        //open csv file
    }

    ~PredictorStatistics()
    {
    }

    void branch_trace(
         const branch_record_c* br, 
         bool taken, 
         uint actual_target_addrss)
    {
    }

private:


};


#endif //PREDICTOR_STATISTICS_H 
