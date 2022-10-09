#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <cstdlib> /* 鈭?賊??賣 */
#include <ctime>
#include <typeinfo>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <random>
#include <thread>
#include <cstdlib>
#include <algorithm>
//#define K_capacity  5
//#define NONAIRUtility   0.5
//#define lambdaWeight 0.8
//#define personal_theta 0.2

int K_capacity = 0;
double NONAIRUtility = 0.5;
double lambdaWeight = 0.8;
double personal_theta = 0.2;
int limitUserSize = 0;

double write_tmp_total = 0.0;
double write_tmp_p = 0.0;
double write_tmp_s = 0.0;

#pragma warning(disable : 4996)

using namespace std;

/*    ----- 瘜冽? & 隞?齒鈭? ----- */
/*

{

    Personal Utility 蝭? 0 ~1

    ??頝?non air??

    頝ata set 閰西岫??social ??

                    PI      ??      SD
    1 憟?4526    3977      294      255
    12  54312   47724     3528     3060

    撠?watchUser ??watchEdges ?寞??私arameter?餅?酣utput撟曉?like        pandas?? .head()

    1. theta?芷personal撠??禿ead?????<???舀???  function???暺?憿? return隞?Ⅳ閬??乩?銝?
       雿????臬?憭抒??絲 so?府瘝?靽?
       -------> Fixed 撽?銝??湔?刻艘?ㄐ?芷??? ?曉???釘ector敺???
       -------> ??-1?斗?臬?ａ? ?其?filter隡潔???撠?暺tility(??閰脫迤撣????賭???瘥?敺之 ??? 雿ilter?舐鈭Ⅱ靽??犖銝??憟嫣??迭?犖 ?喃蝙撠?喟?隞?

    ?拍車utility 撖怠鞈?鋆∠??孵?


}

*/

/*    -----  ?芾???瑽?  -----   */

// 瘥蝙?刻?Set嚗??paper銝剔? S_u
struct User
{
    int capacity{K_capacity};
    set<int> connectUser;
};

// 瘥蝙?刻??餌??? <???蝙?刻D, utility>  (?嗅?雿輻??ID ?典?銝?AP?鄂ey?潘?甇斤Map鋆⊿?AP)
struct Edge
{
    //  <connectID, UTILITY>
    map<int, double> edges;
};

// SVC?曉?嗅??憭吟tility???????(?嗡?baseline銝璅?
struct Rho_selection
{
    int userU;
    int userV;
    double utility;
    bool single_sel; // true = single selection, false = double selection
    double perutility;
    double socialutility;
};

struct Rho_tri
{
    int userU;
    int userV;
    int userX;
    int updateNum;
    double max_rho;
    double utility;
    double perutility;
    double socialutility;
    bool addVtoU;
    bool addXtoU;
    bool addVtoX;
    bool addXtoV;
};

struct dirUtoV
{
    int userU;
    int userV;
};

struct sepUtility
{
    double totU;
    double perU;
    double socU;
};

struct TotalUtility
{
    sepUtility svc;
    sepUtility per;
    sepUtility soc;
    sepUtility rand;
    sepUtility maxD;
    sepUtility avg;
    sepUtility opt;
    int svc_air;
    int per_air;
    int soc_air;
    int rand_air;
    int maxd_air;
    int avg_air;
    int opt_air;

    int svc_nair;
    int per_nair;
    int soc_nair;
    int rand_nair;
    int maxd_nair;
    int avg_nair;
    int opt_nair;
};

struct exeTime
{
    std::chrono::duration<double> svc_elapsed_seconds;
    std::chrono::duration<double> per_elapsed_seconds;
    std::chrono::duration<double> soc_elapsed_seconds;
    std::chrono::duration<double> rand_elapsed_seconds;
    std::chrono::duration<double> maxD_elapsed_seconds;
    std::chrono::duration<double> avg_elapsed_seconds;
    std::chrono::duration<double> opt_elapsed_seconds;
};

/*    -----  霈????瑽?  -----   */

//  User Set
//< 雿輻?D, User蝯?(摰寥?,?蔭?sers) >
// Final Answer
map<int, User> S_star;
// Potential Set
map<int, User> A_theta;
// all user won't change
map<int, User> ALLUSER;

//  Edges
// key = 瘥蝙?刻?ID 嚗alue = Edge struct 嚗誨銵男ey???駁鈭隞蝙?刻?
// <雿輻??u , < 雿輻??u ????餌?暺?, 甈? >
// Personal
map<int, Edge> personalUtility; // for iteration
map<int, Edge> directEdges;     // get utility
map<int, Edge> tmp_edge_fornonair;
// Social
map<int, Edge> socialUtility;

// loop condition
// ?其??斗?蔭餈游??臬蝯?
// int totalCapacity; // { number of users * k }

map<int, Edge> tmp_personalUtility;
map<int, Edge> tmp_directEdges;
map<int, User> tmp_S_star;
map<int, Edge> tmp_socialUtility;
map<int, User> tmp_A_theta;
map<int, User> tmp_ALLUSER;

//蝮賣???
double totalUtility{0};
double totalPersonalutility{0};
double totalSocialutility{0};

double tmp_totalu;
double tmp_totalpu;
double tmp_totalsu;

int nair_total = 0;

struct triangle
{
    int userU;
    int userV;
    int userX;
};

vector<triangle> userTri;

map<int, int> edgeNum;

/*    -----  ????賣   -----   */

//霈???
void read_file(map<int, Edge> &pU, map<int, Edge> &sU, map<int, User> &users, map<int, Edge> &dirEdges, string fileName, bool initialUser);
//撖急??
void write_data(string dataset, double totalutility, int k_capa, double lambda, double theta, exeTime &timeoutcome, TotalUtility &outcome);
void convert_graph(map<int, Edge> &pU, map<int, User> &users);
//瑼Ｘ?函? user U ?臬? user V??蝵株ㄐ嚗??箸?敶梢 Rho selection ??蝞瘜?
bool checkUinV(map<int, User> &users, int userUID, int userVID);
//??璈 w(u,v,x) ??rho_s(u,v) ??憭???x ??user V ??蝵株ㄐ嚗?憭?user U ??user V ?賢???啁??嗡?雿輻??憭?隞?” user U ? user V 撠征瘞??閰梁?璈?頞?
int nonAir(map<int, User> &users, int userUID, int userVID);

void copyUser(map<int, User> &users, map<int, User> &allusers);

// ?湔?典??
// selectionWay  =  true 隞?”?桅嚗雿輻??user U ??connectUser ?1?隞?雿輻??(撠?user V ? S_u) totalCapacity ??銝
// selectionWay  =  false 隞?”?嚗雿輻??user U ??user V  ??connectUser 鈭?撠  (撠?user V ? S_v  ??撠?user U ? S_v )  totalCapacity ??鈭?
void updateUser(map<int, User> &ans, map<int, User> &users, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, int userUID, int userVID, bool single_sel);
// selectionWay  =  true 隞?”?桅嚗?撠蝙?刻?user U ?? user V ?? 敺?map< int, Edge> & personalUtility 鋆∪??銋?餈游??賢?check銝甈∴?銋??銴
// selectionWay  =  false 隞?”?嚗?雿輻??user U ?? user V ????user V ?? user U ??敺?map< int, Edge> & personalUtility 鋆∪??
//                                  甇文?撠?map< int, Edge> & socialUtility ??銋??(?暸??舀??personalUtility?餉???social???????閰脖??蔣?選?雿??臬??
void update_utility(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, User> &users, int userUID, int userVID, bool selectionWay);

// ???典??
//???ser?onnectUser?蔭?捆??
void watchUsers(map<int, User> &users, int head);
//???鈭誑????utility
void watchEdges(map<int, Edge> &edges, int head);

// ???蔭????蝞?
//  ( SVC 瞍?瘜?)
//   Rho ?豢??賣嚗??單?頛???user U , user V , utility, selection way)
Rho_selection compare_roho__utility(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans);
//   銝餉艘??賂?瘥活 iteration ?澆 Rho_selection ?駁???桅???訾蒂敺蝯?
void compare_max_utility(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser);

// (?嗡?baseline瞍?瘜?)
Rho_selection personal_max_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, int perFirst);
Rho_selection social_max_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, int perFirst);
Rho_selection random_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans);
Rho_selection max_degree_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, map<int, User> &ALLUSER);
Rho_selection optimal_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans);
void shopping_avg(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, map<int, User> &ALLUSER);

// ??文??嶔璇??ser ?翰?脰?
void calculateEdgeLessThanK(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser);

bool checkUinV(map<int, User> &ans, int userUID, int userVID)
{

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  ????0???箔?

    //? user V ??connectUser ??摮 v
    auto v = ans[userVID].connectUser;
    //?曄???user U ?臬??v 鋆⊿
    auto search = v.find(userUID);

    //?曉
    if (search != v.end())
    {
        // std::cout << "user(U) :"<< userUID << " in user(V) :" << userVID << "\n";
        return true;
    } //瘝??
    else
    {
        // std::cout << " Not in\n";
        return false;
    }
}

