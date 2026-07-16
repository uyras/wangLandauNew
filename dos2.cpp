#include "dos2.h"

template <typename T_x, typename T_val>
void Dos2<T_x,T_val>::save(string file)
{
    ofstream f(file);
    if (data.size()){
        for (size_t i=0; i<data.size()-1; i++){
                f<< val(i)<<"\t";
                f<< val(i+1)<<"\t";
                f<< data[i] <<endl;
        }

        f<< val(data.size()-1)<<"\t";
        f<< max <<"\t";
        f<< data[data.size()-1] <<endl;
    }

    f.close();
}

template <typename T_x, typename T_val>
void Dos2<T_x,T_val>::load(string file)
{
    ifstream f(file);
    data.clear();
    double a=0, b=0, c=0;
    f>>a; f>>b; f>>c;
    data.push_back(c);
    this->min = a;
    while (f >> a >> b >> c){
        data.push_back(c);
    }
    this->max = b;
    this->intervals = data.size();
    f.close();
}

template <typename T_x, typename T_val>
string Dos2<T_x,T_val>::toString()
{
    stringstream f;
    for (size_t i=0; i<data.size()-1; i++){
            f<< val(i)<<"\t";
            f<< val(i+1)<<"\t";
            f<< data[i] <<endl;
    }


    f<< val(data.size()-1)<<"\t";
    f<< max <<"\t";
    f<< data[data.size()-1] <<endl;
    return f.str();
}


template class Dos2<int,unsigned>;
template class Dos2<int,double>;
template class Dos2<int,char>;
template class Dos2<double,double>;
template class Dos2<double,unsigned>;
template class Dos2<double,char>;
