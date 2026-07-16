#include <iostream>
#include <ctime>
#include <random>

#include "MagneticSystem.h"
#include "wlNew.h"
#include "misc.h"
#include "debug_msg.h"

using namespace std;

#define MEASURE(x,y,n) { \
    unsigned start_time =  clock(); \
    for (unsigned i=0;i<n;i++) x; \
    std::cout<<y<<": "<<std::setprecision(10)<<(std::clock()-start_time)/CLOCKS_PER_SEC/n<<endl;\
    }


int main(int argc, char *argv[])
{
    // Примеры использования отладочных макросов
    DBG() << "Программа запущена, argc = " << argc;

    for (int i = 0; i < argc; ++i) {
        DBG() << "argv[" << i << "] = " << argv[i];
    }

    MagneticSystem sys("apamea_2_2.mfsys", 5.0, Vect{8,8,0});
    // MagneticSystem sys("apamea_4_4.mfsys", 5.0, Vect{16,16,0});
    const unsigned intervals = 10000;

    DBG() << "Система создана, intervals = " << intervals;

    WLNew wl(&sys, intervals, 42, 0.75, 1.00001);
    wl.showMessages=true;
    wl.saveEach = 1e7;
    wl.fullRecalculateEEvery = 1e7;
    // wl.gaps.setUniform(10, intervals, 0.2);
    // cout<<wl.gaps.toWolframString()<<endl;

    // DBG_F("Запуск WL с параметрами: flatness=%f", 0.75);

    MEASURE(wl.run(100000),"WL new",1)

    //wl.runWithSave(10000,10000);
    wl.saveG("g.dat");

    DBG() << "Сохранено в g.dat";

    DBG_W() << "Работа завершена, результат сохранён";

    cout<<"finish";

    return 0;
}
