#include "wlNew.h"

WLNew::WLNew(PartArray *sys, unsigned intervals, double accuracy, double fmin):
    showMessages(false),
    saveEach(0),
    sys(sys),
    intervals(intervals),
    accuracy(accuracy),
    fMin(fmin),
    f(0)
{

    //считаем минимум и максимум системы
    if (sys->EMin()==0 || sys->EMax()==0)
        qFatal("Min or max state is unknown. DOS calculation is impossible.");

    //инициируем DOS
    h.resize(sys->EMin(), sys->EMax(), intervals);
    g.resize(sys->EMin(), sys->EMax(), intervals);
}

WLNew::~WLNew()
{

}

void WLNew::run(unsigned steps)
{
    //ищем состояния, входящие в гапы
    sys->state = sys->Minstate();
    vector<StateMachineFree> states;
    cout<<gaps.Gaps()<<endl;
    states.resize(gaps.Gaps());
    double eTempPrev,eTemp; eTemp = eTempPrev = sys->E();
    unsigned long int i=0; int rotated = 0; bool revert=false;
    for (unsigned gapNumber=0; gapNumber<gaps.Gaps(); gapNumber++){
        while (!gaps.inRange(g.num(eTemp),gapNumber)){
                eTempPrev = eTemp;
                rotated = this->sys->state.randomize();
                eTemp=sys->E();
                if (revert){//идем сверху вниз по энергии: если новое состояние выше, отменяем переворот
                    if (eTemp>eTempPrev){
                        this->sys->parts[rotated]->rotate(true);
                        eTemp = eTempPrev;
                    }
                } else {//идем снизу вверх по энергии: если новое состояние ниже, отменяем переворот
                    if (eTemp<eTempPrev){
                        this->sys->parts[rotated]->rotate(true);
                        eTemp = eTempPrev;
                    }
                }
                //если случайно проскочили энергию, меняем направление работы
                if (revert==true) {
                    if ((g.num(eTemp)) < (gaps.from(gapNumber))){
                        revert=!revert;
                    }
                } else {
                    if ((g.num(eTemp)) > (gaps.to(gapNumber))){
                        revert=!revert;
                    }
                }

                i++;
                if (i==1000000)
                    qInfo()<<"init state makes too long on gap "<<gapNumber<<", E="<<eTemp<<
                             " ("<<g.val(gaps.from(gapNumber))<<";"<<g.val(gaps.to(gapNumber)+1)<<")";
        }
        states[gapNumber] = sys->state;
    }

    const StateMachineFree initState = sys->state;
    this->f = exp(1);
    long unsigned accepted=0, rejected=0, totalSteps=0;
    this->resetH();
    updateGH(sys->E());

    qDebug()<<"steps="<<steps;

    double eOld = sys->E(),eNew=sys->E();

    while (f>fMin){
        //повторяем алгоритм сколько-то шагов
        for (unsigned i=0;i<steps;i++){
            int partNum = sys->state.randomize();

            eNew = sys->E();
            if (Random::Instance()->nextDouble() <= exp(g[eOld]-g[eNew])) {
                eOld = eNew;
                ++accepted;
            } else {
                sys->parts[partNum]->rotate(true); //откатываем состояние
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
            f=sqrt(f);
            this->resetH();
            msg("accepted ",(int)accepted);
            msg("rejected ",(int)rejected);
            msg("h is flat, new f is ",f);
            accepted=0; rejected=0;

        } else { //если гистограмма не плоская, бросаем алгоритм на интервал с большей дисперсией
            double maxDisp=0, temp=0;
            unsigned lastJumped=0;
            for (unsigned j=0; j<gaps.Gaps(); j++){
                temp = dispersion2(j);
                if (temp>maxDisp){
                    maxDisp=temp;
                    lastJumped=j;
                }
            }
            if (!gaps.inRange(g.num(sys->E()), lastJumped)){
                sys->state = states[lastJumped];
                msg("jump to",(int)lastJumped);
                updateGH();
            }
            //qInfo()<<"jump to "<<lastJumped;
        }
    }


    sys->state = initState;
    return;
}

void WLNew::saveH(const string filename) const
{
    ofstream f(filename);
    for (unsigned i=0;i<h.Intervals();i++){
        if (h.at(i)!=0.)
            f<<i<<"\t"<<h.at(i)<<"\t"<<h.val(i)<<"\t"<<h.val(i+1)<<endl;
    }

    f.close();
}

void WLNew::saveG(const string filename) const
{
    ofstream f(filename);
    for (unsigned i=0;i<g.Intervals();i++){
        if (g.at(i)!=0.)
            f<<i<<"\t"<<g.at(i)<<"\t"<<g.val(i)<<"\t"<<g.val(i+1)<<endl;
    }

    f.close();
}

//критерий плоскости гистограммы
bool WLNew::isFlat()
{
    for (unsigned i=0; i<h.Intervals(); i++){//плоскость гистограммы только в своем интервале
        if (h.at(i)!=0. && fabs(h.at(i)-average)/average > (1.0 - accuracy)) //критерий плоскости
            return false;
    }
    return true;
}

double WLNew::dispersion(unsigned gapNumber)
{
    double disp=0;
    unsigned iCount=0;
    for (unsigned i=gaps.from(gapNumber); i<=gaps.to(gapNumber); i++){//считаем матожидание
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
    for (unsigned i=gaps.from(gapNumber); i<=gaps.to(gapNumber); i++){//считаем матожидание
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
    if (E==0.){
        E = this->sys->E();
    }

    g[E]+=log(f);

    bool increased = (h[E]+=1) == 1;

    if (increased){ //прибавляем h и одновременно считаем среднее значение
        //случай если изменилось число ненулевых элементов
        hCount++;
        average = (average * (hCount-1) + 1) / hCount;
    } else {
        average += (1./(double)hCount);
    }
}

void WLNew::resetH()
{
    for (unsigned i=0; i<h.Intervals(); i++){
        h.at(i)=0;
    }
    average=0.0;
    hCount=0;
}

void WLNew::normalizeG()
{
    const double gFirst=g.at(0);
    for (unsigned i=0; i<g.Intervals(); i++){
        g.at(i)-=gFirst;
    }
}
