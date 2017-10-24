/*
 * node.cc
 *
 *  Created on: Dec 20, 2014
 *      Author: orhangazi44
 *
 *      LGPL
 *
 */

#include "node.h" // Diğer kütüphaneler ve kullandığım adresten bağımsız fonksiyonlar bunda.

Define_Module(Node);


int Node::getSiraId(int getid = -1)
{
    if (getid == -1) getid = getId();
    for(int i = 0; i < chSayisi; i++)
    {
        if (zincirDizi[i] == getid - 2)
        {
            benimSiraNo = i;
            return i;
        }
    }
    return -1;
}

void Node::zincirOlustur()
{
    EV << "ZİNCİR OLUŞTURMA BAŞIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n";
    liderNo = bazaEnYakiniBul();
    if (getNodeId() == (liderNo)) liderim = true;
    else liderim = false;

    onceMiSonraMi[liderNo] = -1;

    if (liderim)
    {
        char * cc = new char[8];
        sprintf(cc, "#000000");
        getDisplayString().setTagArg("i", 1, cc); //renk ata
        clusterHead = server;

        gate("cikisKapi")->connectTo(server->gate("girisKapi"));
        EV << getNodeId() << "--> BAZA BAĞLANDIMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n" ;
    }
    else
    {
        int en_uzak_sira_id = bazaEnUzagiBul();
        if(en_uzak_sira_id == benimSiraNo) baslangicim = true;

        //zincirDizi = new int [chSayisi];
        zincirDizi[0] = en_uzak_sira_id;
        secilenler[en_uzak_sira_id] = true;
        EV << zincirDizi[0] << ",";
        int a;
        for(int i = 1; i < chSayisi; i++)
        {
            a = secilmemislerinEnYakini(zincirDizi[i-1]);
            if(a == -1)
            {
                baslangicim = true;
                zincirDizi[i] = chListesi[i];
                EV << zincirDizi[i] << ",";
                continue;
            }
            zincirDizi[i] = a;
            if (a == liderNo) EV << " ---> " << zincirDizi[i] << " <--- ";
            else EV << zincirDizi[i] << " ";
        }
        if (a == getNodeId()) sonum = true; // Son a değeri son düğüm olacak.
        EV << "\n";

        bool oncesi = true;
        for(int i = 0; i < chSayisi; i++)
        {
            if (zincirDizi[i] == liderNo) oncesi = false;
            if (oncesi) onceMiSonraMi[i] = ONCE;
            else onceMiSonraMi[i] = SONRA;
            if (zincirDizi[i] == liderNo) onceMiSonraMi[i] = NE_ONCE_NE_SONRA; // fazla satır olmaması için
        }

        // ÖLEN KONTROLLERİ SONRA EKLE
        EV << getNodeId() << "  AAAAAAAAAAAAAAAAAA     "; for (int i = 0; i < NUM_NODES; i++) EV << zincirDizi[i] << ", "; EV << "\n";
        EV << getNodeId() << "  AAAAAAAAAAAAAAAAAA     "; for (int i = 0; i < NUM_NODES; i++) EV << onceMiSonraMi[i] << ", "; EV << "\n";
        if(!olenler[getSiraId()])
        {
            char * chr = new char[128];
            if (onceMiSonraMi[getSiraId()] == ONCE)
            {
                sprintf(chr, "LeachBasic.nd[%d]", zincirDizi[getSiraId()+1]);
                clusterHead = getModuleByPath(chr);
                gate("cikisKapi")->connectTo(clusterHead->gate("girisKapi"));
                int chNo = zincirDizi[getSiraId()+1];
                chDistance = calculateDistance(xPosition, yPosition, POSXS[chNo], POSYS[chNo]);
            }
            else if(onceMiSonraMi[getSiraId()] == SONRA)
            {
                sprintf(chr, "LeachBasic.nd[%d]", zincirDizi[getSiraId() - 1]);
                clusterHead = getModuleByPath(chr);
                int chNo = zincirDizi[getSiraId()-1];
                chDistance = calculateDistance(xPosition, yPosition, POSXS[chNo], POSYS[chNo]);
                if (onceMiSonraMi[getSiraId()-1] == NE_ONCE_NE_SONRA)
                {// eğer ch den bir sonraki isem diğer kapı dolu olduğu için ikinci kapıya bağlan
                    gate("cikisKapi")->connectTo(clusterHead->gate("girisKapi2"));
                }
                else
                {
                    gate("cikisKapi")->connectTo(clusterHead->gate("girisKapi"));
                }
            }

            EV << "ZİNCİR OLUŞTURMA SONUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\n";
//            char * cc = new char[8];
//            sprintf(cc, "#BBBBBB");
//            getDisplayString().setTagArg("i", 1, cc); //renk ata
        }
    }
}


void Node::bazaUzakliklariHesapla() // Bir kere başlangıçta çalışacak
{
    for (int i = 0; i < NUM_NODES; i++)
    {
        bazaUzakliklar[i] = calculateDistance(bazX, bazY, POSXS[i], POSYS[i]);//calculateDistance();
    }
}

void Node::dugumeUzakliklariHesapla(int dugumSiraNo) // En başta her düğüm için çalışacak
{
    for (int i = 0; i < NUM_NODES; i++)
    {
        dugumUzakliklari[dugumSiraNo][i] = calculateDistance(POSXS[dugumSiraNo], POSYS[dugumSiraNo] , POSXS[i], POSYS[i]);//calculateDistance();
    }
}