// Find how many non air user
// ?莧NS?脣
int randflag = 0;
int svcflag = 0;
int nonAir(map<int, User> &users, int userUID, int userVID)
{
    // NONAIR point
    //  ?桀?蝯?user V ??蔭??users set 嚗?瘥????x
    auto xs = users[userVID].connectUser;
    // ?桀?蝯?user U ??蔭??users set
    auto u = users[userUID].connectUser;

    int Air = 0;

    //?曉?抵漱?? (MAYBE ???湔?set?漱?撘?
    // *閮???x ??? user U*
    for (std::set<int>::iterator x = xs.begin(); x != xs.end(); x++)
    {
        // or
        // for (auto it = myset.begin(); it != myset.end(); it++) {
        // std::std::cout << *it << " ";
        if (*x != userUID)
        {
            auto search = u.find(*x);
            if (search != u.end())
            {
                // std::cout << " found\n";
                // Air++;
                if (tmp_edge_fornonair[userUID].edges[*x] > 0)
                {
                    nair_total++;
                }
                else if (svcflag == 1)
                {
                    if (directEdges[userUID].edges[*x] > 0)
                        nair_total++;
                }
            }
            else
            {
                // x銝u?et鋆⊿ 銝????x??蝞?
                map<int, double>::iterator it;
                it = tmp_edge_fornonair[userUID].edges.find(*x);
                if (it != tmp_edge_fornonair[userUID].edges.end())
                {
                    // if (directEdges[userUID].edges[*x] > 0)
                    //{
                    Air++;
                    //}
                }
                else if (randflag == 1)
                {
                    Air++;
                }

                //  std::cout << " not found\n";
            }
        }
    }

    // std::cout << "Number of NonAir :" << numOfNonAir << endl;

    return Air;
}

//?踵??瘜?nonair
//瘥ser?nswer set?靘?
//?瘥??for?餅炎?亙?拚?撠?蝯?
// 1 2 3 4 5
// 1,2 1,3 1,4 1,5
// 2,3 2,4 ,2,5 ...
// 銝??
// ?府??userU 1 ??2 3 4 5
// ??停?臬??1 2 1 3 1 4 1 5

int calculateNonAirPoint(map<int, User> &ans)
{
    map<int, User>::iterator itr;

    int nonairpoint = 0;
    nair_total = 0;
    for (itr = ans.begin(); itr != ans.end(); itr++)
    {
        for (auto v : itr->second.connectUser)
        {
            nonairpoint += nonAir(ans, itr->first, v);
        }
    }

    return nonairpoint;
}

//???摰??? so Map鋆∩?摰? ????
Rho_selection compare_roho__utility(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{

    // Map 憭? (?臭誑?孵末??暺?霈??)
    //  <雿輻??雿輻????????>
    map<int, Edge>::iterator itr;
    // <雿輻??????,????????
    // May ?批?
    map<int, double>::iterator currentEntry;

    //皞?????貊???
    int maxUserU = -1, maxUserV = -1;
    double maxEffectiveUtility = 0.0, maxRho = 0.0;
    bool sflag = false;

    double perUtility = 0.0, socUtility = 0.0;
    bool onlyper = true;

    // ???ser???餌????嗅?瘥?隞?personal??social??之嚗憭抒?嚗?敺??ser瘥??芰??憭?

    // iterate map<int, Edge>
    // 甇方艘??甈?iteration ?銝??user ( itr ) ?箔?嚗?啣??user ?賜???甈?
    for (itr = person.begin(); itr != person.end(); itr++)
    {
        // iterate Edge map<int,double>
        // 甇方艘??甈?iteration ????Ｚ艘???箔???user ( itr ) ??? ( itr ) ???? ( currentEntry ) ?箔????游?券???銝甈?
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {

            int userUID = itr->first;
            int userVID = currentEntry->first;
            // nonair?賊? (??頝?
            // int numberOfNonAir = nonAir(users, userUID, userVID);
            double single_effectiveUtility = 0.0, double_effectiveUtility = 0.0, rho_double = 0.0;

            //閮? Rho_single ?蝔格?瘜?
            //??user U ?其???user V ??connectUser 鋆⊿
            //?湔?舀dirEdges?餃??湔 so?府閬dirEdges?潭?撠?
            if (checkUinV(ans, userUID, userVID) == true)
            {
                // ?砍? : (1 ??? 繚 (??(?,?)) + ?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
                // single_effectiveUtility = (1 - lambdaWeight) * (*currentEntry).second + lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]);
                single_effectiveUtility = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID] + lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
                onlyper = false;
            }
            else
            {
                // ?砍? : (1 ??? 繚??(?,?)
                // single_effectiveUtility = (1 - lambdaWeight) * (*currentEntry).second;
                single_effectiveUtility = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
                onlyper = true;
            }

            //閮? Rho_double ??瘜?
            //(???摰??? so Map鋆∩?摰? ????)
            //?????(?,?) ???撌脩???鈭?????賢??嚗?賡??芸楛?桅? find v???user鋆⊿<<<<<
            // ?砍? : (1???繚(??(?,?)+??(?,?))+??繚(??(?,?)+??(?,?)) * 1 / 2
            // double_effectiveUtility = (1 - lambdaWeight) * ((*currentEntry).second + dirEdges[userVID].edges[userUID]) + lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]);
            auto checkVstillalive = users.find(userVID);
            if (checkVstillalive != users.end())
            {
                double_effectiveUtility = (1 - lambdaWeight) * (dirEdges[userUID].edges[userVID] + dirEdges[userVID].edges[userUID]) + lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
                // rho_double ????utility閬2嚗??箄??像???臬葆蝯虫??犖?憓????剁??憭? rho_single???頛??血?銝撟?
                rho_double = double_effectiveUtility / 2;
            }

            //瘥? rho_single 憭折???rho_double 憭?
            if (single_effectiveUtility > rho_double)
            {
                // single 憭?
                //瘥??嗅???utility ?臭??舫憚??tility鋆⊥?憭抒?嚗?店???閮???靘?
                if (single_effectiveUtility > maxRho)
                {
                    maxRho = single_effectiveUtility;
                    maxEffectiveUtility = single_effectiveUtility;
                    maxUserU = userUID;
                    maxUserV = userVID;
                    sflag = true;

                    if (onlyper == true)
                    {
                        perUtility = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
                        socUtility = 0;
                    }
                    else
                    {
                        perUtility = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
                        socUtility = lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
                    }
                }
            }
            else
            {
                // double 憭?
                if (rho_double > maxRho)
                {
                    maxRho = rho_double;
                    maxEffectiveUtility = double_effectiveUtility;
                    maxUserU = userUID;
                    maxUserV = userVID;
                    sflag = false;

                    perUtility = (1 - lambdaWeight) * (dirEdges[userUID].edges[userVID] + dirEdges[userVID].edges[userUID]);
                    socUtility = lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
                }
            }
        }
    }

    // std::std::cout << "\nTotal Max : \n";
    // std::std::cout << "UserUID : " << maxUserU << " UserVID : " << maxUserV << " Single_Selection : " << sflag << endl;
    // std::std::cout << "Max Rho = " << maxRho << endl;
    // std::std::cout << "Max Effective Utility = " << maxEffectiveUtility << endl;

    // std::std::cout << "PerU = " << perUtility + socUtility << " SocU = " << socUtility << endl;
    // if (maxEffectiveUtility - (perUtility + socUtility) != 0)
    // {
    //     std::std::cout << "PerU = " << perUtility << " SocU = " << socUtility << endl;
    //     std::std::cout << " +  = " << (perUtility + socUtility);
    //     std::std::cout << " -  = " << maxEffectiveUtility - (perUtility + socUtility);
    //     // std::this_thread::sleep_for(std::chrono::seconds(3));
    // }

    Rho_selection selection;
    selection.userU = maxUserU;
    selection.userV = maxUserV;
    selection.utility = maxEffectiveUtility;
    selection.single_sel = sflag;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

void finduserTri(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{
    // ?曉 vector <triangle> userTri 鋆⊿;

    map<int, Edge>::iterator itr;
    map<int, double>::iterator currentEntry;
    map<int, double>::iterator currentEntry2;
    cout << "Find userTri" << endl;
    for (itr = person.begin(); itr != person.end(); itr++)
    {
        // iterate Edge map<int,double>
        //甇方艘??甈?iteration ????Ｚ艘???箔???user ( itr ) ??? ( itr ) ???? ( currentEntry ) ?箔????游?券???銝甈?
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {

            set<int> track;
            for (currentEntry2 = itr->second.edges.begin(); currentEntry2 != itr->second.edges.end(); ++currentEntry2)
            {
                int userUID = itr->first, userVID = currentEntry->first, userXID = currentEntry2->first;
                if (userVID == userXID)
                {
                    continue;
                }

                //?芣?銝????
                auto checkcomb = track.find(userVID + userXID);
                if (checkcomb != track.end())
                    continue;
                //
                track.insert(userVID + userXID);

                auto vu = person[userVID].edges.find(userUID);
                if (vu == person[userVID].edges.end())
                {
                    continue;
                }

                auto vx = person[userVID].edges.find(userXID);
                if (vx == person[userVID].edges.end())
                {
                    continue;
                }

                auto xv = person[userXID].edges.find(userVID);
                if (xv == person[userXID].edges.end())
                {
                    continue;
                }

                auto xu = person[userXID].edges.find(userUID);
                if (xu == person[userXID].edges.end())
                {
                    continue;
                }

                triangle tri;
                tri.userU = userUID;
                tri.userV = userVID;
                tri.userX = userXID;
                userTri.push_back(tri);
            }
        }
    }

    cout << "userTri size : " << userTri.size() << endl;
}

Rho_tri triselect(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{
    map<int, Edge>::iterator itr;
    map<int, double>::iterator currentEntry;
    map<int, double>::iterator currentEntry2;
    int maxUID = -1, maxVID = -1, maxXID = -1;
    double maxEffectiveutility = 0, maxRho = 0;
    double localpersonalutility = 0, localsocialutility = 0;
    Rho_tri tmp;
    for (itr = person.begin(); itr != person.end(); itr++)
    {
        // iterate Edge map<int,double>
        //甇方艘??甈?iteration ????Ｚ艘???箔???user ( itr ) ??? ( itr ) ???? ( currentEntry ) ?箔????游?券???銝甈?
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {

            set<int> track;
            for (currentEntry2 = itr->second.edges.begin(); currentEntry2 != itr->second.edges.end(); ++currentEntry2)
            {
                int userUID = itr->first, userVID = currentEntry->first, userXID = currentEntry2->first;
                if (userVID == userXID)
                {
                    continue;
                }

                //?芣?銝????
                auto checkcomb = track.find(userVID + userXID);
                if (checkcomb != track.end())
                    continue;
                //
                track.insert(userVID + userXID);

                double trieffectiveutility = 0;
                int number_of_division = 0;

                //??vx鈭?social????c??? ??omega??
                auto checkVinX = ans[userXID].connectUser.find(userVID);
                // X ????V
                if (checkVinX != ans[userXID].connectUser.end())
                {
                    tmp.addVtoX = false;
                    //撠銝??
                    number_of_division += 1;
                    //??V銋?? X
                    auto checkXinV = directEdges[userVID].edges.find(userXID);
                    if (checkXinV != directEdges[userVID].edges.end())
                    {
                        number_of_division += 1;
                        tmp.addXtoV = false;
                    }
                    else
                    {
                        // V 瘝???X
                        tmp.addXtoV = true;
                        trieffectiveutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID]) + lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
                        // localpersonalutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID]);
                        // localsocialutility += lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
                    }
                }
                else // X 瘝???V
                {

                    tmp.addVtoX = true;
                    //??V ??????X
                    auto checkXinV = directEdges[userVID].edges.find(userXID);
                    if (checkXinV != directEdges[userVID].edges.end())
                    {
                        number_of_division += 1;
                        tmp.addXtoV = false;
                    }
                    else
                    {
                        // V 瘝???X
                        tmp.addXtoV = true;
                        trieffectiveutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID] + dirEdges[userXID].edges[userVID]) + lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
                        // localpersonalutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID] + dirEdges[userXID].edges[userVID]);
                    }
                }

                //??U ???歇蝬???V
                auto checkVinU = ans[userUID].connectUser.find(userVID);
                // U ????V personal? 銝??
                if (checkVinU != ans[userUID].connectUser.end())
                {
                    tmp.addVtoU = false;
                    number_of_division += 1;
                }
                else
                {
                    tmp.addVtoU = true;
                    trieffectiveutility += (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
                }
                //??U ???歇蝬???X
                auto checkXinU = ans[userUID].connectUser.find(userXID);
                // U ????X personal 銝??
                if (checkXinU != ans[userUID].connectUser.end())
                {
                    tmp.addXtoU = false;
                    number_of_division += 1;
                }
                else
                {
                    tmp.addXtoU = true;
                    trieffectiveutility += (1 - lambdaWeight) * dirEdges[userUID].edges[userXID];
                }

                //撌脩??踹?NONAIR
                if (number_of_division >= 4)
                {
                    continue;
                }

                // ??mega
                trieffectiveutility += 1;
                trieffectiveutility = trieffectiveutility / (4 - number_of_division);
                if (trieffectiveutility > maxRho)
                {
                    maxEffectiveutility = trieffectiveutility * (4 - number_of_division);
                    maxRho = trieffectiveutility;
                    maxUID = userUID;
                    maxVID = userVID;
                    maxXID = userXID;
                }
            }
        }
    }

    tmp.userU = maxUID;
    tmp.userV = maxVID;
    tmp.userX = maxXID;
    tmp.utility = maxEffectiveutility;
    tmp.perutility = maxEffectiveutility * (1 - lambdaWeight);
    tmp.socialutility = maxEffectiveutility * (lambdaWeight);

    return tmp;
}

