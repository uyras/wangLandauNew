#include "wlNew.h"

WLNew::WLNew(MagneticSystem *sys, unsigned intervals, int seed, double accuracy, double fmin):
    showMessages(false),
    saveEach(0),
    sys(sys),
    intervals(intervals),
    accuracy(accuracy),
    fMin(log(fmin)),
    f(0)
{
    generator.seed(seed);
    state.clear();
    state.resize(sys->N(),1);

    //инициируем DOS
    double EAbs = sys->EAbs();
    h.resize(-EAbs, EAbs, intervals);
    g.resize(-EAbs, EAbs, intervals);
}

WLNew::~WLNew()
{

}

void WLNew::run(unsigned steps)
{
    uniform_int_distribution<int> intDistr(0, sys->N() - 1); // including right edge
    uniform_real_distribution<double> doubleDistr(0, 1);	   // right edge is not included

    //ищем состояния, входящие в гапы
    vector<state_t> states;
    cout<<gaps.Gaps()<<endl;
    states.resize(gaps.Gaps());
    double eTemp,dE; 
    eTemp = this->fullRefreshEnergy();
    bool revert=false;
    // for (unsigned gapNumber=0; gapNumber<gaps.Gaps(); gapNumber++){
    //     unsigned long int i=0;
    //     while (!gaps.inRange(g.num(eTemp),gapNumber)){
    //             int partNum = intDistr(generator);
    //             dE = this->getdE(partNum);
    //             //идем сверху вниз по энергии: если новое состояние ниже, принимаем переворот
    //             //идем снизу вверх по энергии: если новое состояние выше, принимаем переворот
    //             if ((revert && dE<0) || ((!revert) && dE>0)){
    //                 state[partNum] *= -1;
    //                 eTemp += dE;
    //             }
    //             //если случайно проскочили энергию, меняем направление работы
    //             if (
    //                 (revert && (g.num(eTemp) < gaps.from(gapNumber))) ||
    //                 ((!revert) && (g.num(eTemp) > gaps.to(gapNumber)))
    //             ) {
    //                 revert=!revert;
    //             }

    //             i++;
    //             if (i==100000){
    //                 cout<<"init state makes too long on gap "<<gapNumber<<", E="<<eTemp<<
    //                          " ("<<g.val(gaps.from(gapNumber))<<";"<<g.val(gaps.to(gapNumber)+1)<<")"<<endl;
    //                 break;
    //             }
    //     }
    //     states[gapNumber] = this->state;
    // }

    const state_t initState = this->state;
    this->f = 1;
    long unsigned accepted=0, rejected=0, totalSteps=0;
    this->resetH();

    msg("steps=",steps);

    double eOld = this->fullRefreshEnergy();
    updateGH(eOld);

    while (f>fMin){
        //повторяем алгоритм сколько-то шагов
        for (unsigned i=0;i<steps;i++){
            int partNum = intDistr(generator);

            dE = this->getdE(partNum);
            if (doubleDistr(generator) <= exp(g[eOld]-g[eOld+dE])) {
                eOld += dE;
                state[partNum] *= -1;
                ++accepted;
            } else {
                ++rejected;
            }

            updateGH(eOld);
            ++totalSteps;

            if (saveEach && totalSteps%saveEach==0){
                ostringstream fn;
                fn<<"g_"<<totalSteps<<".dat";
                saveG(fn.str());

                fn.str("");
                fn.clear();
                fn<<"h_"<<totalSteps<<".dat";
                saveH(fn.str());
            }
        }

        //проверяем ровность диаграммы
        if (this->isFlat()){
            f /= 2.;
            this->resetH();
            msg("accepted ",(int)accepted);
            msg("rejected ",(int)rejected);
            msg("h is flat, new f is ",exp(f));
            accepted=0; rejected=0;

        // } else { //если гистограмма не плоская, бросаем алгоритм на интервал с наибольшей дисперсией
        //     double maxDisp=0, temp=0;
        //     unsigned lastJumped=0;
        //     for (unsigned j=0; j<gaps.Gaps(); j++){
        //         temp = dispersion2(j);
        //         if (temp>maxDisp){
        //             maxDisp=temp;
        //             lastJumped=j;
        //         }
        //     }
        //     if (!gaps.inRange(g.num(eOld), lastJumped) && states[lastJumped].size()>0){
        //         this->state = states[lastJumped];
        //         eOld = this->fullRefreshEnergy();
        //         msg("jump to",lastJumped);
        //         updateGH(eOld);
        //     }
        }
    }


    //this->state = initState;
    return;
}

