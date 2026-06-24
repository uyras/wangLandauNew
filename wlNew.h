#ifndef WLNEW_H
#define WLNEW_H

#include <QDebug>
#include <cmath>
#include <iostream>
#include <sstream>

#include "PartArray.h"
#include "StateMachineFree.h"
#include "dos2.h"
#include "gapmanager.h"

using namespace std;

class WLNew
{
public:
    WLNew(PartArray *sys, unsigned intervals, double accuracy=0.8, double fmin=1.0001);
    virtual ~WLNew();

    void run(unsigned steps=10000);
    void runWithSave(unsigned steps=10000, unsigned saveEach=100);

    /**
     * @brief save сохранить гистограммы в файл
     * @param filename Имя файла для сохранения. По умолчанию сохраняет в формате g_<number_of_parts>_<intervals>.dat.
     */
    void saveG(const std::string filename="") const;
    void saveH(const std::string filename="") const;

    Dos2<double> g;//g - логарифм плотности состояний (энтропия), h - вспомогательная гистограмма, которая должна быть плоской
    Dos2<unsigned> h;

    bool showMessages;
    unsigned saveEach; ///каждые сколько шагов сохранять данные в файл. если 0 то не сохранять.

    GapManager gaps;
private:
    PartArray *sys;
    unsigned int intervals; //число интервалов в плотности состояний
    double accuracy; //величина погрешности для степени плоскости гистограммы
    double fMin,f;


    double average; //подсчитывает среднее число для h
    unsigned hCount; //количество ненулевых элементов h, нужно для подсчета среднего


    bool isFlat(); //критерий плоскости гистограммы
    double dispersion(unsigned gapNumber); //проверяем дисперсию на участке
    double dispersion2(unsigned gapNumber); //проверяем дисперсию на участке
    void updateGH(double E=0.0);
    void resetH();
    void normalizeG();
    inline void msg(std::string str){if (showMessages) cout<<str<<endl;}
    inline void msg(std::string str,double val){if (showMessages) cout<<str<<val<<endl;}
    inline void msg(std::string str,int val){if (showMessages) cout<<str<<val<<endl;}

};

#endif // WLNEW_H