Rho_tri triselect2(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{
    int maxUID = -1, maxVID = -1, maxXID = -1;
    double maxEffectiveutility = 0, maxRho = 0;
    double localpersonalutility = 0, localsocialutility = 0;
    Rho_tri tmp;
    // 敺?vector <triangle> userTri 鋆⊿??
    vector<triangle>::iterator itr;
    vector<triangle> new_userTri;
    cout << "\nuserTri size : " << userTri.size() << endl;
    for (itr = userTri.begin(); itr != userTri.end(); ++itr)
    {
        int userUID = itr->userU, userVID = itr->userV, userXID = itr->userX;

        if (ans[userUID].connectUser.size() >= K_capacity)
        {
            continue;
        }
        else
        {
            triangle tri;
            tri.userU = userUID;
            tri.userV = userVID;
            tri.userX = userXID;
            new_userTri.push_back(tri);
        }

        double trieffectiveutility = 0;
        int number_of_division = 0;

        //??vx鈭?social????c??? ??omega??
        auto checkVinX = ans[userXID].connectUser.find(userVID);
        // X ????V
        if (checkVinX != ans[userXID].connectUser.end())
        {
            tmp.addVtoX = false;
            //撠銝??
            number_of_division += 1;
            //??V銋?? X
            auto checkXinV = tmp_personalUtility[userVID].edges.find(userXID);
            if (checkXinV != tmp_personalUtility[userVID].edges.end())
            {
                number_of_division += 1;
                tmp.addXtoV = false;
            }
            else
            {
                // V 瘝???X
                tmp.addXtoV = true;
                trieffectiveutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID]) + lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
                // localpersonalutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID]);
                // localsocialutility += lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
            }
        }
        else // X 瘝???V
        {

            tmp.addVtoX = true;
            //??V ??????X
            auto checkXinV = tmp_personalUtility[userVID].edges.find(userXID);
            if (checkXinV != tmp_personalUtility[userVID].edges.end())
            {
                number_of_division += 1;
                tmp.addXtoV = false;
            }
            else
            {
                // V 瘝???X
                tmp.addXtoV = true;
                trieffectiveutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID] + dirEdges[userXID].edges[userVID]) + lambdaWeight * (social[userVID].edges[userXID] + social[userXID].edges[userVID]) * 1 / 2;
                // localpersonalutility += (1 - lambdaWeight) * (dirEdges[userVID].edges[userXID] + dirEdges[userXID].edges[userVID]);
            }
        }

        //??U ???歇蝬???V
        auto checkVinU = ans[userUID].connectUser.find(userVID);
        // U ????V personal? 銝??
        if (checkVinU != ans[userUID].connectUser.end())
        {
            tmp.addVtoU = false;
            number_of_division += 1;
        }
        else
        {
            tmp.addVtoU = true;
            trieffectiveutility += (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
        }
        //??U ???歇蝬???X
        auto checkXinU = ans[userUID].connectUser.find(userXID);
        // U ????X personal 銝??
        if (checkXinU != ans[userUID].connectUser.end())
        {
            tmp.addXtoU = false;
            number_of_division += 1;
        }
        else
        {
            tmp.addXtoU = true;
            trieffectiveutility += (1 - lambdaWeight) * dirEdges[userUID].edges[userXID];
        }

        //撌脩??踹?NONAIR
        if (number_of_division >= 4)
        {
            continue;
        }

        // ??mega
        trieffectiveutility += 1;
        trieffectiveutility = trieffectiveutility / (4 - number_of_division);
        if (trieffectiveutility > maxRho)
        {
            maxEffectiveutility = trieffectiveutility * (4 - number_of_division);
            maxRho = trieffectiveutility;
            maxUID = userUID;
            maxVID = userVID;
            maxXID = userXID;
        }
    }

    tmp.userU = maxUID;
    tmp.userV = maxVID;
    tmp.userX = maxXID;
    tmp.utility = maxEffectiveutility;
    tmp.perutility = maxEffectiveutility * (1 - lambdaWeight);
    tmp.socialutility = maxEffectiveutility * (lambdaWeight);

    // cout << "TMPDELETE SIZE : " << tmpdelete_userU.size() << endl;

    // for (auto cur = tmpdelete_userU.rbegin(); cur != tmpdelete_userU.rend(); cur++)
    // {
    //     userTri.erase(userTri.begin() + *cur);
    // }

    userTri = new_userTri;

    return tmp;
}

Rho_selection social_max_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, int perFirst)
{

    // Map 憭? (?臭誑?孵末??暺?霈??)
    //  <雿輻??雿輻????????>
    map<int, Edge>::iterator itr;
    // <雿輻??????,????????
    // May ?批?
    map<int, double>::iterator currentEntry;

    //皞?????貊???
    int maxUserU = -1, maxUserV = -1;
    double maxSocialutility = 0.0, effectiveUtility = 0.0;
    bool sflag = true; //??賣?犖憭抒? ?隞仿?臬??

    double perUtility = 0.0, socUtility = 0.0;

    for (itr = person.begin(); itr != person.end(); itr++)
    {
        // iterate Edge map<int,double>
        // 甇方艘??甈?iteration ????Ｚ艘???箔???user ( itr ) ??? ( itr ) ???? ( currentEntry ) ?箔????游?券???銝甈?
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;

            /*if (userUID == 0 && userVID == 3) {
                std::cout << "\n\nwtf , person : " << person[userUID].edges[userVID] << " , dir " << directEdges[userUID].edges[userVID] << endl;
            }*/

            if (social[userUID].edges[userVID] > maxSocialutility)
            {
                maxUserU = userUID;
                maxUserV = userVID;
                maxSocialutility = social[userUID].edges[userVID];
            }
        }
    }

    if (checkUinV(ans, maxUserU, maxUserV) == true)
    {
        // ?砍? : (1 ??? 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
        effectiveUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV] + lambdaWeight * (social[maxUserU].edges[maxUserV] + social[maxUserV].edges[maxUserU]) * 1 / 2;
        perUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        socUtility = lambdaWeight * (social[maxUserU].edges[maxUserV] + social[maxUserV].edges[maxUserU]) * 1 / 2;
    }
    else
    {
        // ?砍? : (1 ??? 繚??(?,?)
        // single_effectiveUtility = (1 - lambdaWeight) * (*currentEntry).second;
        effectiveUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        perUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        socUtility = 0;
    }

    // std::std::cout << "\nTotal Max : \n";
    // std::std::cout << "UserUID : " << maxUserU << " UserVID : " << maxUserV << " Single_Selection : " << sflag << endl;
    // std::std::cout << "Max Social Utility = " << maxSocialutility << endl;
    // std::std::cout << "Effective Utility = " << effectiveUtility << endl;

    Rho_selection selection;
    selection.userU = maxUserU;
    selection.userV = maxUserV;
    selection.utility = effectiveUtility;
    selection.single_sel = sflag;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