void WLNew::saveH(const string filename) const
{
    ofstream f(filename);
    for (size_t i=0;i<h.Intervals();i++){
        if (h.at(i)!=0.)
            f<<i<<"\t"<<h.at(i)<<"\t"<<h.val(i)<<"\t"<<h.val(i+1)<<endl;
    }

    f.close();
}

void WLNew::saveG(const string filename) const
{
    ofstream f(filename);
    f<<"# No\tln(g)\teFrom\teTo"<<endl;
    for (size_t i=0;i<g.Intervals();i++){
        if (g.at(i)!=0.)
            f<<i<<"\t"<<g.at(i)<<"\t"<<g.val(i)<<"\t"<<g.val(i+1)<<endl;
    }

    f.close();
}

double WLNew::getdE(size_t rotated)
{
    double dE = 0;
    size_t neighbours_from = sys->neighbours_from[rotated];
    size_t neighbours_to = sys->neighbours_from[rotated] + sys->neighbours_count[rotated];
    for (size_t neigh = neighbours_from; neigh < neighbours_to; neigh++)
    {
        if (state[sys->neighbourNums[neigh]] == state[rotated]) // assume it is rotated, inverse state in mind
            dE -= 2. * sys->eMatrix[neigh];
        else
            dE += 2. * sys->eMatrix[neigh];
    }

    dE += 2 * scalar(sys->parts[rotated].m, field) * state[rotated];
    return dE;
}

double WLNew::fullRefreshEnergy()
{   
    double eOld = 0;
    for (size_t i=0; i<sys->N(); i++){
        size_t nf = sys->neighbours_from[i];
        size_t nc = sys->neighbours_count[i];
        for (size_t k = nf; k < nf + nc; k++){
            eOld +=  sys->eMatrix[k] * state[i] * state[sys->neighbourNums[k]] / 2;
        }

        eOld -= scalar(sys->parts[i].m, field) * state[i];
    }

    return eOld;
}

// критерий плоскости гистограммы
bool WLNew::isFlat()
{
    if (average == 0.0) return false;
    for (size_t i=0; i<h.Intervals(); i++){//плоскость гистограммы только в своем интервале
        if (h.at(i)!=0 && h.at(i) < average * accuracy) {          // односторонний критерий
            DBG_RL() << "not flat at bin " << i << " (" << h.at(i)
                     << "), average=" << average
                     << ", accuracy*average=" << (accuracy * average);
            return false;
        }
    }
    return true;
}

double WLNew::dispersion(unsigned gapNumber)
{
    double disp=0;
    unsigned iCount=0;
    for (size_t i=gaps.from(gapNumber); i<=gaps.to(gapNumber); i++){//считаем матожидание
        if (h.at(i)!=0.){
            disp=((double)disp*(double)iCount+pow((double)h.at(i)-average,2.))/(double)(iCount+1);
            iCount++;
        }
    }
    return sqrt(disp);
}

double WLNew::dispersion2(unsigned gapNumber)
{
    double disp=0;
    for (size_t i=gaps.from(gapNumber); i<=gaps.to(gapNumber); i++){//считаем матожидание
        if (h.at(i)!=0.){
            if (fabs(h.at(i)-average)>disp){
                disp=fabs(h.at(i)-average);
            }
        }
    }
    return disp;
}

void WLNew::updateGH(double E)
{
    if (h[E]==0){ 
        //случай если изменилось число ненулевых элементов
        hCount++;
        average = (average * (hCount-1)) / hCount;
    }

    g[E]+=f;
    //прибавляем h и одновременно считаем среднее значение
    h[E]+=1; 
    average += (1./(double)hCount);
}

void WLNew::resetH()
{
    for (size_t i=0; i<h.Intervals(); i++){
        h.at(i)=0;
    }
    average=0.0;
    hCount=0;
}

void WLNew::normalizeG()
{
    const double gFirst=g.at(0);
    for (size_t i=0; i<g.Intervals(); i++){
        g.at(i)-=gFirst;
    }
}