int Node::secilmemislerinEnYakini(int dugumNo)
{
    double en_yakin = 9999999999999.0; /* TO DO */
    int en_yakin_sira  = -1;
    bool tamamiSecildi = true;
    for(int i = 0; i < chSayisi; i++)
    {
        int nomuz = chListesi[i];
        if (!secilenler[nomuz]) tamamiSecildi = false;
        if (dugumUzakliklari [dugumNo][nomuz] < en_yakin && nomuz != dugumNo && !secilenler[nomuz])
        {
            en_yakin = dugumUzakliklari [dugumNo][nomuz];
            en_yakin_sira = nomuz;
        }
    }
    if(tamamiSecildi) return en_yakin_sira; //-1
    secilenler[en_yakin_sira] = true;
    return en_yakin_sira;
}


int Node::getZincirSira(int getid)
{
    for(int i = 0; i < chSayisi; i++)
    {
        if (zincirDizi[i] == getid) return i;
    }
    return -1;
}

int Node::bazaEnUzagiBul()
{
    int en_uzak_id = -1;
    double en_uzak = .0;

    for(int i = 0; i < chSayisi; i++ )
    {
        if (bazaUzakliklar[chListesi[i]] > en_uzak)
        {
            en_uzak = bazaUzakliklar[chListesi[i]];
            en_uzak_id = chListesi[i];
        }
    }
    return en_uzak_id;
}

int Node::bazaEnYakiniBul()
{
    int en_yakin_id = -1;
    double en_yakin = 99999999999999999.9;

    for(int i = 0; i < chSayisi; i++ )
    {
        if (bazaUzakliklar[chListesi[i]] < en_yakin)
        {
            en_yakin = bazaUzakliklar[chListesi[i]];
            en_yakin_id = chListesi[i];
        }
    }
    return en_yakin_id;
}


/* Buraya kadarki fonksiyonlar sadece CH ler için */


void Node::die()
{
    cancelEvent(nextRoundEvent);
    head2baseMsg *hmsg = new head2baseMsg("die");
    hmsg->setSource(getNodeId());
    sendDirect(hmsg, server, "in");
    cMessage * clearMsg = new cMessage("clr");
    scheduleAt(simTime() + 2.1, clearMsg); //3 olursa sonraki zinciri etkiler, 2 den az olursa o ankini
}

double Node::reduceBattery(double distance, int sendOrReduce, int type = INFO, int packetCount = 1)
{
    int localK;
    if(type == DATA) localK= (K*packetCount);
    if(type == INFO) localK= ((K/4)*packetCount);
    double redu;
    if (sendOrReduce == SEND_REDUCE)
    {
        if (distance < THRS_DIST) redu = (E_ELEC * localK) + ( E_FS * localK * pow(distance, 2) );
        else                      redu = (E_ELEC * localK) + ( E_MP * localK * pow(distance, 4) );
    }
    else if (sendOrReduce == RECEIVE_REDUCE)
    {
        redu = E_ELEC * localK;
    }
    else if (sendOrReduce == DATA_AGGREGATION_REDUCE)
    {
        redu = aggBattery * packetCount;
    }

    battery -= redu;
    if(getNodeId() == 125 ) EV << "125 in bataryası : " << battery << "  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx********************\n";
    if (battery > 0.0)
    {
        head2baseMsg * msg3 = new head2baseMsg("myBattery");
        msg3->setSource(getNodeId());
        msg3->setTotalExpendedEnergy(battery);
        //sendDirect(msg3, server, "in");
    }
    else
    {
        head2baseMsg * msg3 = new head2baseMsg("myBattery");
        msg3->setSource(getNodeId());
        msg3->setTotalExpendedEnergy(0.0);
        //sendDirect(msg3, server, "in");
    }

    char *cs = new char[32];
    if ( battery > (fullBattery*0.80) )
    {
        sprintf(cs, "status/battery_80");
        getDisplayString().setTagArg("i2", 0, cs); // status ikon ata
    }
    else if ( battery > (fullBattery*0.60) )
    {
        sprintf(cs, "status/battery_60");
        getDisplayString().setTagArg("i2", 0, cs); // status ikon ata
    }
    else if ( battery > (fullBattery*0.40) )
    {
        sprintf(cs, "status/battery_40");
        getDisplayString().setTagArg("i2", 0, cs); // status ikon ata
    }
    else if ( battery > (fullBattery*0.20) )
    {
        sprintf(cs, "status/battery_20");
        getDisplayString().setTagArg("i2", 0, cs); // status ikon ata
    }
    else if ( battery <= (fullBattery*0.20) && battery > (fullBattery*0.05))
    {
        sprintf(cs, "status/battery_0");
        getDisplayString().setTagArg("i2", 0, cs); // status ikon ata
    }
    else
    {
        die();
        iAmDead = true;
        char *ci = new char[32];
        sprintf(ci, "old/x_cross");
        getDisplayString().removeTag("i2");
        getDisplayString().setTagArg("i", 0, ci); // ikon ata
    }
    return redu;
}

void Node::beginNextRound()
{
    globalRound++;
    if (curRound == CLUSTER_DEFAULT_MEMBER-1) curRound = 0;
    else curRound++;
    receivedRoundCountVector.record(globalRound);
}

