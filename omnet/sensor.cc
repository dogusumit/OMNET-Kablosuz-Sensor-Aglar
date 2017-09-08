#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <cmath>//matematik

class sensor : public cSimpleModule
{
public:
    int seviye=0;
    int komsu[100];//mesaj yollayacagim ust seviyelerin tutuldugu dizi
    int kmsu_say;//onceki satirdaki diziye indis olarak kullanilacak
    double anten_menzil;//baz gonderecek
    int dongu=-1;
    double batarya=1.0;
    double konum_x;
    double konum_y;
    bool hayat=true;
    double harcadigim=0;
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    double uzaklik_hesapla(double x1,double y1,double x2,double y2);//iki nokta arasi oklid uzakligi dondurur
    void yayin_yap();
    void veri_yolla();
    void kendime_mesaj(char* ne_icin,simtime_t zaman);
    char* renklendir(int a);//parametre olarak seviye alip renk stringi dondurur
    void rapor_yolla();//ag hakkinda raporlama icin;log dosyasi
};
Define_Module(sensor);


void sensor::initialize()
{
    int boyut=par("boyut").longValue();
    int a=intuniform(0,boyut);//0-boyut arasi random integer, konum icin
    int b=intuniform(0,boyut);
    getDisplayString().setTagArg("p",0,a);//gorunum ayarlarina konumu yaz
    getDisplayString().setTagArg("p",1,b);
    konum_x=(double)a;//konumumu kaydettim
    konum_y=(double)b;
    EV<<"sensor"<<idx<<" :  "<<konum_x<<","<<konum_y<<endl;//"sensor[0] : 300,250" seklinde konumumu ekrana yazdim
}

void sensor::handleMessage(cMessage *msg)
{
    if (hayat)//hayatta miyim?
    {
        char *tip=(char *)msg->par("tip").stringValue();//gelen mesajin tipi

        if(strcmp(msg->getSenderModule()->getName(),"baz")==0)//mesaj bazdan geldiyse
        {
            dongu++;
            if(strcmp(tip,"yayin")==0)//baz seviyelenme istiyor
            {
                double baz_x=msg->par("x").doubleValue();//bazin konumunu aldim
                double baz_y=msg->par("y").doubleValue();
                anten_menzil=msg->par("anten_menzil").doubleValue();//anten menzilini atadim
                kmsu_say=0;//iki ve sonraki seviyeleme dongulerinde eski komsulari unutmak icin
                seviye=0;
                if(uzaklik_hesapla(baz_x,baz_y,konum_x,konum_y)<=anten_menzil)//baza ulasabiliyorsam
                {
                    seviye=1;
                    getDisplayString().setTagArg("i",1,renklendir(seviye));//similasyonda sensoru boyadim
                    komsu[kmsu_say]=idx;//idx kapisi baza ulasiyor, bazi komsum olarak ekledim
                    kmsu_say++;//indisi arttirdim
                    kendime_mesaj((char *)"yayin_yapak",0);//yayin yapmak icin tetikleme mesaji yaratiyorum
                }
            }
            kendime_mesaj((char*)"veri_gonderek",800);//veri gondermek icin tetikleme mesaji yaratiyorum
            kendime_mesaj((char*)"rapor_gonderek",900);
            batarya-=(50e-9*500);
            harcadigim+=(50e-9*500);
        }

        else//mesaj bazdan gelmediyse
        {
            if(strcmp(tip,"yayin")==0)//yayin mesajiysa
            {
                if(seviye==0 || msg->par("seviye").longValue()+1<seviye)//seviyem belli degil veya seviyemi dusurme ihtimalim varsa
                {
                    double komsu_x=msg->par("x");//mesaji gonderinin konumunu aldim
                    double komsu_y=msg->par("y");
                    if(uzaklik_hesapla(komsu_x,komsu_y,konum_x,konum_y)<=anten_menzil)//gonderene ulasabiliyormuyum
                    {
                        kmsu_say=0;
                        komsu[kmsu_say]=msg->par("id").longValue();//gondereni komsum olarak ekledim
                        kmsu_say++;//indisi arttirdim
                        seviye=msg->par("seviye").longValue();
                        seviye++;//seviyemi belirledim
                        getDisplayString().setTagArg("i",1,renklendir(seviye));//similasyonunda sensoru boyadim
                        kendime_mesaj((char *)"yayin_yapak",(seviye-1)*100);//yayin yapmak icin tetikleme mesaji yaratiyorum
                        batarya-=(50e-9*500);
                        harcadigim+=(50e-9*500);
                    }
                }
                else if(msg->par("seviye").longValue()<seviye)//mesaj bir alt seviyeden geldiyse
                {
                    double komsu_x=msg->par("x");
                    double komsu_y=msg->par("y");
                    if(uzaklik_hesapla(komsu_x,komsu_y,konum_x,konum_y)<=anten_menzil)//ulasabiliyorsam
                    {
                        komsu[kmsu_say]=msg->par("id");//gondereni komsum olarak ekledim
                        kmsu_say++;
                        batarya-=(50e-9*500);
                        harcadigim+=(50e-9*500);
                    }
                }
            }

            else if(strcmp(tip,"kendime")==0)//kendime tetikleme mesaji attiysam
            {
                if(kmsu_say>0)//kmsu_say=0 sa,anten seviyesi kimseye ulasmama yetmiyor
                {
                    if (strcmp(msg->par("napak").stringValue(),"yayin_yapak")==0)//yayin tetiklemesi
                    {
                        batarya-=((50e-9*500)+(100e-12*500*pow(anten_menzil,2)));
                        harcadigim+=((50e-9*500)+(100e-12*500*pow(anten_menzil,2)));
                        yayin_yap();
                    }
                    else if(strcmp(msg->par("napak").stringValue(),"veri_gonderek")==0)//veri tetiklemesi
                    {
                        batarya-=((50e-9*2000)+(100e-12*2000*pow(anten_menzil,2)));
                        harcadigim+=((50e-9*2000)+(100e-12*2000*pow(anten_menzil,2)));
                        veri_yolla();
                    }
                }
                if(strcmp(msg->par("napak").stringValue(),"rapor_gonderek")==0)//rapor tetiklemesi
                {
                    rapor_yolla();
                    harcadigim=0;
                }
            }

            else if(strcmp(tip,"veri")==0)//sensor verisi geldiyse
            {
                batarya-=(50e-9*2000);//mesaji aldigim icin
                batarya-=((50e-9*2000)+(100e-12*2000*pow(anten_menzil,2)));//mesaji ilettigim icin
                harcadigim+=(50e-9*2000);
                harcadigim+=((50e-9*2000)+(100e-12*2000*pow(anten_menzil,2)));
                msg->addPar("batarya");//mesaja bataryami da ekliyeyim
                msg->par("batarya")=batarya;
                send(msg,"cikis",komsu[rand()%kmsu_say]);//ust seviyeden rasgele komsuma ilettim
            }
        }
        if(batarya<=0) //batarya bittiyse
        {
            hayat=false;
            bubble("Ben bittim la!");
            getDisplayString().setTagArg("i",1,"black");
        }
    }
    else if(harcadigim>0)//olmeden once enerji harcadiysam
    {
        rapor_yolla();
        harcadigim=0;
    }
}


