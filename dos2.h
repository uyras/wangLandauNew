#ifndef DOS2_H
#define DOS2_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <unordered_map>

using namespace std;


template <typename T_x, typename T_val>
class Dos2
{
public:
    Dos2():min(0),max(0),intervals(0){}
    virtual ~Dos2(){}

    Dos2(T_x min, T_x max, size_t intervals)
    {
        this->resize(min,max,intervals);
    }

    inline void resize(T_x min, T_x max, size_t intervals){
        this->min = min;
        this->max= max;
        this->intervals = intervals;
        data.resize(intervals);
    }

    virtual void save(string file);
    virtual void load(string file);
    string toString();

    inline T_val& at(size_t i){return data[i];}
    inline const T_val& at(size_t i) const {return data[i];}
    inline T_val& operator[](const T_x& a){return data[num(a)];}
    inline const T_val& operator[](T_x a) const {return data[num(a)];}

    inline bool const operator==(const Dos2<T_x,T_val>& rhs) const {
        if (this->min != rhs.min) return false;
        if (this->max != rhs.max) return false;
        if (this->intervals != rhs.intervals) return false;
        if (this->data != rhs.data) return false;

        return true;
    }
    inline bool const operator!=(const Dos2<T_x,T_val>& rhs) const { return !(this->operator==(rhs)); }

    inline size_t Intervals() const {return intervals;}
    inline T_x Min() const {return this->min;}
    inline T_x Max() const {return this->max;}


    /**
     * @brief val Возращает значение, с которого начинается i интервал
     * @param i Интервал, для которого надо получить соответствующее значение типа T_x
     * @return
     */
    inline T_x val(size_t i) const{ return min + (max-min)*(i/(double)(intervals)); }

    /**
     * @brief num Возращает номер интервала для соответствующего значения
     * @param a
     */
    inline size_t num(const T_x& a) const{
        if (a<min)
            return 0;
        if(a>=max)
            return intervals-1;
        size_t r= (size_t)floor(double(a-min)/(max-min)*(intervals));
        if (r>intervals-1)
            return size_t(intervals-1);
        else
            return r;
    }

    /**
     * @brief clear Reset all values of DOS to 0
     */
    inline void clear(){
        for (auto& i: data){
            i=0.;
        }
    }

protected:
    T_x min, max;
    size_t intervals;
    vector< T_val > data;
};

#endif // DOS2_H