int Node::getNodeId()
{
   return getId()-2;  /*DİKKATTTTTTTTTT -2 tehlikeli, deneme için sadece*/
}

void Node::updateDisplayStringPositionAndTag(int x, int y)
{
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);

    char* chr = new char[16];
    sprintf(chr, "(%d,%d)", x, y); //
    /*getDisplayString().setTagArg("t", 0, chr);*/ /* Bu kısım ister kaldırılır, ister kullanılır, Modül üerinde koordinatı yazar.*/
}

double Node::treshold(int t_algorithm = LEACH)
{
    if (t_algorithm == LEACH)    return (P / (  1.0  -  P * (globalRound % (int)(1.0/P))  ));
    if (t_algorithm == IMPLEACH) return  (P / (  1.0  -  P * (globalRound % (int)(1.0/P))  )) * sqrt(battery / fullBattery);
    if (t_algorithm == CCRP)
    {
        double Ec = battery;
        double Ei = fullBattery;
        int Rc = globalRound;
        return  (double)(P / (  1.0  -  P * (Rc % (int)(1.0/P))  )) * ( (Ec/Ei) + (1-(Ec/Ei)) * (Nb / (1/P - 1)));
    }
    if (t_algorithm == WSTLEACH)
    {
        double Eres = battery;
        double E0 = fullBattery;
        int Rc = globalRound;
        double W1 = 0.2;
        double W2 = 0.3;
        double W3 = 0.5;
        double result = (double)(P / (  1.0  -  P * (Rc % (int)(1.0/P))  )) * (  (W1 * (Eres/E0))  +  (W2 * (Nb/ (P * NUM_NODES)))  +  (W3 * (1/dBS))  );
        EV <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  " << (W2 * (Nb/ (P * NUM_NODES))) << "   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n";
        return result;
    }
    else return .0;
}

void Node::beginHeadSelection()
{
    double nDeg = uniform(0.0, 1.0);
    double tDeg = treshold(algorithm);

    EV << "tDeg = " << tDeg << "\n";
    EV << "nDeg = " << nDeg << "\n";

    if (gDeg == 0)
    {
        if (nDeg < tDeg)
        {
            iAmHead = true;
            gDeg = (int)(1.0/P);
        }
        else iAmHead = false;
    }
    else if (gDeg > 0)
    {
        gDeg --;
        iAmHead = false;
    }

//    if (getNodeId() % int(1/P) == curRound) iAmHead = true;
//    else iAmHead = false;

    if (iAmHead)
    {
        EV << getNodeId() << " --> CUR_ROUND : " << globalRound << "===========" << "(" << xPosition << ", " << yPosition << ") " <<"\n";
        broadcastSent = false; // eğer yeniden ch olursa diye
        chDistance = MIN_DISTANCE;
        headWaitEvent = new cMessage("hwM");

        char *ci = new char[32];
        char *cc = new char[7];

        sprintf(ci, "misc/node");
        sprintf(cc, color);

        getDisplayString().setTagArg("i", 0, ci); //ikon ata
        getDisplayString().setTagArg("i", 1, cc); //renk ata
    }
    else
    {
        EV << getNodeId() << " --> CUR_ROUND : " << globalRound << " <---------------------------------------------------\n";
        sprintf(chFullPath, "LeachBasic.base");
        clusterHead = server;
        char *ci = new char[32];
        sprintf(ci, "misc/node_s");

        getDisplayString().setTagArg("i", 0, ci); //ikon ata
        getDisplayString().setTagArg("i", 1, "#BBBBBB"); //renk ata
    }
}

