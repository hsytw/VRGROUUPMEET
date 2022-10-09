#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <cstdlib>
#include <ctime>
#include <typeinfo>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <random>
#include <thread>
#include <cstdlib>
#include <time.h>
#include <algorithm>

using namespace std;

int K_capacity = 0;
double lambdaWeight = 0;
double personal_theta = 0;

#pragma warning(disable : 4996)

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

struct dirUtoV
{
    int userU;
    int userV;
};

struct User
{
    int capacity{K_capacity};
    set<int> connectUser;
};

struct Rho_selection
{
    int userU;
    int userV;
    double utility;
    bool single_sel; // true = single selection, false = double selection
    double perutility;
    double socialutility;
};

struct vuindex
{
    int useru;
    int userv;
    int idx;
};
struct swedge
{
    int useru;
    int userv;
    double value;
};

map<int, map<int, double>> personal_edges;
map<int, map<int, double>> social_edges;
map<int, User> live_users;
map<int, User> ans_users;

double totalUtility{0};
double totalPersonalutility{0};
double totalSocialutility{0};

// -------------- heap ---------------
vector<swedge> p_maxheap;
vector<swedge> s_maxheap;
map<int, map<int, int>> user_p_in_heap_idx;
map<int, map<int, int>> user_s_in_heap_idx;
map<int, map<int, bool>> check_s_edge_made;

int p_maxheap_size = -1;
int s_maxheap_size = -1;
int parent(int i, vector<swedge> &maxheap)
{

    return (i - 1) / 2;
}

// Function to return the index of the
// left child of the given node
int leftChild(int i, vector<swedge> &maxheap)
{

    return ((2 * i) + 1);
}

// Function to return the index of the
// right child of the given node
int rightChild(int i, vector<swedge> &maxheap)
{

    return ((2 * i) + 2);
}

// Function to get value of the current
// maximum element
swedge getMax(vector<swedge> &maxheap)
{

    return maxheap.at(0);
}

// Function to shift up the node in order
// to maintain the heap property
void shiftUp(bool sh, int useru, int userv, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx)
{
    while (i > 0 && maxheap[parent(i, maxheap)].value < maxheap[i].value)
    {

        //更新edge的 map id
        user_in_heap_idx[useru][userv] = parent(i, maxheap);
        user_in_heap_idx[maxheap[i].useru][maxheap[i].userv] = i;
        if (sh == true)
        {
            user_in_heap_idx[userv][useru] = parent(i, maxheap);
            user_in_heap_idx[maxheap[i].userv][maxheap[i].useru] = i;
        }

        // Swap parent and current node
        swap(maxheap[parent(i, maxheap)], maxheap[i]);

        // Update i to parent of i
        i = parent(i, maxheap);
    }
}

// Function to shift down the node in
// order to maintain the heap property
void shiftDown(bool sh, int useru, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    int maxIndex = i;

    // Left Child
    int l = leftChild(i, maxheap);

    if (l <= maxheap_size && maxheap[l].value > maxheap[maxIndex].value)
    {
        maxIndex = l;
    }

    // Right Child
    int r = rightChild(i, maxheap);

    if (r <= maxheap_size && maxheap[r].value > maxheap[maxIndex].value)
    {
        maxIndex = r;
    }

    // If i not same as maxIndex
    if (i != maxIndex)
    {
        //要先更新edge的 map id
        user_in_heap_idx[useru][maxheap[maxIndex].useru] = maxIndex;
        user_in_heap_idx[maxheap[maxIndex].useru][useru] = i;

        if (sh == true)
        {
            user_in_heap_idx[maxheap[maxIndex].useru][useru] = maxIndex;
            user_in_heap_idx[useru][maxheap[maxIndex].useru] = i;
        }

        // cout << "swap : "
        //      << "  maxheap[i] = " << maxheap[i].useru << "  maxheap[maxIndex] " << maxheap[maxIndex].useru << "\n";
        swap(maxheap[i], maxheap[maxIndex]);

        shiftDown(sh, useru, maxIndex, maxheap, user_in_heap_idx, maxheap_size);
    }
}

