#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <cmath>

class baz : public cSimpleModule
{
public:
    int snsr_say;//dongu baslangicindaki sensor sayisi
    int aktif=0;//dongu sonunda hala aktif sensor sayisi
    double anten_menzil=100;//sensorlere gonderilecek
    double anten_arttir=50;//olum durumunda artirilacak menzil
    double uzak;//en uzak sensore olan uzaklik, yani en buyuk menzil degeri
    int dongu=1;
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void yayin_yap(int a,char* b);
    void kendime_mesaj(simtime_t zaman);
    int rapor_aktif=0;
    double rapor_toplam_enerji=0;

};
Define_Module(baz);

void baz::initialize()
{
    snsr_say=par("adet").longValue();//ned dosyasindan parametere olarak cektik
    uzak=par("boyut").doubleValue()*1.5;//en uzak sensor, boyut*kok2 uzaklikta olabilir
    yayin_yap(anten_menzil,(char *)"agkur");//ilk dongu
}

void baz::handleMessage(cMessage *msg)
{
    char *tip=(char *)msg->par("tip").stringValue();//gelen mesajin tipi
    if(strcmp(tip,"veri")==0)//veri geldiyse
    {
        if(msg->getParList().exist("batarya"))//mesajda batarya bilgisi var mi?
        {
            double batarya=msg->par("batarya").doubleValue();
            if(batarya<=((50e-9*4500)+(100e-12*2000*pow(anten_menzil,2))))//sensor olmek uzeredir
            {
               aktif--;
            }
        }
        aktif++;
        EV<<"sensor["<<msg->par("id").longValue()<<"] verisi : "<<msg->par("sicaklik").longValue()<<"°C"<<endl;
        //ekrana yazdir, sensor[0]=42°C
    }
    else if(strcmp(tip,"rapor")==0)
    {
        rapor_aktif++;
        rapor_toplam_enerji+=msg->par("harcadigim").doubleValue();
    }
    else if(strcmp(tip,"kendime")==0)//dongu bitmistir
    {
        std::ofstream dosya;
        dosya.open ("aktif_sensor_sayisi.txt",std::ofstream::app);
        dosya << rapor_aktif<<endl;
        dosya.close();
        dosya.open("harcanan_enerji.txt",std::ofstream::app);
        dosya <<rapor_toplam_enerji<<endl;
        dosya.close();
        rapor_aktif=0;
        rapor_toplam_enerji=0;
        if(anten_menzil>uzak && aktif>0)//butun sensorler baza ulasabiliyorsa,yeniden seviyelemeye gerek yok
        {
            snsr_say=aktif;
            aktif=0;
            yayin_yap(anten_menzil,(char *)"devam");//yeniden seviyelenme yapmadan devam edin kocum
        }
        else if(aktif<snsr_say)//olmek uzere yada olu varsa
        {
            snsr_say=aktif;
            aktif=0;
            anten_menzil+=anten_arttir;
            yayin_yap(anten_menzil,(char *)"agkur");//yeniden seviyelenme icin yayin yap
        }
        else if(aktif>0)//butun sensorler olene kadar
        {
            snsr_say=aktif;
            aktif=0;
            yayin_yap(anten_menzil,(char *)"devam");//yeniden seviyelenme yapmadan devam edin kocum
        }
    }
}

void baz::yayin_yap(int a,char* b)
{
    if (strcmp(b,"agkur")==0)
    {
        for (int i=0;i<(gateCount())/2;i++)
        {
               bubble("DONGU BASLIYOR");
               double x=atoi(getDisplayString().getTagArg("p",0));//similasyon goruntu ayarlarından konum al
               double y=atoi(getDisplayString().getTagArg("p",1));
               cMessage *msg=new cMessage("yayin");
               msg->addPar("tip");
               msg->par("tip")="yayin";
               msg->addPar("x");//mesaja konum koy
               msg->par("x")=x;
               msg->addPar("y");
               msg->par("y")=y;
               msg->addPar("anten_menzil");//mesaja anten menzilini koy
               msg->par("anten_menzil")=a;
               send(msg,"cikis",i);
        }
    }
    else if(strcmp(b,"devam")==0)
    {
        for (int i=0;i<(gateCount())/2;i++)
        {
               bubble("DONGU BASLIYOR");
               cMessage *msg=new cMessage("yayin");
               msg->addPar("tip");
               msg->par("tip")="devam";
               send(msg,"cikis",i);
        }
    }
    kendime_mesaj(dongu*1000);//dongu sonunda tetiklenmek icin; her dongu 1000 sn periyod
    dongu++;
}

void baz::kendime_mesaj(simtime_t zaman)
{
    cMessage *msg=new cMessage("kendime");//tetikleme mesaji yaratip kendime gonderecem
    msg->addPar("tip");
    msg->par("tip")="kendime";
    scheduleAt(zaman,msg);
}