void Node::initialize()
{
    if(first_init)
    {
        battery = par("battery");
        fullBattery = par("battery");
        globalRound = 1;
        color = generateRandomColor();
        first_init = false;
        mainModule = getModuleByPath("LeachBasic");
        server = getModuleByPath("LeachBasic.base");
        WATCH(battery);
        WATCH(globalRound);
        WATCH(chFullPath);
        WATCH(Nb);

        receivedRoundCountVector.setName("Received Round Count Vector IN NODE ");
        receivedRoundCountVector.record(globalRound);

        // Her modülün bunu yapması mantıksız, ama ana modülün initialize kısmını bulduğumda oraya taşıyacağım, şimdilik dursun.
        P = mainModule->par("p").doubleValue();
        E_AMP = mainModule->par("eAmp").doubleValue();
        E_ELEC = mainModule->par("eElec").doubleValue();
        E_MP = mainModule->par("eMp").doubleValue();
        E_FS = mainModule->par("eFs").doubleValue();
        THRS_DIST = mainModule->par("thresholdDist").doubleValue();
        aggBattery = mainModule->par("aggregation");

        NUM_NODES = mainModule->par("numNodes");
        CLUSTER_DEFAULT_MEMBER = (int)(1 / P);
        NUM_CLUSTERS = (NUM_NODES / CLUSTER_DEFAULT_MEMBER);
        K = mainModule->par("k");
        range = mainModule->par("Range");
        algorithm = mainModule->par("algorithm");
        gDeg = 0;

        int xMax = mainModule->par("maxX");
        int yMax = mainModule->par("maxY");

        xPosition = POSXS[getNodeId()];//intuniform(0, xMax, 1);
        yPosition = POSYS[getNodeId()];//intuniform(0, yMax, 1);
        updateDisplayStringPositionAndTag(xPosition, yPosition);
        EV << xPosition << "," << yPosition << "\n"; // Yapay Zeka proje için

        int serX = mainModule->par("bazX");
        int serY = mainModule->par("bazY");
        if (algorithm != LEACH && algorithm != IMPLEACH) // leach ve impleach de Nb hesaplama işlemleri başlatılmayavak
        {
            EV << "BLAAAAAA\n";
            head2baseMsg * nmsg1 = new head2baseMsg("myXY");
            nmsg1->setSource(getNodeId());
            nmsg1->setPosX( xPosition );
            nmsg1->setPosY( yPosition );
            nmsg1->setKind(1); // Renk
            sendDirect(nmsg1, server, "in");
            reduceBattery(calculateDistance(xPosition, yPosition, serX, serY), SEND_REDUCE, INFO, 1);
        }

        if (xMax > yMax) networkRadius = xMax/2.0;
        else  networkRadius = yMax/2.0;
        dBS = calculateDistance(xPosition, yPosition, serX, serY);
        posXs = new int[NUM_NODES];
        posYs = new int[NUM_NODES];
        for(int i = 0; i < NUM_NODES; i++)
        {
            posXs[i] = 0;
            posYs[i] = 0;
        }

        bazX = mainModule->par("bazX");
        bazY = mainModule->par("bazY");
        chListesi = new int [NUM_NODES];
        bataryalar = new double [NUM_NODES];
        bazaUzakliklar = new double [NUM_NODES];
        dugumUzakliklari = new double * [NUM_NODES];
        secilenler = new bool [NUM_NODES];
        onceMiSonraMi = new int [NUM_NODES];
        olenler = new bool [NUM_NODES];
        zincirDizi = new int [NUM_NODES];
        for(int i = 0; i < NUM_NODES; i++)
        {
            olenler[i] = false;
            bataryalar[i] = fullBattery;
            bazaUzakliklar[i] = .0; // Öylesine
            dugumUzakliklari[i] = new double [NUM_NODES];
            dugumeUzakliklariHesapla(i);
        }

        benimSiraNo = getNodeId();
        bazaUzakliklariHesapla();
    }

    for(int i = 0; i < NUM_NODES; i++)
    {
        secilenler[i] = false; // Her cevrim düğümler baştan seçilmemiş olur
        onceMiSonraMi[i] = -1;
        chListesi[i] = -3;
        zincirDizi[i] = -2;
    }

    clusterMembers = new int [NUM_NODES]; // Maximum 100 ya da girilen sayı kadar düğümlük cluster için bellek ayırımı, düz hesap boş olanlar -1 olacak
    clusterMemberXs = new int [NUM_NODES];
    clusterMemberYs = new int [NUM_NODES];
    for(int i = 0; i < NUM_NODES; i++)
    {
        clusterMembers[i] = -1;
        clusterMemberXs[i] = -0.1;
        clusterMemberYs[i] = -0.1;
    }

    //zincirEvent = new cMessage("zM");
    nextRoundEvent = new cMessage("nrM");
    waitEvent = new cMessage("wM");
    headWaitEvent = new cMessage("hwM");
    broadcastEvent = new cMessage("bcM");
    littleBroadcastEvent = new cMessage("lbcM");
    node2headDataMessage = new cMessage("n2hdM");
    node2headChooseMessage = new cMessage("n2hcM");
    ch2baseMessage = new cMessage("h2bM");

    dieCount = 0;
    totalExpendedEnergy = 0;
    clusterMemberCount = 0;
    chDistance = MIN_DISTANCE;
    clusterHead = server;

    chSayisi = 0;
    gelenMesajSayisiIlk = 0; // Bunlara gerek kalmayacak galiba bu ikisine
    gelenMesajSayisiSon = 0; //
    liderGelenMesajSayisi = 1;
    liderNo = -1;
    liderim = false;
    baslangicim = false;
    sonum = false;

    beginHeadSelection();
    if(iAmHead)
    {
        cancelEvent(waitEvent);
        cancelEvent(headWaitEvent);
        scheduleAt(simTime()+1.0, headWaitEvent);
    }
    else
    {
        cancelEvent(waitEvent);
        cancelEvent(headWaitEvent);
        scheduleAt(simTime()+1.5, waitEvent);
    }
}

