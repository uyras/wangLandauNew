#ifndef WLNEW_H
#define WLNEW_H

#include <cmath>
#include <iostream>
#include <sstream>
#include <random>

#include "dos2.h"
#include "gapmanager.h"
#include "MagneticSystem.h"
#include "debug_msg.h"

using namespace std;

class WLNew
{
public:

    /**
     * @brief Construct a new WLNew object
     * 
     * @param sys 
     * @param intervals 
     * @param accuracy 
     * @param fmin критерий остановки. Минимальное значение, которого должен достигнуть f
     */
    WLNew(
        MagneticSystem *sys, 
        unsigned intervals,
        int seed = 42, 
        double accuracy=0.8, 
        double fmin=1.0001);
    virtual ~WLNew();

    void run(unsigned steps=10000);
    void runWithSave(unsigned steps=10000, unsigned saveEach=100);

    /**
     * @brief save сохранить гистограммы в файл
     * @param filename Имя файла для сохранения. По умолчанию сохраняет в формате g_<number_of_parts>_<intervals>.dat.
     */
    void saveG(const std::string filename="") const;
    void saveH(const std::string filename="") const;

    Dos2<double,double> g;//g - логарифм плотности состояний (энтропия), h - вспомогательная гистограмма, которая должна быть плоской
    Dos2<double,unsigned> h;
    Dos2<double,char> visited; //список столбцов, которые посетили хотя бы один раз

    bool showMessages;
    unsigned saveEach; ///каждые сколько шагов сохранять данные в файл. если 0 то не сохранять.
    unsigned fullRecalculateEEvery; // каждые сколько шагов делать полный пересчет энергии

    GapManager gaps;
    Vect field = {0,0,0};

    unsigned long accepted;
    unsigned long rejected;
    unsigned long totalSteps;
private:
    MagneticSystem *sys;
    unsigned int intervals; //число интервалов в плотности состояний
    double accuracy; //величина погрешности для степени плоскости гистограммы
    double fMin,f;


    double average; //подсчитывает среднее число для h
    unsigned hCount; //количество ненулевых элементов h, нужно для подсчета среднего

    double getdE(size_t rotated);
    double fullRefreshEnergy();

    bool isFlat(); //критерий плоскости гистограммы
    double dispersion(unsigned gapNumber); //проверяем дисперсию на участке
    double dispersion2(unsigned gapNumber); //проверяем дисперсию на участке
    void updateGH(double E);
    void resetH();
    void normalizeG();
    void resetState();

    template<typename... Args>
    inline void msg(const Args&... args) {
        if (showMessages) {
            (std::cout << ... << args) << std::endl;
        }
    }

    default_random_engine generator;
    state_t state;
    state_t minState;
    double minE;
    double currentE;
    unsigned long lastReset;
};

#endif // WLNEW_H