Rho_selection personal_max_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, int perFirst)
{
    map<int, Edge>::iterator itr;
    map<int, double>::iterator currentEntry;

    int maxUserU = -1, maxUserV = -1;
    double maxPersonalutility = 0.0, effectiveUtility = 0.0;
    bool sflag = true;
    double perUtility = 0.0, socUtility = 0.0;

    for (itr = person.begin(); itr != person.end(); itr++)
    {
        // iterate Edge map<int,double>
        // 甇方艘??甈?iteration ????Ｚ艘???箔???user ( itr ) ??? ( itr ) ???? ( currentEntry ) ?箔????游?券???銝甈?
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;

            /*if (userUID == 0 && userVID == 3) {
                std::cout << "\n\nwtf , person : " << person[userUID].edges[userVID] << " , dir " << directEdges[userUID].edges[userVID] << endl;
            }*/

            if (directEdges[userUID].edges[userVID] > maxPersonalutility)
            {
                maxUserU = userUID;
                maxUserV = userVID;
                maxPersonalutility = directEdges[userUID].edges[userVID];
            }
        }
    }

    if (checkUinV(ans, maxUserU, maxUserV) == true)
    {
        // ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
        effectiveUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV] + lambdaWeight * (social[maxUserU].edges[maxUserV] + social[maxUserV].edges[maxUserU]) * 1 / 2;
        perUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        socUtility = lambdaWeight * (social[maxUserU].edges[maxUserV] + social[maxUserV].edges[maxUserU]) * 1 / 2;
    }
    else
    {
        // ?砍? : (1 ??? 繚??(?,?)
        effectiveUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        perUtility = (1 - lambdaWeight) * directEdges[maxUserU].edges[maxUserV];
        socUtility = 0;
    }

    std::cout << "\nTotal Max : \n";
    std::cout << "UserUID : " << maxUserU << " UserVID : " << maxUserV << " Single_Selection : " << sflag << endl;
    std::cout << "Max Personal Utility = " << maxPersonalutility << endl;
    std::cout << "Effective Utility = " << effectiveUtility << endl;

    Rho_selection selection;
    selection.userU = maxUserU;
    selection.userV = maxUserV;
    selection.utility = effectiveUtility;
    selection.single_sel = sflag;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