void Node::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(), "clr") == 0)
    {
        if (gate("cikisKapi")->isConnected()) gate("cikisKapi")->disconnect();
    }
    if(strcmp(msg->getName(), "clr1") == 0)
    {
        char * chr = new char[32];
        for(int i = 1; i <= baglanacakSayi; i++)
        {
            sprintf(chr, "giris%d", i);
            gate(chr)->disconnect();
            deleteGate(chr);
        }
    }
    if(strcmp(msg->getName(), "info") == 0)
    {
        reduceBattery(dBS*123456, RECEIVE_REDUCE, INFO); // uzaklık önemli değil zaten 123456....
        head2baseMsg * msg1 = check_and_cast<head2baseMsg *>(msg);
        std::string datastr = msg1->getMes();
        std::istringstream iss0(datastr);
        std::string token__;
        std::string token;

        EV << datastr << "\n";

        int i = 0;
        while(std::getline(iss0, token__, '_'))
        {
            std::istringstream iss1(token__);
            std::getline(iss1, token, ':');
            posXs[i] = atoi( token.c_str() );
            std::getline(iss1, token, ':');
            posYs[i] = atoi( token.c_str() );
            i++;
        }
        cMessage * msg2 = new cMessage("NbBroadcastEvent");
        simtime_t stt2 = 0.3;
        scheduleAt(stt2, msg2);
    }
    if(strcmp(msg->getName(), "NbBroadcastEvent") == 0)
    {
        double distanceToMe = .0;
        broadcastMsg * bcmsg;
        for(int i = 0; i < NUM_NODES; i++)
        {
            distanceToMe = calculateDistance(xPosition, yPosition, posXs[i], posYs[i]);
            if (distanceToMe < range)
            {
                bcmsg = new broadcastMsg("NbBc");
                bcmsg->setPosX(xPosition);
                bcmsg->setPosY(yPosition);
                bcmsg->setChFullPath(getFullPath().c_str());
                bcmsg->setKind(2); // BLUE DOT
                bcmsg->setSource(getNodeId());
                char *chr = new char[32];
                sprintf(chr, "LeachBasic.nd[%d]",i);
                sendDirect(bcmsg, getModuleByPath(chr), "in");
            }
        }
        reduceBattery(range, SEND_REDUCE, INFO);
    }
    if(strcmp(msg->getName(), "NbBc") == 0)
    {
        reduceBattery(range*123456, RECEIVE_REDUCE, INFO); // uzaklık önemli değil zaten 123456....
        broadcastMsg * msg1 = check_and_cast<broadcastMsg *>(msg);
        cModule * senderModule = msg1->getSenderModule();
        msg1->setName("Nb");
        msg1->setKind(3);
        sendDirect(msg, senderModule, "in");
        reduceBattery(calculateDistance(xPosition, yPosition, msg1->getPosX(), msg1->getPosY()), SEND_REDUCE, INFO);
    }
    if(strcmp(msg->getName(), "Nb") == 0)
    {
        broadcastMsg * msg1 = check_and_cast<broadcastMsg *>(msg);
        reduceBattery(calculateDistance(xPosition, yPosition, msg1->getPosX(), msg1->getPosY()), SEND_REDUCE, INFO);
        Nb++;
    }
    if (!iAmDead)
    {
    /**/if(strcmp(msg->getName(), "nrM") == 0)
        {
            cancelEvent(waitEvent);
            cancelEvent(headWaitEvent);
            EV << msg->getOwner()->getFullName() << " : beginNextRound(); \n";
            liderim = false;
            iAmHead = false;
            chDistance = MIN_DISTANCE;
            chooseSent = false;
            broadcastSent = false;
            littleBroadcastSent = false;
            broadcastArrived = false;
            noCh = false;
            ch2headMsgStr = "";
            chFullPath = new char[128];
            sprintf(chFullPath, "LeachBasic.base");
            clusterHead = getModuleByPath(chFullPath);
            if (iAmDead)
            {
                return;
            }

            beginNextRound(); // Round ilerlet, sonuncudaysa 1 e
            initialize();
        }

    /**/if(strcmp(msg->getName(), "agacInf") == 0)
        {
            reduceBattery(dBS*123456, RECEIVE_REDUCE, INFO);
            EV << getNodeId() << "   ------------------ AGAC BİLGİSİ ALINDI...................\n";

            base2chInfoMsg * b2cm1 = check_and_cast<base2chInfoMsg *>(msg);
            baglanacakSayi = b2cm1->getBaglanacakSayi();
            gonderilecekNo = b2cm1->getGonderilecekNo();
            gonderilecekKapiNo = b2cm1->getGonderilecekKapiNo();
            char * chr = new char[128];

            if (gonderilecekNo == -1453)
            {
                liderim = true;
                sprintf(chr, "LeachBasic.base", gonderilecekNo);
                clusterHead = getModuleByPath(chr);
                gate("cikisKapi")->connectTo(clusterHead->gate("girisKapi"));
                chDistance = dBS;
                cMessage * clearMsg = new cMessage("clr");
                scheduleAt(simTime() + 0.95, clearMsg); //3.05 de silinsin
                clearMsg = new cMessage("clr1");
                scheduleAt(simTime() + 1.0, clearMsg); //3.1 de silinsin
            }
            else
            {
                sprintf(chr, "LeachBasic.nd[%d]", gonderilecekNo);
                clusterHead = getModuleByPath(chr);
                sprintf(chr, "giris%d", gonderilecekKapiNo);
                clusterHead->addGate(chr, cGate::INPUT);
                gate("cikisKapi")->connectTo(clusterHead->gate(chr));
                int chNo = zincirDizi[getSiraId()+1];
                chDistance = calculateDistance(xPosition, yPosition, POSXS[chNo], POSYS[chNo]);
                cMessage * clearMsg = new cMessage("clr");
                scheduleAt(simTime() + 0.95, clearMsg); //3.05 de silinsin
                clearMsg = new cMessage("clr1");
                scheduleAt(simTime() + 1.0, clearMsg); //3.1 de silinsin
            }

        }

    /**/if(strcmp(msg->getName(), "bcM") == 0) // Modül: standart düğüm, büyük reklam mesajı buraya düşer.
        {
            broadcastMsg * msg1 = check_and_cast<broadcastMsg *>(msg);
            broadcastArrived = true;
            int broX = msg1->getPosX(); // X kardeş, Y kardeş :)
            int broY = msg1->getPosY();
            int source_no = msg1->getSource();
            double dist = calculateDistance(broX, broY, xPosition, yPosition);
            std::string str_f_path = msg1->getChFullPath();
            std::string str_color = msg1->getColor();
            if(iAmHead)
            {
                EV << "(" << xPosition << ", " << yPosition << ") " << "nd["<< getNodeId()<<"].head = " << str_f_path << "=====already===ch=======\n";
                chDistance = MIN_DISTANCE; // ch kendisi
                sprintf(chFullPath, getFullPath().c_str());
                clusterHead = getModuleByPath(chFullPath);
                return;
            }
            else if (chDistance == MIN_DISTANCE) // ch değilim, ama uzaklık -1, demek ki hiç set edilmemiş.
            {
                EV << "(" << xPosition << ", " << yPosition << ") nd["<< getNodeId()<<"] --> dist: " << dist << ", chDistance: " << chDistance << "=====first=bc=======\n";
                chDistance = dist;
                sprintf(chFullPath, str_f_path.c_str());

                EV << "(" << xPosition << ", " << yPosition << ") " << "nd["<< getNodeId()<<"].head = " << str_f_path << "\n";
                getDisplayString().setTagArg("i", 1, str_color.c_str());
                clusterHead = getModuleByPath(chFullPath);
                return;
            }
            if (dist < chDistance) // yeni broadcast'çi eskisinden daha yakınsa
            {
                sprintf(chFullPath, str_f_path.c_str());
                EV << "nd["<< getNodeId()<<"] --> dist: " << dist << ", chDistance: " << chDistance << "====new=ch=better========\n";
                EV << "nd["<< getNodeId()<<"].head = " << str_f_path << "\n";
                chDistance = dist;
                clusterHead = getModuleByPath(chFullPath);
                getDisplayString().setTagArg("i", 1, str_color.c_str());
                return;
            }
            reduceBattery(chDistance, RECEIVE_REDUCE, INFO);
            if (iAmDead)return;
        }
    /**/if(strcmp(msg->getName(), "lbcM") == 0) // Modül: standart düğüm, küme içi yayın mesajı
        {
            // Şimdilik boş, önemsiz
            reduceBattery(chDistance, RECEIVE_REDUCE);
            if (iAmDead)return;
        }
    /**/if (strcmp(msg->getName(), "n2hdM") == 0) // Modül: ch, broadcast'tan sonra gelen mesajlar buraya düşer.
        {
            node2headMsg *nmsg = check_and_cast<node2headMsg *>(msg);
            reduceBattery( calculateDistance(xPosition, yPosition, nmsg->getPosX(), nmsg->getPosY()), RECEIVE_REDUCE);
            if (iAmDead) return;
            if (nmsg->getDie() == true) dieCount++;
            totalExpendedEnergy += nmsg->getExpendedEnergy();
            char *c = new char[64];
            sprintf(c, "%d:", nmsg->getPosX());
            ch2headMsgStr.append(c);
            sprintf(c, "%d:", nmsg->getPosY());
            ch2headMsgStr.append(c);
            sprintf(c, "%f:", nmsg->getExpendedEnergy());
            ch2headMsgStr.append(c);
            sprintf(c, "%d", nmsg->getDie());
            ch2headMsgStr.append(c);
            ch2headMsgStr.append("_"); // ilgili düğümün bilgisi stringe eklenince bitirmek için '_' koydum, sonra base'de split edeceğim.
        }

    /**/if (strcmp(msg->getName(), "n2hcM") == 0) // Modül: ch, kümeiçi broadcast'tan sonra gelen mesajlar buraya düşer.
        {
            node2headMsg *nmsg = check_and_cast<node2headMsg *>(msg);
            reduceBattery( calculateDistance(xPosition, yPosition, nmsg->getPosX(), nmsg->getPosY()), RECEIVE_REDUCE);
            if (iAmDead) return;
            clusterMemberCount++;
            clusterMembers[clusterMemberCount-1] = nmsg->getSource();
            char *c = new char[64];
            sprintf(c, "%d:", nmsg->getPosX());
        }
    /**/if (strcmp(msg->getName(), "wM") == 0) // Modül: standart düğüm
        {
            cancelEvent(waitEvent);
            cancelEvent(nextRoundEvent);
            if (chooseSent && broadcastArrived) // ch seçildi, data gönderilecek
            {
                node2headMsg *msg2 = new node2headMsg("n2hdM");
                msg2->setSource(getNodeId());
                msg2->setPosX( xPosition );
                msg2->setPosY( yPosition );
                msg2->setKind(0); // Kırmızı nokta
                msg2->setExpendedEnergy(reduceBattery(chDistance, SEND_REDUCE, DATA));
                msg2->setDie(iAmDead);
                EV << this->getNodeId() << " --> Waiting Complete, Trying to send message to: " << chFullPath << "\n";
                sendDirect(msg2, clusterHead, "in");
                scheduleAt(simTime()+1.5, nextRoundEvent); // 4 saniye sonra yeni roundun mesajı gönderilecek
                if (!iAmDead) // Mesaj gönderdikten sonra ölmeyeceksem
                {
                    getDisplayString().setTagArg("i", 0, "misc/node_s"); //ikon ata
                }
                else
                {
                    getDisplayString().setTagArg("i", 0, "old/x_cross");
                }
            }
            else if (!chooseSent && chDistance != MIN_DISTANCE) //ch seçildi ama choose mesajı gönderilmedi
            {
                node2headMsg *msg2 = new node2headMsg("n2hcM");
                msg2->setSource(getNodeId());
                msg2->setPosX( xPosition );
                msg2->setPosY( yPosition );
                msg2->setKind(0); // Kırmızı nokta
                msg2->setExpendedEnergy(reduceBattery(chDistance, SEND_REDUCE, INFO));
                msg2->setDie(iAmDead);
                EV << this->getNodeId() << " --> I choose you message to: " << chFullPath << "\n";
                clusterHead = getModuleByPath(chFullPath);
                sendDirect(msg2, clusterHead, "in");
                chooseSent = true;
                if (iAmDead) // Mesaj gönderdikten sonra ölmeyeceksem
                {
                    getDisplayString().setTagArg("i", 0, "old/x_cross");
                }
                scheduleAt(simTime() + 1.0, waitEvent);
            }
            else if (noCh) // broadcast gelmedi,
            {
                reduceBattery(dBS, SEND_REDUCE, DATA);
                head2baseMsg *msg2 = new head2baseMsg("h2bM(n2b)");
                sendDirect(msg2, server, "in"); // Sadece enerji düşecek, baz mesaja tepki vermeyecek
                scheduleAt(simTime()+1.5, nextRoundEvent);
                if (!iAmDead) // Mesaj gönderdikten sonra ölmeyeceksem
                {
                    getDisplayString().setTagArg("i", 0, "misc/node_s"); //ikon ata // 4 saniye sonra yeni roundun mesajı gönderilecek
                }
                else
                {
                    getDisplayString().setTagArg("i", 0, "old/x_cross");
                }
            }
            else if (!broadcastArrived)
            {
                noCh = true;
                cancelEvent(waitEvent);
                scheduleAt(simTime() + 1.0, waitEvent);
            }

        }
    /**/if (strcmp(msg->getName(), "hwM") == 0) // Modül: ch: broadcast yapar, mesaj gelmesini bekler, hepsi gelince baza yollar.
        {
            if (!broadcastSent)
            {
                cancelEvent(headWaitEvent);
                sprintf(chFullPath, getFullPath().c_str());
                clusterHead = getModuleByPath(chFullPath);
                chDistance = MIN_DISTANCE;
                char *chr = new char[16];
                broadcastMsg *bcmsg = new broadcastMsg();
                if(getNodeId() == 0) EV << xPosition << ", " << yPosition << "\n";
                for(int i = 0; i < NUM_NODES; i++)
                {
                    sprintf(chr, "LeachBasic.nd[%d]",i);
                    bcmsg = new broadcastMsg("bcM");
                    bcmsg->setPosX(xPosition);
                    bcmsg->setPosY(yPosition);
                    bcmsg->setChFullPath(getFullPath().c_str());
                    bcmsg->setColor(color);
                    bcmsg->setKind(2); // BLUE DOT
                    bcmsg->setSource(getNodeId());
                    broadcastEvent = check_and_cast<cMessage *> (bcmsg);
                    if(i != getNodeId()) sendDirect(broadcastEvent, getModuleByPath(chr), "in");
                }
                broadcastSent = true;
                reduceBattery(networkRadius*2.82, SEND_REDUCE, INFO);
                if (iAmDead) return;
                scheduleAt(simTime()+1.0, headWaitEvent); // 2 sn sonra düğümler geri mesaj dönmüş olacak
            }
            else if (littleBroadcastSent)// ch, broadcast'tan sonra düğümlerden mesajlar alındıktan sonra self mesaj ile buraya girer
            {
                cancelEvent(nextRoundEvent);
                if (clusterMemberCount > 1) reduceBattery(NO_DISTANCE, DATA_AGGREGATION_REDUCE, INFO, clusterMemberCount);

                if (baglanacakSayi == 0)
                {
                    if(gonderilecekNo == -1453)
                    {
                        head2baseMsg *msg2 = new head2baseMsg("h2bM");
                        send(msg2, "cikisKapi");
                        reduceBattery(dBS, SEND_REDUCE, DATA, clusterMemberCount);
                        // tamamı bu düğüme bağlı
                        EV << "TEK KÜME VARRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR\n";
                    }
                    else
                    {
                        EV << "BEN " << getNodeId() << " NOLU CH, chDistance: " << chDistance << ", clusterMemberCount: " << clusterMemberCount << " #####################\n";
                        reduceBattery( chDistance, SEND_REDUCE , DATA, clusterMemberCount); // Zincir oluşurken chDistance a gönderilecek düğüme uzaklık atanmıştı
                        ch2chMsg * chmsg1 = new ch2chMsg("ch2ch");
                        chmsg1->setMessageCount(clusterMemberCount);
                        chmsg1->setDistance(chDistance);
                        send(chmsg1, "cikisKapi");

                    }
                }
//                if (iAmDead) return;
//                EV << this->getNodeId() <<" --> Waiting Complete, SENDING ALL PACKETS!";
//                head2baseMsg *msg2 = new head2baseMsg("h2bM");
//                msg2->setSource(getId());
//                msg2->setPosX( xPosition );
//                msg2->setPosY( yPosition );
//                msg2->setMes(ch2headMsgStr.c_str());
//                msg2->setTotalExpendedEnergy(totalExpendedEnergy);
//                msg2->setDieCount(dieCount);
//                sendDirect(msg2, server, "in");
//                //char *cc = new char[7];
                char *ci = new char[32];
                if (!iAmDead)
                {
                    sprintf(ci, "misc/node_s");
                    //sprintf(cc, "#FFFFFF");
                    getDisplayString().setTagArg("i", 0, ci); //ikon ata
                    //getDisplayString().setTagArg("i", 1, cc); //renk ata
                }
                scheduleAt(simTime()+1.0, nextRoundEvent); // 1 saniye sonra yeni round
            }
            else // broadcastSent && !littleBroadcastSent
            {
                char *chr = new char[16];
                broadcastMsg *lbcmsg = new broadcastMsg();
                if(getNodeId() == 0) EV << xPosition << ", " << yPosition << "\n";

                double maxdist = .0, dis;
                for(int i = 0; i < clusterMemberCount; i++)
                {
                    dis = calculateDistance(xPosition, yPosition, clusterMemberXs[i], clusterMemberYs[i]);
                    if (dis > maxdist)
                    {
                        maxdist = dis;
                    }
                }

                for(int i = 0; i < clusterMemberCount; i++)
                {
                    sprintf(chr, "LeachBasic.nd[%d]",clusterMembers[i]);
                    lbcmsg = new broadcastMsg("lbcM");
                    lbcmsg->setKind(3); // white DOT
                    littleBroadcastEvent = check_and_cast<cMessage *> (lbcmsg);
                    sendDirect(littleBroadcastEvent, getModuleByPath(chr), "in");
                }
                littleBroadcastSent = true;
                reduceBattery(maxdist, SEND_REDUCE, INFO); // Maxdist, cluster içi maximum uzaklık
                if (iAmDead) return; // Baza yollamadan ölecekse ölsün yoksa yarıda kalmasın, siz demiştiniz hocam.
                clusterMemberCount++; // Kendisini de ekliyoruz
                ch2baseInfoMsg * ch2baseMes = new ch2baseInfoMsg("chInf");
                ch2baseMes->setKalanEnerji(battery);
                ch2baseMes->setKaynakNo(getNodeId());
                ch2baseMes->setKomsuSayisi(Nb);
                ch2baseMes->setKumeUyeSayisi(clusterMemberCount);
                ch2baseMes->setBazaMesafe(dBS);
                sendDirect(ch2baseMes, server, "in");
                /*
                reduceBattery(dBS, SEND_REDUCE, INFO); // Buna almıyorum deneme amaçlı
                */
                scheduleAt(simTime()+1.0, headWaitEvent); // 1 saniye sonra ch --> base
            }
        }
    /**/if(strcmp(msg->getName(), "ch2ch") == 0)
        {
            ch2chMsg * chmsg1 = check_and_cast<ch2chMsg *>(msg);
            double senderDistance = chmsg1->getDistance();
            double senderMesCount = chmsg1->getMessageCount();
            reduceBattery(senderDistance, RECEIVE_REDUCE, DATA, senderMesCount); // DATA ya da INFO olması önemli değil...

            if(liderim)
            {
                if (liderGelenMesajSayisi < baglanacakSayi)
                {
                    liderGelenMesajSayisi++;
                    delete chmsg1;
                    clusterMemberCount += senderMesCount; // gelen mesaj sayısını da kendi küme toplamına ekledik
                }
                else if (liderGelenMesajSayisi >= baglanacakSayi)
                {
                    liderGelenMesajSayisi++; // artık 2 belki işe yarar diye yine de artırdım
                    clusterMemberCount += senderMesCount;
                    delete chmsg1;
                    head2baseMsg *msg2 = new head2baseMsg("h2bM");
                    msg2->setSource(getNodeId());
                    EV << "BAZA YOLLANIYOOORRRR:  clusterMemberCount: " << clusterMemberCount << "\n";
                    sendDirect(msg2, server, "in");
                    reduceBattery(dBS, SEND_REDUCE, DATA, clusterMemberCount); // artık tamamı bu düğüme bağlı bir küme imiş gibi
                }
                return;
            }

            if (liderGelenMesajSayisi < baglanacakSayi)
            {
                liderGelenMesajSayisi++;
                delete chmsg1;
                clusterMemberCount += senderMesCount; // gelen mesaj sayısını da kendi küme toplamına ekledik
            }
            else if (liderGelenMesajSayisi >= baglanacakSayi)
            {
                int newMessageCount = clusterMemberCount + senderMesCount;
                chmsg1->setMessageCount(newMessageCount);
                chmsg1->setDistance(chDistance);
                EV << getNodeId() <<  "  --> CH YE YOLLANIYOOORRRR:  clusterMemberCount: " << clusterMemberCount << "\n";
                send(chmsg1, "cikisKapi");
                //if (newMessageCount > 1) reduceBattery(NO_DISTANCE, DATA_AGGREGATION_REDUCE, INFO, newMessageCount);
                reduceBattery(chDistance, SEND_REDUCE, DATA, 1);/*reduceBattery(chDistance, SEND_REDUCE, DATA, newMessageCount);reduceBattery(chDistance, SEND_REDUCE, DATA, newMessageCount);*/

            }
        }
    }
}

void Node::finish()
{
    receivedRoundCountVector.record(globalRound); // Program tasarımında hata yapmışım, 5 cluster, yaklaşık bunun beşte biri olacak.
}



