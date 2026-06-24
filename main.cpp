#include <QtDebug>
#include <iostream>
#include <ctime>

#include "random.h"
#include "squareisinglattice.h"

#include "wlNew.h"
#include "wanglandau.h"

using namespace std;


#define MEASURE(x,y,n) { \
    unsigned start_time =  clock(); \
    for (unsigned i=0;i<n;i++) x; \
    std::cout<<y<<": "<<std::setprecision(10)<<(std::clock()-start_time)/CLOCKS_PER_SEC/n<<endl;\
    }


int main(int argc, char *argv[])
{

    SquareIsingLattice sys;
    const unsigned n=20;
    sys.dropSquareLattice(n,n);
    sys.setMinstate(sys.groundState());
    sys.setMaxstate(sys.maximalState());


    WangLandau wl2(&sys, n*n*4, 0.8, 1.01);
    wl2.showMessages=true;
    wl2.saveEach = 100000;
    MEASURE(wl2.run(100000),"WL old",1);

    WLNew wl(&sys, n*n*4, 0.8, 1.01);
    wl.showMessages=true;
    wl.saveEach = 100000;
    wl.gaps.setUniform(10, n*n*4, 0.2);
    cout<<wl.gaps.toWolframString()<<endl;
    MEASURE(wl.run(100000),"WL new",1)

    //wl.runWithSave(10000,10000);
    wl.saveG("g.dat");


    qInfo()<<"finish";

    return 0;
}