//?啁?鈭 ?其噶??銝??瑽?
Rho_selection random_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, map<int, User> &ALLUSER)
{
    map<int, User>::iterator itu;
    map<int, User>::iterator findv;
    itu = users.begin();

    //皞?????貊???
    int userV = itu->first;
    double effectiveUtility = 0.0;
    bool sflag = true; //??賣?犖憭抒? ?隞仿?臬??
    double perUtility = 0.0, socUtility = 0.0;

    std::random_device rd;
    /* 鈭?Ｙ???*/
    std::default_random_engine generator(rd());
    /* 鈭????撣?*/
    std::uniform_real_distribution<> unif(0, ALLUSER.size());
    /* ?Ｙ?鈭 */
    bool breakflag = false;
    while (userV == itu->first)
    {

        if (breakflag)
        {
            break;
        }
        int vidx = 0, vend = unif(generator);
        for (findv = ALLUSER.begin(); findv != ALLUSER.end(); findv++)
        {
            if (vidx >= vend)
            {
                userV = findv->first;

                // std::cout << "USERRRRRRRRRRRRRRRRRRRRRRRR : " << itu->first << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv : " << userV << endl;
                breakflag = true;
                break;
            }
            else
            {
                vidx++;
            }
        }
    }

    std::map<int, double>::iterator it;
    it = person[itu->first].edges.find(userV);
    if (it != person[itu->first].edges.end())
    {
        if (checkUinV(ans, itu->first, userV) == true)
        {
            // ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
            effectiveUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[userV] + lambdaWeight * (social[itu->first].edges[userV] + social[userV].edges[itu->first]) * 1 / 2;
            perUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[userV];
            socUtility = lambdaWeight * (social[itu->first].edges[userV] + social[userV].edges[itu->first]) * 1 / 2;
        }
        else
        {
            // ?砍? : (1 ??? 繚??(?,?)
            effectiveUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[userV];
            perUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[userV];
            socUtility = 0;
        }
    }
    else
    {
        effectiveUtility = 0.0;
        perUtility = 0;
        socUtility = 0;
    }

    // if (effectiveUtility != 0.0)
    // {
    //     std::std::cout << "\nRandom : \n";
    //     std::std::cout << "UserUID : " << itu->first << " UserVID : " << userV << " Single_Selection : " << sflag << endl;
    //     std::std::cout << "Have Effective Utility = " << effectiveUtility << endl;
    //     // std::this_thread::sleep_for(std::chrono::seconds(3));
    // }
    // else
    // {
    //     std::std::cout << "\nRandom : \n";
    //     std::std::cout << "UserUID : " << itu->first << " UserVID : " << userV << " Single_Selection : " << sflag << endl;
    //     std::std::cout << "Effective Utility = " << effectiveUtility << endl;
    // }

    Rho_selection selection;
    selection.userU = itu->first;
    selection.userV = userV;
    selection.utility = effectiveUtility;
    selection.single_sel = sflag;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

Rho_selection max_degree_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, map<int, User> &ALLUSER)
{
    /*銝???ser?餌?
    ?銝?ser
    ?餌?隞??????餅?隞????ize()??憭抒?*/

    map<int, User>::iterator itu;
    map<int, double>::iterator checkVedge;
    //??皛輻?user
    itu = users.begin();

    // U ????????V 隞?”?????圈??ser
    int maxEdgeV = person[itu->first].edges.begin()->first;
    // V????銝?閰脣??箸??芾?撠??隞交dirEdges
    // int maxEdgeNum = directEdges[maxEdgeV].edges.size();
    int maxEdgeNum = person[maxEdgeV].edges.size();
    // ?餌?隞??暑????暺??芸?憭?
    for (checkVedge = person[itu->first].edges.begin(); checkVedge != person[itu->first].edges.end(); checkVedge++)
    {
        if (person[checkVedge->first].edges.size() > maxEdgeNum)
        {
            maxEdgeV = checkVedge->first;
            maxEdgeNum = person[checkVedge->first].edges.size();
        }
    }

    double effectiveUtility = 0.0;
    bool sflag = true;
    double perUtility = 0.0, socUtility = 0.0;

    //雿?活? return ?
    if (checkUinV(ans, itu->first, maxEdgeV) == true)
    {
        // ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
        effectiveUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[maxEdgeV] + lambdaWeight * (social[itu->first].edges[maxEdgeV] + social[maxEdgeV].edges[itu->first]) * 1 / 2;
        perUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[maxEdgeV];
        socUtility = lambdaWeight * (social[itu->first].edges[maxEdgeV] + social[maxEdgeV].edges[itu->first]) * 1 / 2;
    }
    else
    {
        // ?砍? : (1 ??? 繚??(?,?)
        effectiveUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[maxEdgeV];
        perUtility = (1 - lambdaWeight) * directEdges[itu->first].edges[maxEdgeV];
        socUtility = 0;
    }

    // std::std::cout << "\nMax Edges: \n";
    // std::std::cout << "UserUID : " << itu->first << " UserVID : " << maxEdgeV << " Single_Selection : " << sflag << endl;
    // std::std::cout << "Max Edges Num = " << maxEdgeNum << endl;
    // std::std::cout << "Effective Utility = " << effectiveUtility << endl;

    Rho_selection selection;
    selection.userU = itu->first;
    selection.userV = maxEdgeV;
    selection.utility = effectiveUtility;
    selection.single_sel = sflag;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

void shopping_avg(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans, map<int, User> &ALLUSER)
{
    map<int, User>::iterator itr;

    std::random_device rd;
    /* 鈭?Ｙ???*/
    std::default_random_engine generator(rd());
    /* 鈭????撣?*/
    std::uniform_real_distribution<> unif(0, ALLUSER.size() - 1);
    std::uniform_real_distribution<float> genAlpha(0.0, 1);
    /* ?Ｙ?鈭 */

    // ? userX ??ID
    int sample_userX = unif(generator);
    // alpha ??
    float alpha = genAlpha(generator);
    float decay = 0.1;
    vector<int> tmpUseru;
    while (tmpUseru.size() == 0)
    {
        // std::cout << "alpha : " << alpha << "\n";
        for (itr = users.begin(); itr != users.end(); itr++)
        {
            if (itr->first != sample_userX && checkUinV(ans, sample_userX, itr->first) == false)
            {
                // ?Ⅱ隤?瘝?????????
                std::map<int, double>::iterator it;
                it = dirEdges[itr->first].edges.find(sample_userX);
                // ??
                if (it != dirEdges[itr->first].edges.end())
                {
                    // row 8
                    if (dirEdges[itr->first].edges[sample_userX] >= alpha && ans[itr->first].capacity > 0)
                    {
                        // std::cout << " 4 " << endl;
                        //  ???湔閮? utility 鈭?

                        tmpUseru.push_back(itr->first);
                    }
                }
            }
        }
        alpha -= decay;
        if (alpha <= 0)
        {
            tmpUseru.push_back(users.begin()->first);
            break;
        }
    }

    vector<int>::iterator idx;
    for (idx = tmpUseru.begin(); idx != tmpUseru.end(); idx++)
    {
        double effectiveUtility = 0.0;
        double perUtility = 0.0, socUtility = 0.0;

        if (checkUinV(ans, *idx, sample_userX) == true)
        {
            // std::cout << " 5 " << endl;
            //  ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
            effectiveUtility = (1 - lambdaWeight) * dirEdges[*idx].edges[sample_userX] + lambdaWeight * (social[*idx].edges[sample_userX] + social[sample_userX].edges[*idx]) * 1 / 2;
            perUtility = (1 - lambdaWeight) * dirEdges[*idx].edges[sample_userX];
            socUtility = lambdaWeight * (social[*idx].edges[sample_userX] + social[sample_userX].edges[*idx]) * 1 / 2;
        }
        else
        {
            // std::cout << " 6 " << endl;
            //  ?砍? : (1 ??? 繚??(?,?)
            effectiveUtility = (1 - lambdaWeight) * dirEdges[*idx].edges[sample_userX];
            perUtility = (1 - lambdaWeight) * dirEdges[*idx].edges[sample_userX];
            socUtility = 0;
        }

        totalUtility += effectiveUtility;
        totalPersonalutility += perUtility;
        totalSocialutility += socUtility;

        // std::std::cout << "\n AVG : \n";
        // std::std::cout << " userUID : " << *idx << " Sample X ID : " << sample_userX << endl;
        // std::std::cout << " person utility : " << dirEdges[*idx].edges[sample_userX] << " alpha : " << alpha << endl;
        // std::std::cout << " Effective Utility = " << effectiveUtility << endl;

        updateUser(ans, users, person, social, dirEdges, *idx, sample_userX, true);
    }
}

Rho_selection optimal_selection(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{
    // Map 憭? (?臭誑?孵末??暺?霈??)
    //  <雿輻??雿輻????????>
    int userUID = -1;
    map<int, User>::iterator itr = users.begin();
    for (itr = users.begin(); itr != users.end(); itr++)
    {
        if (itr->first != -1)
        {
            userUID = itr->first;
            break;
        }
    }

    // <雿輻??????,????????
    // May ?批?
    map<int, double>::iterator currentEntry;

    //皞?????貊???
    int maxUserU = -1, maxUserV = -1;
    double maxEffectiveUtility = 0.0;
    bool sflag = false;

    double perUtility = 0.0, socUtility = 0.0;

    for (currentEntry = dirEdges[userUID].edges.begin(); currentEntry != dirEdges[userUID].edges.end(); currentEntry++)
    {
        int userVID = currentEntry->first;
        double thisrun = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID] + lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
        if (thisrun > maxEffectiveUtility)
        {
            cout << "thisrun " << thisrun << endl;
            maxEffectiveUtility = thisrun;
            maxUserV = userVID;
            perUtility = (1 - lambdaWeight) * dirEdges[userUID].edges[userVID];
            socUtility = lambdaWeight * (social[userUID].edges[userVID] + social[userVID].edges[userUID]) * 1 / 2;
        }
    }

    std::cout << "\nTotal Max : \n";
    std::cout << "UserUID : " << userUID << " UserVID : " << maxUserV << " Single_Selection : " << sflag << endl;
    std::cout << "Max Effective Utility = " << maxEffectiveUtility << endl;

    Rho_selection selection;
    selection.userU = userUID;
    selection.userV = maxUserV;
    selection.utility = maxEffectiveUtility;
    selection.single_sel = true;
    selection.perutility = perUtility;
    selection.socialutility = socUtility;

    return selection;
}

// pass personalUtility and socialUtility to find_max_utility than compare the return pair
void compare_max_utility(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser)
{

    //銝?脰? iteration ?游摰寥?甇賊
    if (way == "SVC")
    {
        int tmp_users_size = users.size();
        Rho_tri tri;
        bool first = true;
        bool dotri = false;
        while (users.size() > 0)
        {

            //敺?嗅?瘥?????
            Rho_selection sel = compare_roho__utility(person, social, dirEdges, users, ans);

            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }

            // if (first == true || dotri == true)
            // {
            //     cout << "\n triselect 2 " << endl;
            //     tri = triselect2(person, social, dirEdges, users, ans);
            //     first = false;
            //     dotri = false;
            // }
            // tmp_users_size = users.size();
            // if (tri.utility > 0)
            // {
            //     cout << "sel.utility : " << sel.utility << "  tri.utility : " << tri.utility << endl;
            // }
            // if (sel.utility > tri.utility)
            // {
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;
            // }
            // else
            // {
            //     std::cout << "\nTotal Max : \n";
            //     std::cout << "UserUID : " << tri.userU << " UserVID : " << tri.userV << " UserXID : " << tri.userX << endl;
            //     std::cout << "Max Effective Utility = " << tri.utility << endl;
            //     if (tri.addVtoU)
            //     {
            //         updateUser(ans, users, person, social, dirEdges, tri.userU, tri.userV, true);
            //     }
            //     if (tri.addXtoU)
            //     {
            //         updateUser(ans, users, person, social, dirEdges, tri.userU, tri.userX, true);
            //     }
            //     if (tri.addXtoV)
            //     {
            //         updateUser(ans, users, person, social, dirEdges, tri.userV, tri.userX, true);
            //     }
            //     if (tri.addVtoX)
            //     {
            //         updateUser(ans, users, person, social, dirEdges, tri.userX, tri.userV, true);
            //     }
            //     totalUtility = totalUtility + tri.utility;
            //     totalPersonalutility += tri.perutility;
            //     totalSocialutility += tri.socialutility;

            //     tri.utility = 0;
            // }
            // cout << "tmp_users_size : " << tmp_users_size << endl;
            std::cout << "user.size() = " << users.size() << endl;
            // if (tmp_users_size != users.size())
            // {
            //     dotri = true;
            // }
        }
    }
    else if (way == "Personal")
    {
        while (users.size() > 0)
        {
            Rho_selection sel = personal_max_selection(person, social, dirEdges, users, ans, 0);
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;
            std::cout << "user.size() = " << users.size() << endl;
        }
    }
    else if (way == "Social")
    {
        while (users.size() > 0)
        {
            Rho_selection sel = social_max_selection(person, social, dirEdges, users, ans, 1);
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;
            std::cout << "user.size() = " << users.size() << endl;
        }
    }
    else if (way == "Random")
    {
        while (users.size() > 0)
        {
            Rho_selection sel = random_selection(person, social, dirEdges, users, ans, alluser);
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;
            std::cout << "user.size() = " << users.size() << endl;
        }
    }
    else if (way == "Degree")
    {
        while (users.size() > 0)
        {
            Rho_selection sel = max_degree_selection(person, social, dirEdges, users, ans, alluser);
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;
            std::cout << "user.size() = " << users.size() << endl;
        }
    }
    else if (way == "Avg")
    {
        while (users.size() > 0)
        {
            shopping_avg(person, social, dirEdges, users, ans, alluser);
            std::cout << "user.size() = " << users.size() << endl;
            // std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else if (way == "opt")
    {
        while (users.size() > 0)
        {
            //敺?嗅?瘥?????
            Rho_selection sel = optimal_selection(person, social, dirEdges, users, ans);

            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            //?寞?蝯??餅??user ??connectUser嚗蒂銝皜?蝮箱apacity
            updateUser(ans, users, person, social, dirEdges, sel.userU, sel.userV, sel.single_sel);
            //?寞?蝯??餅?圈??tility
            // update_utility(ans, person, social, users, sel.userU, sel.userV, sel.single_sel);
            //撠??tility?蜇

            // watchUsers(users, 10);
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;

            std::cout << "user.size() = " << users.size() << endl;
        }
    }
    else
    {
        std::cout << " ----------------------------------------------    Error    --------------------------------------------------------------\n";
    }
}
void allocateLastCapacity(map<int, User> &ans, map<int, User> &users, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, int userUID, int userVID)
{
    while (ans[userUID].capacity > 0)
    {

        map<int, Edge>::iterator itu;
        itu = dirEdges.begin();
        //皞?????貊???
        int userV = itu->first;

        std::random_device rd;
        /* 鈭?Ｙ???*/
        std::default_random_engine generator(rd());
        /* 鈭????撣?*/
        std::uniform_real_distribution<> unif(0, dirEdges.size());
        // std::cout << "******** : " << dirEdges.size() << endl;
        /* ?Ｙ?鈭 */
        bool breakflag = false;
        while (userV == itu->first)
        {
            if (breakflag)
            {
                break;
            }
            int vidx = 0, vend = unif(generator);
            // std::cout << "userv:" << userV << "vend: " << itu->first << endl;
            // std::cout << "vend : " << vend << endl;
            for (itu = dirEdges.begin(); itu != dirEdges.end(); itu++)
            {
                if (vidx >= vend)
                {
                    userV = itu->first;
                    breakflag = true;
                    break;
                }
                else
                {
                    vidx++;
                }
            }
        }
        if (userUID != userV)
        {
            const bool not_in = ans[userUID].connectUser.find(userV) == ans[userUID].connectUser.end();
            if (not_in)
            {
                ans[userUID].connectUser.insert(userV);
                ans[userUID].capacity--;
                users[userUID].capacity--;
            }
        }
    }
}
// selection way: single -> 0 , double -> 1
void updateUser(map<int, User> &ans, map<int, User> &users, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, int userUID, int userVID, bool single_sel)
{
    if (single_sel == true)
    {
        // std::std::cout << "?桅\n" << endl;
        auto iter = users.find(userUID);
        // userU ?隞?”??摰寥? ?血?撌脩??蔭摰?
        if (iter != users.end())
        {
            users[userUID].capacity--;
            ans[userUID].connectUser.insert(userVID);
            ans[userUID].capacity--;
            //憒? capcaity瘝? 閬???ser personal?????質???(?敺ap鋆∠宏?? ?脫迫???賊?
            if (users[userUID].capacity <= 0)
            {
                //?蝙?刻???歹?銝?隞?
                // std::cout << "蝘駁雿輻?n";
                person.erase(userUID);
                users.erase(userUID);
            }
            else
            {
                // std::cout << "?芷?\n";
                person[userUID].edges.erase(userVID);
                //?湔 edge utility
                dirEdges[userUID].edges[userVID] = 0;
                // dirEdges[userVID].edges[userUID] += social[userUID].edges[userVID];
                // dirEdges[userVID].edges[userUID] += social[userVID].edges[userUID];
                // social[userUID].edges[userVID] = 0;
                // social[userVID].edges[userUID] = 0;

                if (person[userUID].edges.size() <= 0)
                {
                    // std::cout << "user U :" << userUID << "???詨?鈭?蝘駁雿輻?n";
                    allocateLastCapacity(ans, users, person, social, dirEdges, userUID, userVID);

                    users.erase(userUID);
                    person.erase(userUID);
                }
            }
            // watchUsers(ans, 10);
            // watchEdges(person, 10);
        }
    }
    else
    {
        //??賣userV ?? userU 嚗?隞亥?蝣箄? userU ?臬撌脩??蔭摰?鈭?
        // std::cout << "?\n" << endl;
        auto iter = users.find(userUID);
        // userU ?隞?”??摰寥? ?血?撌脩??蔭摰?
        // std::cout << "??user U = " << userUID << endl;
        if (iter != users.end())
        {
            users[userUID].capacity--;
            ans[userUID].connectUser.insert(userVID);
            ans[userUID].capacity--;
            //憒? capcaity瘝? 閬???ser personal?????質???(?敺ap鋆∠宏?? ?脫迫???賊?
            if (users[userUID].capacity <= 0)
            {
                // std::cout << "蝘駁雿輻?n";
                person.erase(userUID);
                users.erase(userUID);
            }
            else
            {
                // std::cout << "?芷?\n";
                person[userUID].edges.erase(userVID);

                //?湔 edge utility
                dirEdges[userUID].edges[userVID] = 0;
                social[userUID].edges[userVID] = 0;
                social[userVID].edges[userUID] = 0;

                if (person[userUID].edges.size() <= 0)
                {
                    allocateLastCapacity(ans, users, person, social, dirEdges, userUID, userVID);
                    // std::cout << "user U :" << userUID << "???詨?鈭?蝘駁雿輻?n";
                    users.erase(userUID);
                    person.erase(userUID);
                }
            }
            // watchUsers(ans, 10);
            // watchEdges(person, 10);
        }

        auto iter2 = users.find(userVID);
        // std::cout << "??user V = " << userVID << endl;
        if (iter2 != users.end())
        {
            users[userVID].capacity--;
            ans[userVID].connectUser.insert(userUID);
            ans[userVID].capacity--;
            if (users[userVID].capacity <= 0)
            {
                // std::cout << "蝘駁雿輻?n";
                person.erase(userVID);
                users.erase(userVID);
            }
            else
            {
                // std::cout << "?芷?\n";
                person[userVID].edges.erase(userUID);

                //?湔 edge utility
                dirEdges[userVID].edges[userUID] = 0;
                social[userUID].edges[userVID] = 0;
                social[userVID].edges[userUID] = 0;

                if (person[userVID].edges.size() <= 0)
                {
                    // std::cout << "user V :" << userVID << "???詨?鈭?蝘駁雿輻?n";
                    allocateLastCapacity(ans, users, person, social, dirEdges, userUID, userVID);
                    users.erase(userVID);
                    person.erase(userVID);
                }
            }
            // watchUsers(ans, 10);
            // watchEdges(person, 10);
        }
    }
}

void watchUsers(map<int, User> &users, int head)
{
    map<int, User>::iterator currentEntry;

    std::cout << "\n\n* * * * * * * * * * * *  User Map  * * * * * * * * * * * * " << endl;

    for (currentEntry = users.begin(); currentEntry != users.end(); ++currentEntry)
    {
        //   currentEntry->second  ??(*) 靘見
        if (head == 0)
        {
            break;
        }
        std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
        std::cout << "user ID : " << currentEntry->first << endl;
        std::cout << "capacity : " << currentEntry->second.capacity << endl;
        for (const auto &s : currentEntry->second.connectUser)
        {
            std::cout << s << " ";
        }
        std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
        std::cout << endl
                  << endl;
        head--;
    }
}

void writeUsers(map<int, User> &users, int head, string way)
{
    map<int, User>::iterator currentEntry;

    ofstream ofs;
    std::stringstream ss;
    string filename;

    ss << way;
    ss << "k" << K_capacity;
    ss << "l" << lambdaWeight;
    ss << "t" << personal_theta;
    ss << "_users.txt";
    filename = ss.str();
    ofs.open(filename);
    if (!ofs.is_open())
    {
        std::cout << "Failed to open file.\n";
    }
    else
    {
        ofs << "\n\n* * * * * * * * * * * *  User Map  * * * * * * * * * * * * "
            << "\n";
        for (currentEntry = users.begin(); currentEntry != users.end(); ++currentEntry)
        {
            //   currentEntry->second  ??(*) 靘見
            if (head == 0)
            {
                break;
            }
            ofs << "- - - - - - - - - - - - - - - - - - - - - - - - -"
                << "\n";
            ofs << "user ID : " << currentEntry->first << "\n";
            ofs << "capacity : " << currentEntry->second.capacity << "\n";
            for (const auto &s : currentEntry->second.connectUser)
            {
                ofs << s << " ";
            }
            ofs << "\n- - - - - - - - - - - - - - - - - - - - - - - - -"
                << "\n";
            ofs << "\n"
                << "\n";
            head--;
        }
        ofs.close();
    }
}

void watchEdges(map<int, Edge> &edges, int head)
{
    //?風
    /*std::cout << "------------------------------\n";
    std::cout << "Edge utility : \n";

    for (const auto& u : edges) {
        std::cout << "[user U id: " << u.first << "]\n";
        std::cout << "[" <<u.second.edges.size() << "]" << endl;
        for (const auto& e : u.second.edges) {
            std::cout << "user V id: " << e.first << ", utility: " << e.second << "\n";
        }
        std::cout << "\n";
    }*/

    std::cout << "\n\n* * * * * * * * * * * *  Edges Map  * * * * * * * * * * * * " << endl;

    for (const auto &u : edges)
    {
        if (head == 0)
        {
            break;
        }
        std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
        std::cout << "user U ID: " << u.first << endl;
        std::cout << "number of edges : " << u.second.edges.size() << endl;
        for (const auto &e : u.second.edges)
        {
            std::cout << " ---> user V ID : " << e.first << ", utility :  " << e.second << endl;
        }
        std::cout << endl;
        std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
        head--;
    }
}

void theta_filter(map<int, Edge> &person, map<int, Edge> &dirEdges, map<int, User> &users)
{

    map<int, Edge>::iterator itr;
    map<int, double>::iterator currentEntry;

    vector<dirUtoV> toDeleteEdges;
    int edgesize = 0;

    for (itr = person.begin(); itr != person.end(); itr++)
    {

        auto eitr = edgeNum.find(itr->second.edges.size());
        if (eitr != edgeNum.end())
        {
            //撌脫? +1
            edgeNum[itr->second.edges.size()] += 1;
        }
        else
        {
            //瘝????
            edgeNum[itr->second.edges.size()] = 1;
        }

        edgesize += itr->second.edges.size();
        for (currentEntry = itr->second.edges.begin(); currentEntry != itr->second.edges.end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;
            // if (itr->second.edges.size() > K_capacity)
            //{
            if (currentEntry->second < personal_theta)
            {
                // std::cout << "Filter find => userUID : " << userUID << " userVID : " << userVID << " utility = " << currentEntry->second << " < personal_theta \n";
                dirEdges[userUID].edges[userVID] = 0;
                dirUtoV de;
                de.userU = userUID;
                de.userV = userVID;
                toDeleteEdges.push_back(de);
            }
            //}
        }
    }
    std::cout << "Edge size : " << edgesize << endl;
    std::cout << "                           Start Delete                       " << endl;

    vector<dirUtoV>::iterator itr2;
    for (itr2 = toDeleteEdges.begin(); itr2 != toDeleteEdges.end(); itr2++)
    {
        // std::cout << "delete => userUID : " << itr2->userU << " userVID : " << itr2->userV << " utility = " << dirEdges[itr2->userU].edges[itr2->userV] << " < personal_theta \n";
        person[itr2->userU].edges.erase(itr2->userV);
        edgesize--;
    }

    std::cout << "Edge size : " << edgesize << endl;
}
int timikL = 0;
// read_file(personalUtility,socialUtility, '.csv', 0 or 1)
void read_file(map<int, Edge> &pU, map<int, Edge> &sU, map<int, User> &users, map<int, Edge> &dirEdges, string fileName, bool initialUser)
{
    // File pointer
    fstream file(fileName, ios::in);
    string line, word;
    bool head = true;
    int test = 0;

    std::cout << "read file :" << fileName << endl
              << endl;
    if (file.is_open())
    {
        while (getline(file, line))
        {

            if (limitUserSize != 0)
            {
                if (users.size() == 100000 * limitUserSize)
                {
                    cout << "hit!" << endl;
                    break;
                }
            }

            if (head == true)
            {
                // std::cout << line << endl;
                head = false;
                continue;
            }

            stringstream templine(line);
            // userU userV personalutility socialutility
            int userU, userV;
            double putility, sutility;

            // timik**********
            /*
            int index;
            getline(templine, word, ',');
            index = stod(word);*/

            getline(templine, word, ',');
            userU = stoi(word);
            getline(templine, word, ',');
            userV = stoi(word);
            getline(templine, word, ',');
            putility = stof(word);
            getline(templine, word, ',');
            sutility = stof(word);

            // cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc1
            // float cf = 0.34;
            // if (putility > cf)
            // {
            //     putility = (putility - cf);
            // }

            if (test < 6)
            {
                std::cout << "line : " << line << endl;
                string sas;
                templine >> sas;
                std::cout << "test : " << test << " user U : " << userU << " user V : " << userV << " putility : " << putility << " sutility : " << sutility << endl;
                test++;
            }
            //? map<int, Edge> 雿輻????
            auto iter = pU.count(userU);
            // userU 撌脣鋆⊿
            if (iter == true)
            {
                // ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc2
                // if (pU[userU].edges.size() <= 1500)
                // {
                pU[userU].edges[userV] = putility;
                dirEdges[userU].edges[userV] = putility;
                // }
            }
            else
            {
                //?????user U ???洵銝璇?
                // if (putility > personal_theta) {
                pU[userU].edges.insert(make_pair(userV, putility));
                dirEdges[userU].edges.insert(make_pair(userV, putility));
                //}
            }

            //? map<int, Edge> 雿輻????(social)
            auto iter3 = sU.find(userU);
            // userU 撌脣鋆⊿
            if (iter3 != sU.end())
            {
                sU[userU].edges[userV] = sutility;
            }
            else
            {
                //?????user U ???洵銝璇?
                Edge uvsu;
                uvsu.edges[userV] = sutility;
                sU[userU] = uvsu;
            }

            if (initialUser == true)
            {
                //? map<int, User> 雿輻??
                auto iter2 = users.find(userU);
                if (iter2 == users.end())
                {
                    User ucs;
                    ucs.capacity = K_capacity;
                    ucs.connectUser.clear();
                    users[userU] = ucs;
                }

                //??銝??券??湔?userV
            }
        }
    }
    else
    {
        std::cout << "Could not open the file\n";
    }

    file.close();
}

void write_data(string dataset, double totalutility, int k_capa, double lambda, double theta, exeTime &timeoutcome, TotalUtility &outcome)
{
    ofstream ofs;
    std::stringstream ss;

    /*auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    ss << std::put_time(std::localtime(&in_time_t), "%m-%d_%H_%M_%S");*/

    string filename;
    /*ss >> filename;
    filename += ".txt";*/
    // ss << "exceptsvc_";
    ss << "k" << K_capacity;
    ss << "l" << lambdaWeight;
    ss << "t" << personal_theta;
    ss << ".txt";
    filename = ss.str();
    ofs.open(filename);
    if (!ofs.is_open())
    {
        std::cout << "Failed to open file.\n";
    }
    else
    {
        ofs << "Dataset : " << dataset << "\n";
        ofs << "K : " << k_capa << "\nlambda : " << lambda << "\ntheta : " << theta << "\n";
        ofs << "\ntmptotal: " << write_tmp_total << "\n";
        ofs << "Way : "
            << "SVC"
            << "\n";
        ofs << "Total utility : " << outcome.svc.totU << "\n";
        ofs << "personal : " << outcome.svc.perU << " social : " << outcome.svc.socU << "\n";
        ofs << "elapsed : " << timeoutcome.svc_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.svc_air << "\n";
        ofs << "nonair : " << outcome.svc_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "Personal Max"
            << "\n";
        ofs << "Total utility : " << outcome.per.totU << "\n";
        ofs << "personal : " << outcome.per.perU << " social : " << outcome.per.socU << "\n";
        ofs << "elapsed : " << timeoutcome.per_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.per_air << "\n";
        ofs << "nonair : " << outcome.per_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "Social Max"
            << "\n";
        ofs << "Total utility : " << outcome.soc.totU << "\n";
        ofs << "personal : " << outcome.soc.perU << " social : " << outcome.soc.socU << "\n";
        ofs << "elapsed : " << timeoutcome.soc_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.soc_air << "\n";
        ofs << "nonair : " << outcome.soc_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "Random"
            << "\n";
        ofs << "Total utility : " << outcome.rand.totU << "\n";
        ofs << "personal : " << outcome.rand.perU << " social : " << outcome.rand.socU << "\n";
        ofs << "elapsed : " << timeoutcome.rand_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.rand_air << "\n";
        ofs << "nonair : " << outcome.rand_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "MaxDegree"
            << "\n";
        ofs << "Total utility : " << outcome.maxD.totU << "\n";
        ofs << "personal : " << outcome.maxD.perU << " social : " << outcome.maxD.socU << "\n";
        ofs << "elapsed : " << timeoutcome.maxD_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.maxd_air << "\n";
        ofs << "nonair : " << outcome.maxd_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "AVG"
            << "\n";
        ofs << "Total utility : " << outcome.avg.totU << "\n";
        ofs << "personal : " << outcome.avg.perU << " social : " << outcome.avg.socU << "\n";
        ofs << "elapsed : " << timeoutcome.avg_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.avg_air << "\n";
        ofs << "nonair : " << outcome.avg_nair << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs << "Way : "
            << "opt"
            << "\n";
        ofs << "Total utility : " << outcome.opt.totU << "\n";
        ofs << "personal : " << outcome.opt.perU << " social : " << outcome.opt.socU << "\n";
        ofs << "elapsed : " << timeoutcome.opt_elapsed_seconds.count() << "\n";
        ofs << "air : " << outcome.opt_air << "\n";
        ofs << "nonair : " << outcome.opt_nair << "\n";

        ofs.close();
    }
}

void copyUser(map<int, User> &users, map<int, User> &allusers)
{
    allusers = users;
}

void clear_map()
{
    personalUtility.clear();
    socialUtility.clear();
    A_theta.clear();
    directEdges.clear();
    S_star.clear();
    totalUtility = 0.0;
    totalPersonalutility = 0.0;
    totalSocialutility = 0.0;
    tmp_totalpu = 0;
    // tmp_totalsu = 0;
}

void calculateEdgeLessThanK(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser)
{
    map<int, Edge>::iterator itr;
    map<int, double>::iterator cur;
    vector<dirUtoV> tmp_del;

    std::cout << "start User size : " << person.size() << " user size : " << users.size() << endl;

    for (itr = person.begin(); itr != person.end(); itr++)
    {
        if (itr->second.edges.size() <= K_capacity)
        {
            for (cur = itr->second.edges.begin(); cur != itr->second.edges.end(); cur++)
            {

                dirUtoV tmp;
                tmp.userU = itr->first;
                tmp.userV = cur->first;
            }
        }
    }

    vector<dirUtoV>::iterator toDel;
    std::cout << "to delete edge size : " << tmp_del.size() << endl;

    for (toDel = tmp_del.begin(); toDel != tmp_del.end(); toDel++)
    {
        float perUtility = (1 - lambdaWeight) * dirEdges[toDel->userU].edges[toDel->userV];
        totalUtility += perUtility;
        totalPersonalutility += perUtility;
        updateUser(ans, users, person, social, dirEdges, toDel->userU, toDel->userV, true);
    }

    std::cout << "after delete edge <= k, person size : " << person.size() << "user size : " << users.size() << endl;
}

int countedgenum = 0;
int countuserzie = 0;

void fastcalculateEdgeLessThanK(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser)
{
    map<int, Edge>::iterator itr;
    map<int, double>::iterator cur;
    vector<int> tmp_del;

    std::cout << "start person size : " << person.size() << " user size : " << users.size() << endl;

    for (itr = person.begin(); itr != person.end(); itr++)
    {

        if (itr->second.edges.size() <= K_capacity)
        {
            tmp_del.push_back(itr->first);

            for (cur = itr->second.edges.begin(); cur != itr->second.edges.end(); cur++)
            {
                dirUtoV tmp;
                tmp.userU = itr->first;
                tmp.userV = cur->first;

                ans[tmp.userU].connectUser.insert(tmp.userV);
                ans[tmp.userU].capacity--;

                float perUtility = (1 - lambdaWeight) * dirEdges[tmp.userU].edges[tmp.userV];
                // totalUtility += perUtility;
                tmp_totalpu += perUtility;
            }
            // allocateLastCapacity(ans, users, person, social, dirEdges, itr->first, 0);
            int idx = 0;
            while (ans[itr->first].connectUser.size() < K_capacity)
            {
                ans[itr->first].connectUser.insert(idx);
                idx++;
            }
        }
    }

    vector<int>::iterator toDel;
    std::cout << "to delete user size : " << tmp_del.size() << endl;

    for (toDel = tmp_del.begin(); toDel != tmp_del.end(); toDel++)
    {
        // cout << "delete userU : " << *toDel << endl;
        person.erase(*toDel);
        users.erase(*toDel);
    }

    std::cout << "after delete edge <= k, person size : " << person.size() << " user size : " << users.size() << endl;
    tmp_edge_fornonair = person;
}

void BFOPTIMAL(map<int, User> &ans, map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, string way, map<int, User> &alluser)
{
    vector<int> c;
    vector<int> d;
    vector<int> f;

    c.push_back(2);
    c.push_back(4);
    c.push_back(2);
    c.push_back(6);
    c.push_back(4);
    c.push_back(6);

    d.push_back(1);
    d.push_back(3);
    d.push_back(1);
    d.push_back(5);
    d.push_back(3);
    d.push_back(5);
    d.push_back(1);
    d.push_back(6);
    d.push_back(3);
    d.push_back(6);
    d.push_back(5);
    d.push_back(6);

    f.push_back(3);
    f.push_back(5);
    f.push_back(3);
    f.push_back(4);
    f.push_back(4);
    f.push_back(5);

    double opt = 0;
    int idx = 0;
    for (int ci = 0; ci < 6; ci = ci + 2)
    {
        cout << "ci:" << c[ci] << endl;
        cout << "ci+1:" << c[ci + 1] << endl;
        /*updateUser(ans, users, person, social, dirEdges, 3, c[ci], true);
        updateUser(ans, users, person, social, dirEdges, 3, c[ci+1], true);*/
        ans[3].connectUser.insert(c[ci]);
        ans[3].connectUser.insert(c[ci + 1]);
        for (int di = 0; di < 12; di = di + 2)
        {
            ans[4].connectUser.insert(d[di]);
            ans[4].connectUser.insert(d[di + 1]);
            for (int fi = 0; fi < 6; fi = fi + 2)
            {
                idx++;
                ans[6].connectUser.insert(f[fi]);
                ans[6].connectUser.insert(f[fi + 1]);
                double thisrun = 0;

                for (auto it : ans[3].connectUser)
                {
                    cout << "[" << it << "]" << endl;
                    if (checkUinV(ans, 3, it) == true)
                    {
                        // std::cout << " 5 " << endl;
                        //  ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
                        thisrun += (1 - lambdaWeight) * dirEdges[3].edges[it] + lambdaWeight * (social[3].edges[it] + social[it].edges[3]) * 1 / 2;
                    }
                    else
                    {
                        // std::cout << " 6 " << endl;
                        //  ?砍? : (1 ??? 繚??(?,?)
                        thisrun += (1 - lambdaWeight) * dirEdges[3].edges[it];
                    }
                }

                for (auto it : ans[4].connectUser)
                {
                    if (checkUinV(ans, 4, it) == true)
                    {
                        // std::cout << " 5 " << endl;
                        //  ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
                        thisrun += (1 - lambdaWeight) * dirEdges[4].edges[it] + lambdaWeight * (social[4].edges[it] + social[it].edges[4]) * 1 / 2;
                    }
                    else
                    {
                        // std::cout << " 6 " << endl;
                        //  ?砍? : (1 ??? 繚??(?,?)
                        thisrun += (1 - lambdaWeight) * dirEdges[4].edges[it];
                    }
                }

                for (auto it : ans[6].connectUser)
                {
                    if (checkUinV(ans, 6, it) == true)
                    {
                        // std::cout << " 5 " << endl;
                        //  ?砍? : (1 ????) 繚 (??(?,?)) +?? 繚 (??(?,?) + ??(?, ?)) * 1 / 2
                        thisrun += (1 - lambdaWeight) * dirEdges[6].edges[it] + lambdaWeight * (social[6].edges[it] + social[it].edges[6]) * 1 / 2;
                    }
                    else
                    {
                        // std::cout << " 6 " << endl;
                        //  ?砍? : (1 ??? 繚??(?,?)
                        thisrun += (1 - lambdaWeight) * dirEdges[6].edges[it];
                    }
                }

                cout << "thisrun : " << thisrun << endl;
                if (thisrun > opt)
                {
                    opt = thisrun;
                    std::cout << "ci : " << ci << "di : " << di << "fi : " << fi << endl;
                }
                ans[6].connectUser.clear();
            }
            ans[4].connectUser.clear();
        }
        ans[3].connectUser.clear();
    }

    cout << "idx:" << idx << endl;
    std::cout << "totalu :" << totalUtility << endl;
    std::cout << "BF :  " << opt + totalUtility << endl;
}

void copyMap()
{
    personalUtility = tmp_personalUtility;
    directEdges = tmp_directEdges;
    S_star = tmp_S_star;
    socialUtility = tmp_socialUtility;
    A_theta = tmp_A_theta;
    ALLUSER = tmp_ALLUSER;
}

void calculatelessKSocialPoint(map<int, User> &ans, map<int, Edge> &social)
{
    map<int, User>::iterator itr;
    cout << "calculatesocial" << endl;

    for (itr = ans.begin(); itr != ans.end(); itr++)
    {
        for (auto v : itr->second.connectUser)
        {
            if (checkUinV(ans, v, itr->first))
            {
                tmp_totalsu += lambdaWeight * (social[itr->first].edges[v]) * 1 / 2;
            }
        }
    }
}

void addlesskutility(map<int, User> &ans, map<int, Edge> &social)
{
    totalUtility = tmp_totalpu + tmp_totalsu;
    totalPersonalutility = tmp_totalpu;
    totalSocialutility = tmp_totalsu;
    write_tmp_total = totalUtility;
    cout << "tmptotal: " << totalUtility << endl;
    cout << "tmpper: " << tmp_totalpu << " tmpsoc: " << tmp_totalsu << endl;
}

int main(int argc, char *argv[])
{
    std::cout << "We have " << argc << " arguments" << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "[" << i << "] " << argv[i] << std::endl;
    }
    //  0   1    2     3      4        5
    // a.out K Lambda theta  dataset  limitUserSize
    K_capacity = atoi(argv[1]);
    lambdaWeight = atof(argv[2]);
    personal_theta = atof(argv[3]);

    string dataset = argv[4];
    TotalUtility outcome;
    exeTime timeoutcome;

    limitUserSize = atoi(argv[5]);

    std::cout << "SVC: " << endl;
    clear_map();
    read_file(personalUtility, socialUtility, A_theta, directEdges, dataset, true);

    tmp_personalUtility = personalUtility;
    tmp_directEdges = directEdges;
    tmp_S_star = S_star;
    tmp_socialUtility = socialUtility;
    tmp_A_theta = A_theta;
    tmp_ALLUSER = ALLUSER;

    // theta_filter(personalUtility, directEdges, A_theta);
    // //銝??蔭
    // fastcalculateEdgeLessThanK(S_star, personalUtility, socialUtility, directEdges, A_theta, "SVC", ALLUSER);
    // //??蝵?
    // // calculateEdgeLessThanK(S_star, personalUtility, socialUtility, directEdges, A_theta, "SVC", ALLUSER);

    // addlesskutility(S_star, socialUtility);

    // finduserTri(personalUtility, socialUtility, directEdges, A_theta, S_star);

    // for (auto const &keyValue : edgeNum)
    // {
    //     // keyValue.first;  // Key
    //     // keyValue.second; // Value
    //     if (keyValue.first == 0)
    //     {
    //         continue;
    //     }
    //     countedgenum += keyValue.first * keyValue.second;
    //     countuserzie += keyValue.second;
    //     cout << "edge size : " << keyValue.first << " number : " << keyValue.second << endl;
    // }

    // cout << "countedgenum : " << countedgenum << " countuserzie : " << countuserzie << endl;

    std::cout << "Personal: " << endl;
    clear_map();
    copyMap();
    // theta_filter(personalUtility, directEdges, A_theta);
    // fastcalculateEdgeLessThanK(S_star, personalUtility, socialUtility, directEdges, A_theta, "SVC", ALLUSER);
    auto Personal_begin = std::chrono::high_resolution_clock::now();

    for (auto p = personalUtility.begin(); p != personalUtility.end(); ++p)
    {
        vector<int> ptmp;
        for (auto z = p->second.edges.begin(); z != p->second.edges.end(); ++z)
        {
            ptmp.push_back(z->first);
        }
        // cout << ptmp.size() << "\n";
        sort(ptmp.begin(), ptmp.end());
        int i = 0;
        for (auto j = ptmp.begin(); j != ptmp.end(); ++j)
        {
            if (i < K_capacity)
            {
                break;
            }
            // cout << p->first << "\n";
            A_theta[p->first].connectUser.insert(*j);
            ++i;
        }
    }

    // compare_max_utility(S_star, personalUtility, socialUtility, directEdges, A_theta, "Personal", ALLUSER);
    auto Personal_end = std::chrono::high_resolution_clock::now();
    timeoutcome.per_elapsed_seconds = Personal_end - Personal_begin;
    outcome.per.totU = totalUtility;
    outcome.per.perU = totalPersonalutility;
    outcome.per.socU = totalSocialutility;
    std::cout << "total utility : " << outcome.per.totU << endl;
    std::cout << "total p utility : " << totalPersonalutility << endl;
    std::cout << "total s utility : " << totalSocialutility << endl;

    std::cout << "Social: " << endl;
    clear_map();
    copyMap();
    // theta_filter(personalUtility, directEdges, A_theta);
    // fastcalculateEdgeLessThanK(S_star, personalUtility, socialUtility, directEdges, A_theta, "SVC", ALLUSER);

    // addlesskutility(S_star, socialUtility);
    auto Social_begin = std::chrono::high_resolution_clock::now();

    for (auto p = socialUtility.begin(); p != socialUtility.end(); ++p)
    {
        vector<int> ptmp;
        for (auto z = p->second.edges.begin(); z != p->second.edges.end(); ++z)
        {
            ptmp.push_back(personalUtility[z->first].edges.size());
        }
        sort(ptmp.begin(), ptmp.end());
        int i = 0;
        for (auto j = ptmp.begin(); j != ptmp.end(); ++j)
        {
            if (i < K_capacity)
            {
                break;
            }
            // cout << p->first << "\n";
            A_theta[p->first].connectUser.insert(*j);
            ++i;
        }
    }

    auto Social_end = std::chrono::high_resolution_clock::now();
    timeoutcome.soc_elapsed_seconds = Social_end - Social_begin;
    std::cout << "Time taken by program is : " << timeoutcome.soc_elapsed_seconds.count() << " sec" << endl;
    outcome.soc.totU = totalUtility;
    outcome.soc.perU = totalPersonalutility;
    outcome.soc.socU = totalSocialutility;
    std::cout << "total utility : " << outcome.soc.totU << endl;
    std::cout << "total p utility : " << totalPersonalutility << endl;
    std::cout << "total s utility : " << totalSocialutility << endl;

    write_data(dataset, totalUtility, K_capacity, lambdaWeight, personal_theta, timeoutcome, outcome);

    return 0;
}