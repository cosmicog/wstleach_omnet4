/*
 * base.cc
 *
 *  Created on: Dec 20, 2014
 *      Author: orhangazi44
 *
 *      LGPL
 *
 */

#include "base.h"

Define_Module(Base);

void Base::initialize()
{
    mainModule = getModuleByPath("LeachBasic");
    p = mainModule->par("p").doubleValue();
    numNodes = mainModule->par("numNodes");
    clusterMaxMember = (int)(1 / p);
    numClusters = (numNodes / clusterMaxMember);
    numberOfLivingNodes = numNodes;
    receivedRoundCount = 0;
    totalExpendedEnergy = .0;
    nodeDeaths = new bool[ numNodes ];
    nodeBatteries = new double[ numNodes ];
    posXs = new int [numNodes];
    posYs = new int [numNodes];
    chNumaralari = new int [numNodes];
    bazaMesafeler = new double[ numNodes ];
    kalanEnerjiler = new double[ numNodes ];
    komsuSayilari = new int [numNodes];
    kumeElemanSayilari = new int [numNodes];
    full_battery = mainModule->par("nodeBatteries");
    agirliklar = new double * [ numNodes ];
    secilenler = new bool [ numNodes ];
    atalar = new int [numNodes];
    agacaEklenenler = new int [numNodes];
    baglanilanKapilar = new int [numNodes];
    baglananSayilari = new int [numNodes];

    for(int i = 0; i < numNodes; i++)
    {
        nodeDeaths[i] = false; // Buna sonra bak
        nodeBatteries[i] = full_battery;
        posXs[i] = 0;
        posYs[i] = 0;
        chNumaralari[i] = -1;
        bazaMesafeler[i] = -1.0;
        kalanEnerjiler[i] = -1.0;
        komsuSayilari[i] = -1;
        kumeElemanSayilari[i] = -1;
        secilenler[i] = false;
        atalar[i] = -1;
        agacaEklenenler[i] = -1;
        baglanilanKapilar[i] = -1;
        baglananSayilari[i] = 0;
    }
    roundChSayisi = 0;
    agactakiSayi = 0;

    int agirlikMaxEleman = numNodes + 1; // Artı baz istasyonu
    for(int i = 0; i < agirlikMaxEleman; i++)
    {
        agirliklar[i] = new double[ agirlikMaxEleman ];
        for(int j = 0; j < agirlikMaxEleman; j++)
        {
            agirliklar[i][j] = -1.0;
        }
    }

    agacOlusturmaEvent = new cMessage("agacOlustur");
    scheduleAt(4.0, agacOlusturmaEvent); // sadece ilk sefer cancel edebilmek içiçn.

    WATCH(numberOfLivingNodes);
    WATCH(receivedRoundCount);

    char* str = new char[16];
    xPosition = mainModule->par("bazX");
    yPosition = mainModule->par("bazY");
    sprintf(str, "(%d, %d)", xPosition, yPosition);
    getDisplayString().setTagArg("t", 0, str);
    getDisplayString().setTagArg("p", 0, xPosition);
    getDisplayString().setTagArg("p", 1, yPosition);

    totalEnergyVector.setName("Total Energy Vector");
    numberOfLivingNodesVector.setName("Number Of Living Nodes Vector");
    receivedRoundCountVector.setName("Received Round Count Vector");
    expendedEnergiesForRoundsVector.setName("Expended Energies For Rounds Vector");

    totalEnergyVector.record(totalExpendedEnergy);
    numberOfLivingNodesVector.record(numberOfLivingNodes);
    receivedRoundCountVector.record(receivedRoundCount);
    expendedEnergiesForRoundsVector.record(0.0);

    algorithm = mainModule->par("algorithm");
    if (algorithm != LEACH && algorithm != IMPLEACH) // leach ve impleach de Nb hesaplama işlemleri başlatılmayavak
    {
        cMessage * msg1 = new cMessage("xyToNodes");
        simtime_t stt1 = 0.2;
        scheduleAt(stt1, msg1);
    }

    cMessage * msg1 = new cMessage("recordEnergy");
    simtime_t stt1 = 0.1;
    scheduleAt(stt1, msg1);
}