// Function to insert a new element
// in the Binary Heap
void insert(bool sh, swedge p, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    // std::cout << "insert : " << p.useru << ", " << p.userv << "\n";
    maxheap_size = maxheap_size + 1;

    // maxheap[size] = p;
    maxheap.push_back(p);
    user_in_heap_idx[p.useru][p.userv] = maxheap_size;
    // Shift Up to maintain heap property
    shiftUp(sh, p.useru, p.userv, maxheap_size, maxheap, user_in_heap_idx);
}

// Function to extract the element with
// maximum priority
swedge extractMax(bool sh, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{

    if (maxheap_size < 0)
    {
        swedge a;
        a.useru = -1;
        a.userv = -1;
        a.value = 0;
        return a;
    }

    swedge max_sw_edge = maxheap[0];
    double result = max_sw_edge.value;
    int useru = max_sw_edge.useru;
    int userv = max_sw_edge.userv;
    // Replace the value at the root
    // with the last leaf
    maxheap[0] = maxheap[maxheap_size];
    maxheap_size = maxheap_size - 1;
    user_in_heap_idx[useru][userv] = -1;
    if (sh == true)
    {
        user_in_heap_idx[userv][useru] = -1;
    }

    // Shift down the replaced element
    // to maintain the heap property
    // std::cout << "next maxheap[0].useru = " << maxheap[0].useru << "\n";

    shiftDown(sh, maxheap[0].useru, 0, maxheap, user_in_heap_idx, maxheap_size);
    return max_sw_edge;
}

void heap_remove(bool sh, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    maxheap[i].value = getMax(maxheap).value + 1;

    // cout << "heap_remove" << "\n";

    // Shift the node to the root
    // of the heap
    // shiftUp(int useru, int userv, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx)
    shiftUp(sh, maxheap[i].useru, maxheap[i].userv, i, maxheap, user_in_heap_idx);

    // Extract the node
    extractMax(sh, maxheap, user_in_heap_idx, maxheap_size);

    // cout << "after remove :" << maxheap_size << "\n";
}

// Function to change the priority
// of an element
void changePriority(bool sh, int useru, int userv, double utility, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    int i = user_in_heap_idx[useru][userv];
    double oldp = maxheap[i].value;
    //計算新的utility
    maxheap[i].value = utility;

    //應該只會跑下降
    if (maxheap[i].value > oldp)
    {
        shiftUp(sh, useru, userv, i, maxheap, user_in_heap_idx);
    }
    else
    {
        shiftDown(sh, useru, i, maxheap, user_in_heap_idx, maxheap_size);
    }
}

void read_file(string fileName);
void theta_filter();

void makeSWheap();

void read_file(string fileName)
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

            getline(templine, word, ',');
            userU = stoi(word);
            getline(templine, word, ',');
            userV = stoi(word);
            getline(templine, word, ',');
            putility = stof(word);
            getline(templine, word, ',');
            sutility = stof(word);

            if (test < 6)
            {
                std::cout << "line : " << line << endl;
                string sas;
                templine >> sas;
                std::cout << "test : " << test << " user U : " << userU << " user V : " << userV << " putility : " << putility << " sutility : " << sutility << endl;
                test++;
            }

            personal_edges[userU][userV] = putility;
            // personal_edges[userV][userU] = putility;

            social_edges[userU][userV] = sutility;
            // social_edges[userV][userU] = sutility;

            if (!live_users.count(userU))
            {
                User ucs;
                ucs.capacity = K_capacity;
                ucs.connectUser.clear();
                live_users[userU] = ucs;
            }
        }
    }
    else
    {
        std::cout << "Could not open the file\n";
    }

    file.close();
}
void write_data(string dataset, exeTime &timeoutcome, TotalUtility &outcome)
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
        ofs << "K : " << K_capacity << "\nlambda : " << lambdaWeight << "\ntheta : " << personal_theta << "\n";
        ofs << "Way : "
            << "SVC"
            << "\n";
        ofs << "Total utility : " << outcome.svc.totU << "\n";
        ofs << "personal : " << outcome.svc.perU << " social : " << outcome.svc.socU << "\n";
        ofs << "elapsed : " << timeoutcome.svc_elapsed_seconds.count() << "\n";
        ofs << "\n-------------------------------------------\n";
        ofs.close();
    }
}
void theta_filter()
{
    //計算capacity 接著在底下 filter裡面扣掉多的
    //     for (auto i = live_users.begin(); i != live_users.end(); ++i)
    //     {
    //         i->second.capacity = personal_edges[i->first].size() >= K_capacity ? K_capacity : personal_edges[i->first].size();
    //     }

    //     vector<dirUtoV> toDeleteEdges;
    //     int edgesize = 0;

    //     for (auto itr = personal_edges.begin(); itr != personal_edges.end(); itr++)
    //     {
    //         for (auto currentEntry = itr->second.begin(); currentEntry != itr->second.end(); ++currentEntry)
    //         {
    //             int userUID = itr->first;
    //             int userVID = currentEntry->first;
    //             if (currentEntry->second < personal_theta)
    //             {
    //                 personal_edges[userUID][userVID] = 0;
    //                 live_users[userUID].capacity--;
    //                 if (live_users[userUID].capacity == 0)
    //                 {
    //                     live_users.erase(userUID);
    //                 }
    //             }
    //         }
    //     }
    // }

    vector<dirUtoV> toDeleteEdges;
    int edgesize = 0;

    for (auto itr = personal_edges.begin(); itr != personal_edges.end(); itr++)
    {
        for (auto currentEntry = itr->second.begin(); currentEntry != itr->second.end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;
            if (currentEntry->second < personal_theta)
            {
                personal_edges[userUID][userVID] = 0;
                dirUtoV de;
                de.userU = userUID;
                de.userV = userVID;
                toDeleteEdges.push_back(de);
            }
        }
    }

    vector<dirUtoV>::iterator itr2;
    for (itr2 = toDeleteEdges.begin(); itr2 != toDeleteEdges.end(); itr2++)
    {
        personal_edges[itr2->userU].erase(itr2->userV);
    }

    // vector<int> less_k_kill;
    // for (auto i = live_users.begin(); i != live_users.end(); ++i)
    // {
    //     i->second.capacity = personal_edges[i->first].size() >= K_capacity ? K_capacity : personal_edges[i->first].size();
    // }
    ;
    vector<int> tmp_del;

    for (auto itr = personal_edges.begin(); itr != personal_edges.end(); itr++)
    {

        if (itr->second.size() <= K_capacity)
        {
            tmp_del.push_back(itr->first);

            for (auto cur = itr->second.begin(); cur != itr->second.end(); cur++)
            {
                dirUtoV tmp;
                tmp.userU = itr->first;
                tmp.userV = cur->first;

                live_users[tmp.userU].connectUser.insert(tmp.userV);
                live_users[tmp.userU].capacity--;
            }
        }
    }

    for (auto toDel = tmp_del.begin(); toDel != tmp_del.end(); toDel++)
    {
        personal_edges.erase(*toDel);
        live_users.erase(*toDel);
    }
}