double sensor::uzaklik_hesapla(double x1,double y1,double x2,double y2)
{
    double a=sqrt(pow((x1-x2),2)+pow((y1-y2),2));//oklid uzaklik hesapla
    return a;
}

void sensor::yayin_yap()
{
    for(int i=0;i<gateCount()/2;i++)
    {
        if(i!=idx)//idx kapisi baza gidiyor
        {
            cMessage *msg=new cMessage("yayin");
            msg->addPar("tip");
            msg->par("tip")="yayin";
            msg->addPar("seviye");
            msg->par("seviye")=seviye;
            msg->addPar("id");
            msg->par("id")=idx;
            msg->addPar("x");
            msg->par("x")=konum_x;
            msg->addPar("y");
            msg->par("y")=konum_y;
            send(msg,"cikis",i);
        }
     }
}

void sensor::veri_yolla()
{
    cMessage *msg=new cMessage("veri");
    msg->addPar("tip");
    msg->par("tip")="veri";
    msg->addPar("id");
    msg->par("id")=idx;
    msg->addPar("sicaklik");
    msg->par("sicaklik")=rand()%50;//rasgele sicaklik verisi olusturdum
    if(seviye==1)//seviye 1 se
    {
        msg->addPar("batarya");//mesaja bataryami da ekliyeyim
        msg->par("batarya")=batarya;
    }
    send(msg,"cikis",komsu[rand()%kmsu_say]);//alt seviyeden rasgele komsuya yollicam
}

void sensor::kendime_mesaj(char* ne_icin,simtime_t zaman)
{
    cMessage *msg=new cMessage("kendime");//tetikleme mesaji yaratip kendime gonderecem
    msg->addPar("tip");
    msg->par("tip")="kendime";
    msg->addPar("napak");
    msg->par("napak")=ne_icin;
    scheduleAt((dongu*1000)+zaman+idx,msg);//similasyonda gorsel cakisma olmamasi icin,zamanlama ayarladim
}

char* sensor::renklendir(int a)
{
    switch(a)
    {
    case 1:return (char*)"magenta";
    case 2:return (char*)"blue";
    case 3:return (char*)"green";
    case 4:return (char*)"yellow";
    case 5:return (char*)"#123456";
    case 6:return (char*)"#654321";
    case 7:return (char*)"#194c03";
    case 8:return (char*)"#b27007";
    case 9:return (char*)"#9cdb08";
    case 10:return (char*)"#086fc9";
    default:return (char*)"#ffffff";
    }
}
void sensor::rapor_yolla()
{
    cMessage *msg=new cMessage("rapor");//tetikleme mesaji yaratip kendime gonderecem
    msg->addPar("tip");
    msg->par("tip")="rapor";
    msg->addPar("harcadigim");
    msg->par("harcadigim")=harcadigim;
    send(msg,"cikis",idx);//baza raporu yolladim
}