void Base::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(), "chInf") == 0 )
    {
        cancelEvent(agacOlusturmaEvent); // Her bilgi gelişinde sıfırlanacak zaten aynı eşit t anında geliyor tüm hepsi.

        ch2baseInfoMsg * chmsg1 = check_and_cast<ch2baseInfoMsg *>(msg);
        chNumaralari[roundChSayisi] = chmsg1->getKaynakNo();
        bazaMesafeler[roundChSayisi] = chmsg1->getBazaMesafe();
        kalanEnerjiler[roundChSayisi] = chmsg1->getKalanEnerji();
        komsuSayilari[roundChSayisi] = chmsg1->getKomsuSayisi();
        kumeElemanSayilari[roundChSayisi] = chmsg1->getKumeUyeSayisi();
        roundChSayisi ++;

        scheduleAt(simTime()+0.1, agacOlusturmaEvent); // Ve aynı şekilde hepsi yeniden schedule edecek...
        delete msg;
        return;
    }
    if(strcmp(msg->getName(), "agacOlustur") == 0 )
    {
        int elemanSayisi = roundChSayisi +1;
        chNumaralari[roundChSayisi] = -1453; // Baz istasyonu
        bazaMesafeler[roundChSayisi] = .0;
        kalanEnerjiler[roundChSayisi] = 0.0;
        komsuSayilari[roundChSayisi] = 0.0;
        kumeElemanSayilari[roundChSayisi] = 0.0;
        int ino, jno;
        double ijdist;
        for(int i = 0; i < elemanSayisi; i++)
        {
            for(int j = 0; j < elemanSayisi; j++)
            {
                if(chNumaralari[i] == -1453)
                {
                    agirliklar[i][j] = .0;
                }
                else if(chNumaralari[j] == -1453)
                {
                    agirliklar[i][j] = (kalanEnerjiler[i]/full_battery) * (komsuSayilari[i]/kumeElemanSayilari[i]) * pow(bazaMesafeler[i], 4);
                }
                else
                {
                    if(i == j) agirliklar[i][j] = .0;
                    else
                    {
                        ino = chNumaralari[i];
                        jno = chNumaralari[j];
                        ijdist = calculateDistanceInBS(POSXS[ino], POSYS[ino], POSXS[jno], POSXS[jno]);
                        agirliklar[i][j] = (kalanEnerjiler[i]/full_battery) * (komsuSayilari[i]/kumeElemanSayilari[i]) * pow((1/ijdist), 2);
                    }
                }
            }
        }
        double max_agirlik = -1453; // rastgele sayı
        int max_no; // ch listesindeki numara, 0, 1, 2, 3, ... diye gider
        for(int i = 0; i < roundChSayisi; i++)
        {
            if (max_agirlik < agirliklar[i][roundChSayisi])
            {
                max_agirlik = agirliklar[i][roundChSayisi];
                max_no = i;
            }
        }
        atalar[max_no] = -1453;
        secilenler[max_no] = true;
        agacaEklenenler[0] = max_no;
        agactakiSayi = 1;
        while (agactakiSayi < roundChSayisi)
        {
            max_agirlik = -9999999999.99; // rastgele sayı
            max_no = -1;
            for(int i = 0; i < roundChSayisi; i++)
            {
                if(!secilenler[i])
                {
                    for(int j = 0; j < agactakiSayi; j++)
                    {
                        if ( max_agirlik < agirliklar [i] [ agacaEklenenler[j] ] )
                        {
                            max_agirlik = agirliklar [i] [ agacaEklenenler[j] ];
                            max_no = agacaEklenenler[j];
                        }
                    }
                    atalar[i] = max_no;
                    secilenler[i] = true;
                    agacaEklenenler[agactakiSayi] = i;
                    agactakiSayi++;
                }
            }
        }
        EV << "AGACA EKLENENLER    ";
        for (int i = 0; i < numNodes; i++)
        {
            EV << agacaEklenenler[i] << ", ";
        }EV << "\n";

        for(int i = 0; i < roundChSayisi; i++)
        {
            for(int j = 0; j < roundChSayisi; j++)
            {
                if(atalar[i] == j)
                {
                    baglananSayilari[j]++;
                    baglanilanKapilar[i] = baglananSayilari[j];
                }
            }
        }

        EV << "BAGLANILANLAR   ";
        for(int i = 0; i < roundChSayisi; i++) EV << chNumaralari[atalar[i]] << ", ";
        EV << "\n";
        base2chInfoMsg * b2cm;
        char *chr = new char[32];
        for(int i = 0; i < roundChSayisi; i++)
        {
            b2cm = new base2chInfoMsg("agacInf");
            sprintf(chr, "LeachBasic.nd[%d]", chNumaralari[i]);
            b2cm->setBaglanacakSayi(baglananSayilari[i]);
            b2cm->setGonderilecekKapiNo(baglanilanKapilar[i]);
            if(atalar[i]== -1453) b2cm->setGonderilecekNo(-1453);
            else b2cm->setGonderilecekNo(chNumaralari[atalar[i]]);
            EV << chr << " 'ye gönderilmeye çalışılıyor...........................\n";
            sendDirect(b2cm, getModuleByPath(chr), "in");
        }
        EV << "AGAÇ GÖNDERİLDİ XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";

        for(int i = 0; i < numNodes; i++)
        {
            chNumaralari[i] = -1;
            bazaMesafeler[i] = -1.0;
            kalanEnerjiler[i] = -1.0;
            komsuSayilari[i] = -1;
            kumeElemanSayilari[i] = -1;
            secilenler[i] = false;
            atalar[i] = -1;
            agacaEklenenler[i] = -1;
            baglanilanKapilar[i] = -1;
            baglananSayilari[i] = 0;
        }

        for(int i = 0; i < (numNodes + 1); i++)
        {
            for(int j = 0; j < (numNodes + 1); j++)
            {
                agirliklar[i][j] = -1.0;
            }
        }
        roundChSayisi = 0;
        agactakiSayi = 0;

        return;
    }

    if(strcmp(msg->getName(), "recordEnergy") == 0 )
    {
        if(noEnergy)
        {
            EV << "NO MORE INFORMATION, SIMULATION IS STOPPING!!!!!!!!XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
            return;
        }
        EV << "RECORDING ENERGY=======================================XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  " << expendedEnergyForRound << "    XXXXXXXXXX\n";

        // Yaşayan düğümleri say
//        int count = 0;
//        for(int i = 0; i < numNodes; i++)
//        {
//            if (nodeDeaths[i]) count++;
//        }

        expendedEnergiesForRoundsVector.record(expendedEnergyForRound);
        // expendedEnergyForRound = .0;
        noEnergy = true;
        scheduleAt(simTime()+4, msg);
        return;
    }
    if(strcmp(msg->getName(), "xyToNodes") == 0 )
    {
        for(int i = 0; i < numNodes; i++)
        {
            char *c = new char[64];
            sprintf(c, "%d:", posXs[i]);
            posInfoMessageString.append(c);
            sprintf(c, "%d", posYs[i]);
            posInfoMessageString.append(c);
            posInfoMessageString.append("_"); // ilgili düğümün bilgisi stringe eklenince bitirmek için '_' koydum, sonra split edeceğim.
        }

        EV << "POSXS[" << numNodes << "] = {";
        for(int i = 0; i < numNodes; i++)
        {
            EV << posXs[i] << ",";
        }
        EV << "};\n";

        EV << "POSYS[" << numNodes << "] = {";
        for(int i = 0; i < numNodes; i++)
        {
            EV << posYs[i] << ",";
        }
        EV << "};";

        for(int i = 0; i < numNodes; i++)
        {
            head2baseMsg * msg0 = new head2baseMsg("info");
            msg0->setMes(posInfoMessageString.c_str());
            msg0->setKind(2);
            char *chr = new char[32];
            sprintf(chr, "LeachBasic.nd[%d]",i);
            sendDirect(msg0, getModuleByPath(chr), "in");
        }
        return;
    }

    head2baseMsg * msg1 = check_and_cast<head2baseMsg *>(msg);

    if(strcmp(msg->getName(), "die") == 0 && nodeDeaths[msg1->getSource()] == false)
    {
        nodeDeaths[msg1->getSource()] = true;
        numberOfLivingNodes--;
        numberOfLivingNodesVector.record(numberOfLivingNodes);
        return;
    }

    if(strcmp(msg->getName(), "myXY") == 0 )
    {
        int source = msg1->getSource();
        posXs[source] = msg1->getPosX();
        posYs[source] = msg1->getPosY();
    }

    if(strcmp(msg->getName(), "myBattery") == 0 )
    {
        noEnergy = false;
        int src = msg1->getSource();
        double olddd = nodeBatteries[src];
        nodeBatteries[src] = msg1->getTotalExpendedEnergy();
        expendedEnergyForRound += (olddd - nodeBatteries[src]);
    }

    if(strcmp(msg1->getName(), "h2bM") == 0)
    { // Message arrived.
        EV << "BASE: Message " << msg->getName() << " arrived from node[" << msg->getSenderModuleId()-2 << "].\n";
        EV << "BASE: RECEIVED DIE COUNT : " << msg1->getDieCount() << " *********************************************************************.\n";
        bubble("ARRIVED");
    }
    delete msg;
}

void Base::finish()
{
    recordScalar("#receivedRoundCount", receivedRoundCount);
    numberOfLivingNodesStats.recordAs("Number of Living Nodes");
}