void makeSWheap()
{
    bool onlyper = true;

    for (auto itr = live_users.begin(); itr != live_users.end(); itr++)
    {
        // std::cout << itr->first << " : c = " << itr->second.capacity << " , edgesize : " << personal_edges[itr->first].size() << "\n";
        for (auto currentEntry = personal_edges[itr->first].begin(); currentEntry != personal_edges[itr->first].end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;
            double single_effectiveUtility = 0.0, double_effectiveUtility = 0.0, rho_double = 0.0;

            // vector<swedge> p_maxheap;
            // vector<swedge> s_maxheap;
            // map<int, int> user_p_in_heap_idx;
            // map<int, int> user_s_in_heap_idx;
            // map<int, map<int,bool>> check_s_edge_made

            single_effectiveUtility = (1 - lambdaWeight) * personal_edges[userUID][userVID];
            swedge psw;
            psw.useru = userUID;
            psw.userv = userVID;
            psw.value = single_effectiveUtility;

            // std::cout << userUID << " , " << userVID << "\n";
            insert(false, psw, p_maxheap, user_p_in_heap_idx, p_maxheap_size);

            // 建立 social 的 heap
            double_effectiveUtility = (1 - lambdaWeight) * (personal_edges[userUID][userVID] + personal_edges[userVID][userUID]) + lambdaWeight * (social_edges[userUID][userVID] + social_edges[userVID][userUID]) * 1 / 2;
            // didn't make

            if (!check_s_edge_made[userUID].count(userVID))
            {
                swedge ssw;
                ssw.useru = userUID;
                ssw.userv = userVID;
                ssw.value = double_effectiveUtility;

                insert(false, ssw, s_maxheap, user_s_in_heap_idx, s_maxheap_size);

                check_s_edge_made[userUID][userVID] = true;
                check_s_edge_made[userVID][userUID] = true;
            }
        }
    }

    vector<vuindex> vuidxtmp;
    for (auto z = user_s_in_heap_idx.begin(); z != user_s_in_heap_idx.end(); ++z)
    {
        for (auto zz = z->second.begin(); zz != z->second.end(); ++zz)
        {
            vuindex tete;
            tete.useru = zz->first;
            tete.userv = z->first;
            tete.idx = zz->second;
            vuidxtmp.push_back(tete);
        }
    }
    // user_s_in_heap_idx 反向建造回來
    for (auto i = vuidxtmp.begin(); i != vuidxtmp.end(); ++i)
    {
        int useru = i->useru;
        int userv = i->userv;
        int idx = i->idx;
        user_s_in_heap_idx[useru][userv] = idx;
    }
}

Rho_selection compare_roho__utility2()
{
    // users = A_theta

    // -------- DS ------------
    // vector<swedge> p_maxheap;
    // vector<swedge> s_maxheap;
    // map<int, int> user_p_in_heap_idx;
    // map<int, int> user_s_in_heap_idx;
    // map<int, map<int,bool>> check_s_edge_made
    // int p_maxheap_size = -1;
    // int s_maxheap_size = -1;

    // ------- Func -----------
    // changePriority(int useru, double minus_utility, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
    // extractMax(vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
    // insert(swedge p, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
    // swedge getMax(vector<swedge> &maxheap)

    // std::cout << "to p max heap\n";
    swedge per_edge = getMax(p_maxheap);
    // std::cout << "to s max heap\n";
    swedge soc_edge = getMax(s_maxheap);

    double rho_per = per_edge.value;
    double rho_soc = soc_edge.value / 2;
    double maxrho = 0;
    double effective_utility = 0;
    Rho_selection selection;
    int useru = -1, userv = -1;
    bool single = false;

    if (rho_soc >= rho_per)
    {
        soc_edge = extractMax(true, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
        useru = soc_edge.useru;
        userv = soc_edge.userv;
        maxrho = soc_edge.value / 2;
        effective_utility = soc_edge.value;
        if (live_users.count(useru) && live_users.count(userv))
        {
            live_users[useru].capacity--;
            live_users[userv].capacity--;
            if (live_users[useru].capacity <= 0)
            {
                live_users.erase(useru);
                // for (auto i = user_p_in_heap_idx[useru].begin(); i != user_p_in_heap_idx[useru].end(); ++i)
                // {
                //     heap_remove(i->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                // }
            }

            if (live_users[userv].capacity <= 0)
            {
                live_users.erase(userv);
            }

            changePriority(true, userv, useru, 0, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            changePriority(true, useru, userv, 0, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
        }
        else
        {
            //有人不能選了 無效選擇
            effective_utility = 0;
        }
    }
    else
    {
        single = true;
        per_edge = extractMax(false, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
        useru = per_edge.useru;
        userv = per_edge.userv;
        maxrho = per_edge.value;
        effective_utility = per_edge.value;
        if (live_users.count(useru))
        {
            live_users[useru].capacity--;
            if (live_users[useru].capacity <= 0)
            {
                live_users.erase(useru);
                // for (auto i = user_p_in_heap_idx[useru].begin(); i != user_p_in_heap_idx[useru].end(); ++i)
                // {
                //     if (i->second != -1)
                //     {
                //         heap_remove(i->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                //     }
                // }
            }
        }
        else
        {
            effective_utility = 0;
        }

        // 1. update v->u per
        int v_idx = user_p_in_heap_idx[userv][useru];
        double nu = (1 - lambdaWeight) * personal_edges[userv][useru] + lambdaWeight * (social_edges[useru][userv] + social_edges[userv][useru]) * 1 / 2;
        changePriority(false, userv, useru, nu, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
        // 2. kill double sel
        // 待做
        changePriority(false, useru, userv, 0, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
    }

    selection.userU = useru;
    selection.userV = userv;
    selection.utility = effective_utility;
    selection.single_sel = single;
    selection.perutility = effective_utility * (1 - lambdaWeight);
    selection.socialutility = effective_utility * lambdaWeight;

    std::cout << "\nTotal Max : \n";
    std::cout << "UserUID : " << selection.userU << " UserVID : " << selection.userV << " Single_Selection : " << selection.single_sel << endl;
    std::cout << "Max Rho = " << maxrho << endl;
    std::cout << "Max Effective Utility = " << effective_utility << endl;
    std::cout << "p_maxheap_size : " << p_maxheap_size << ", "
              << "s_maxheap_size : " << s_maxheap_size << "\n";
    return selection;
}

void compare_max_utility(string way)
{

    if (way == "SVC")
    {
        while (live_users.size() > 0)
        {
            Rho_selection sel = compare_roho__utility2();
            std::cout << "\n\n\n user.size() = " << live_users.size() << endl;
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;

            std::cout << "totalUtility =" << totalUtility << "\n";
        }
    }
}

exeTime timeoutcome;
TotalUtility outcome;

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
    cout << "read_file\n";
    read_file(dataset);
    // test read file
    // int counter = 0;
    // for (auto i = personal_edges.begin(); i != personal_edges.end(); ++i)
    // {
    //     if (counter == 3)
    //         break;
    //     cout << "user u : " << i->first << "\n";
    //     for (auto j = i->second.begin(); j != i->second.end(); ++j)
    //     {
    //         cout << "---> " << j->first << ", : " << j->second << "\n";
    //     }
    //     counter++;
    // }
    cout << "theta_filter\n";
    theta_filter();
    // cout << "\n\n\n ---------- after theta filter  --------------\n\n\n";
    // counter = 0;
    // for (auto i = personal_edges.begin(); i != personal_edges.end(); ++i)
    // {
    //     if (counter == 3)
    //         break;
    //     cout << "user u : " << i->first << "\n";
    //     for (auto j = i->second.begin(); j != i->second.end(); ++j)
    //     {
    //         cout << "---> " << j->first << ", : " << j->second << "\n";
    //     }
    //     counter++;
    // }

    //建heap
    // cout << "makeheap\n";
    // makeSWheap();

    // watch social heap
    // int k = 0;
    // while (k <= s_maxheap_size)
    // {
    //     cout << "(" << s_maxheap[k].useru << ", " << s_maxheap[k].userv << ")" << s_maxheap[k].value << "\n";
    //     k++;
    // }

    // watch social heap index
    // for (auto z = user_s_in_heap_idx.begin(); z != user_s_in_heap_idx.end(); ++z)
    // {
    //     std::cout << "userU : " << z->first
    //               << "\n";
    //     for (auto zz = z->second.begin(); zz != z->second.end(); ++zz)
    //     {
    //         std::cout << "userV : " << zz->first << " , idx : " << zz->second << "\n";
    //     }
    // }
    // std::cout << "max size u:" << s_maxheap[s_maxheap_size].useru << "max user v:" << s_maxheap[s_maxheap_size].userv << "\n";
    // std::cout << getMax(s_maxheap).useru << "\n";
    // std::cout << " s_maxheap_size : " << s_maxheap_size << "\n";

    // // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // auto SVC_begin = std::chrono::high_resolution_clock::now();
    // cout << "start SVC\n";
    // compare_max_utility("SVC");
    // auto SVC_end = std::chrono::high_resolution_clock::now();
    // timeoutcome.svc_elapsed_seconds = SVC_end - SVC_begin;
    // std::cout << "Time taken by program is : " << timeoutcome.svc_elapsed_seconds.count() << " sec" << endl;
    // outcome.svc.totU = totalUtility;
    // outcome.svc.perU = totalPersonalutility;
    // outcome.svc.socU = totalSocialutility;
    // std::cout << "total utility : " << outcome.svc.totU << endl;
    // std::cout << "total p utility : " << totalPersonalutility << endl;
    // std::cout << "total s utility : " << totalSocialutility << endl;
    // cout << "finish\n";
    // write_data(dataset, timeoutcome, outcome);

    return 0;
}