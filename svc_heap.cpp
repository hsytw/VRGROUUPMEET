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
// maxheap_size 記錄了 heap 的 index location
int p_maxheap_size = -1;
int s_maxheap_size = -1;

void watch_per_heap_idx();
void watch_soc_heap_idx();
void watch_heap(bool sh);

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

void watch_heap(bool sh)
{
    if (sh)
    {
        int k = 0;
        cout << "-------------------------     SOC     ------------------------------\n";
        while (k <= s_maxheap_size)
        {
            cout << "(" << s_maxheap[k].useru << ", " << s_maxheap[k].userv << ")" << s_maxheap[k].value << "\n";
            k++;
        }
        cout << "-------------------------------------------------------------------\n\n";
    }
    else
    {
        int k = 0;
        cout << "-------------------------    PER      ------------------------------\n";
        while (k <= p_maxheap_size)
        {
            cout << "(" << p_maxheap[k].useru << ", " << p_maxheap[k].userv << ")" << p_maxheap[k].value << "\n";
            k++;
        }
        cout << "-------------------------------------------------------------------\n\n";
    }
}

swedge getMax(bool sh, vector<swedge> &maxheap)
{
    // if (sh)
    // {
    //     int k = 0;
    //     cout << "-------------------------     SOC     ------------------------------\n";
    //     while (k <= s_maxheap_size)
    //     {
    //         cout << "(" << s_maxheap[k].useru << ", " << s_maxheap[k].userv << ")" << s_maxheap[k].value << "\n";
    //         k++;
    //     }
    //     cout << "-------------------------------------------------------------------\n\n";
    // }
    // else
    // {
    //     int k = 0;
    //     cout << "-------------------------    PER      ------------------------------\n";
    //     while (k <= p_maxheap_size)
    //     {
    //         cout << "(" << p_maxheap[k].useru << ", " << p_maxheap[k].userv << ")" << p_maxheap[k].value << "\n";
    //         k++;
    //     }
    //     cout << "-------------------------------------------------------------------\n\n";
    // }

    if (sh)
    {
        if (s_maxheap_size >= 0)
            return maxheap.at(0);

        swedge zero;
        zero.useru = -1;
        zero.userv = -1;
        zero.value = -1;

        return zero;
    }
    else
    {

        if (p_maxheap_size >= 0)
            return maxheap.at(0);

        swedge zero;
        zero.useru = -1;
        zero.userv = -1;
        zero.value = -1;

        return zero;
    }

    return maxheap.at(0);
}

// Function to shift up the node in order
// to maintain the heap property
void shiftUp(bool sh, int useru, int userv, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx)
{
    // i 一開始是 p_max_heap_size / s_max_heap_size 後面會不斷的更新成在現在這個 node 在 heap 的當前位置
    // i = 0 代表已經是最上面 so要 > 0 才繼續 shift up
    // 且要parent的value小於當前node的value
    while (i > 0 && maxheap[parent(i, maxheap)].value < maxheap[i].value)
    {
        int parent_index = parent(i, maxheap);
        swedge parent_node = maxheap[parent_index];
        // 更新edge的 map id
        user_in_heap_idx[useru][userv] = parent(i, maxheap);        // 原先的改成node的父節點的index
        user_in_heap_idx[parent_node.useru][parent_node.userv] = i; // 父節點的index要更新成現在這個node的index，

        // 若是 social heap, sh會是true
        if (sh == true)
        {
            // 因為是social heap 現在的node會有 userV userU 指到同樣這個index 要更新
            user_in_heap_idx[userv][useru] = parent(i, maxheap);
            // 父節點同樣有userV userU的指到同一個index
            user_in_heap_idx[parent_node.userv][parent_node.useru] = i;
        }

        // Swap parent and current node
        swap(maxheap[parent_index], maxheap[i]);

        // Update i to parent of i
        i = parent_index;
    }
}

// Function to shift down the node in
// order to maintain the heap property
void shiftDown(bool sh, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    // cout << "shiftDown : " << i << "\n";
    int useru = maxheap[i].useru;
    int userv = maxheap[i].userv;
    // cout << "useru :" << useru << "userv :" << userv << "\n";
    int maxIndex = i;

    // Left Child
    int l = leftChild(i, maxheap);
    // cout << "l : " << l << "maxheap_size : " << maxheap_size << "\n";
    if (l <= maxheap_size)
    {
        if (maxheap[l].value > maxheap[maxIndex].value)
        {
            // cout << "maxisl\n";
            maxIndex = l;
        }
    }

    // Right Child
    int r = rightChild(i, maxheap);
    // cout << "r : " << r << "maxheap_size : " << maxheap_size << "\n";
    if (r <= maxheap_size)
    {
        if (maxheap[r].value > maxheap[maxIndex].value)
        {
            // cout << "maxisr\n";
            maxIndex = r;
        }
    }

    // If i not same as maxIndex
    if (i != maxIndex)
    {
        // 要先更新edge的 map id
        // cout << "shiftDown1 : " << i << "\n";
        user_in_heap_idx[useru][userv] = maxIndex;
        user_in_heap_idx[maxheap[maxIndex].useru][maxheap[maxIndex].userv] = i;
        // user_in_heap_idx[useru][maxheap[maxIndex].useru] = maxIndex;
        // user_in_heap_idx[maxheap[maxIndex].useru][useru] = i;
        // cout << "shiftDown1 : finish\n";
        if (sh == true)
        {
            // cout << "shiftDown2 : " << i << "\n";
            user_in_heap_idx[userv][useru] = maxIndex;
            user_in_heap_idx[maxheap[maxIndex].userv][maxheap[maxIndex].useru] = i;
            // user_in_heap_idx[maxheap[maxIndex].useru][useru] = maxIndex;
            // user_in_heap_idx[useru][maxheap[maxIndex].useru] = i;
            // cout << "shiftDown2 : finish\n";
        }

        // cout << "swap : "
        //      << "  maxheap[i] = " << maxheap[i].useru << "  maxheap[maxIndex] " << maxheap[maxIndex].useru << "\n";
        swap(maxheap[i], maxheap[maxIndex]);
        // cout << "finish swap\n";
        shiftDown(sh, maxIndex, maxheap, user_in_heap_idx, maxheap_size);
    }
    // cout << "exit shiftDown : " << i << "\n";
}

// Function to insert a new element
// in the Binary Heap
void insert(bool sh, swedge p, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    // std::cout << "insert : " << p.useru << ", " << p.userv << "\n";
    // maxheap_size 記錄了 heap 的 index location
    maxheap_size = maxheap_size + 1;

    // maxheap[size] = p;
    // 先放在最後面，再做shift up
    maxheap.push_back(p);
    user_in_heap_idx[p.useru][p.userv] = maxheap_size;
    if (sh == true)
    {
        user_in_heap_idx[p.userv][p.useru] = maxheap_size;
    }
    // Shift Up to maintain heap property
    shiftUp(sh, p.useru, p.userv, maxheap_size, maxheap, user_in_heap_idx);
}

// Function to extract the element with
// maximum priority
vector<dirUtoV> tmp_remove_idx;
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
    swedge max_return;
    max_return.value = max_sw_edge.value;
    max_return.useru = max_sw_edge.useru;
    max_return.userv = max_sw_edge.userv;
    // Replace the value at the root
    // with the last leaf
    // cout << "with the last leaf\n";
    maxheap[0] = maxheap[maxheap_size];
    maxheap_size = maxheap_size - 1;
    // cout << "maxheap_size : " << maxheap_size << "\n";
    user_in_heap_idx[max_return.useru][max_return.userv] = -1;
    dirUtoV t;
    t.userU = max_return.useru;
    t.userV = max_return.userv;
    tmp_remove_idx.push_back(t);
    // 可以不用再紀錄最大的這個 node 的 index 因為要拿出來了
    // user_in_heap_idx[max_return.useru].erase(max_return.userv);
    // 因為 social heap 會有兩個紀錄，所以反向回來的也要刪掉。
    if (sh == true)
    {
        user_in_heap_idx[max_return.userv][max_return.useru] = -1;
        dirUtoV t2;
        t2.userU = max_return.userv;
        t2.userV = max_return.useru;
        tmp_remove_idx.push_back(t2);
        // user_in_heap_idx[max_return.userv].erase(max_return.useru);
    }

    // Shift down the replaced element
    // to maintain the heap property
    // std::cout << "next maxheap[0].useru = " << maxheap[0].useru << "\n";
    // cout << "extractMax shift down\n";
    shiftDown(sh, 0, maxheap, user_in_heap_idx, maxheap_size);
    // return max_sw_edge;

    // watch_heap_idx();

    return max_return;
}

void heap_remove(bool sh, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    maxheap[i].value = getMax(sh, maxheap).value + 1;

    // cout << "heap_remove" << "\n";

    // Shift the node to the root
    // of the heap
    // shiftUp(int useru, int userv, int i, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx)
    // cout << " heap remove shiftUp\n";
    shiftUp(sh, maxheap[i].useru, maxheap[i].userv, i, maxheap, user_in_heap_idx);

    // Extract the node
    // cout << " heap remove ExtractMax\n";
    extractMax(sh, maxheap, user_in_heap_idx, maxheap_size);

    // cout << "after remove :" << maxheap_size << "\n";
}

// Function to change the priority
// of an element
void changePriority(bool sh, int useru, int userv, double utility, vector<swedge> &maxheap, map<int, map<int, int>> &user_in_heap_idx, int &maxheap_size)
{
    int i = user_in_heap_idx[useru][userv];
    double oldp = maxheap[i].value;
    // 計算新的utility
    maxheap[i].value = utility;

    if (maxheap[i].value > oldp)
    {
        shiftUp(sh, useru, userv, i, maxheap, user_in_heap_idx);
    }
    else
    {
        shiftDown(sh, i, maxheap, user_in_heap_idx, maxheap_size);
    }
}

void read_file(string fileName);
void theta_filter();

void makeSWheap();
int head = 1;
void read_file(string fileName)
{
    // File pointer
    fstream file(fileName, ios::in);
    string line, word;
    // bool head = true;
    int test = 0;

    std::cout << "read file :" << fileName << endl
              << endl;
    if (file.is_open())
    {
        while (getline(file, line))
        {
            //  => skip first line
            if (head == 1)
            {
                // std::cout << line << endl;
                head = 0;
                continue;
            }

            stringstream templine(line);
            // => userU userV personalutility socialutility
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

            // => prints out the first 6 lines
            if (test < 6)
            {
                std::cout << "read line : " << line << endl;
                string sas;
                templine >> sas; // ???? useless
                std::cout << "test : " << test << " user U : " << userU << " user V : " << userV << " putility : " << putility << " sutility : " << sutility << endl;
                test++;
            }

            personal_edges[userU][userV] = putility;
            // => 避免沒有對面看回來的，如果已經有值，代表前面有userV userU，所以保留原值，否則給一個default
            if (!personal_edges[userV].count(userU))
                personal_edges[userV][userU] = putility;

            social_edges[userU][userV] = sutility;
            // => 避免沒有對面看回來的，如果已經有值，代表前面有userV userU，所以保留原值，否則給一個default
            if (!social_edges[userV].count(userU))
                social_edges[userV][userU] = sutility;

            // => 若 live_users 裡沒有這個userU，建立這個userU
            if (!live_users.count(userU))
            {
                User ucs;
                ucs.capacity = K_capacity;
                ucs.connectUser.clear();
                live_users[userU] = ucs;
                User ucs2;
                ucs2.capacity = K_capacity;
                ucs2.connectUser.clear();
                ans_users[userU] = ucs2;
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

    // 把小於theta的邊先存起來
    vector<dirUtoV> toDeleteEdges;
    int edgesize = 0;
    for (auto itr = personal_edges.begin(); itr != personal_edges.end(); itr++)
    {
        for (auto currentEntry = itr->second.begin(); currentEntry != itr->second.end(); ++currentEntry)
        {
            int userUID = itr->first;
            int userVID = currentEntry->first;
            if (currentEntry->second <= personal_theta)
            {
                dirUtoV de;
                de.userU = userUID;
                de.userV = userVID;
                toDeleteEdges.push_back(de);
            }
        }
    }

    //  刪除小於theta的邊
    for (auto itr2 = toDeleteEdges.begin(); itr2 != toDeleteEdges.end(); itr2++)
    {
        personal_edges[itr2->userU].erase(itr2->userV);
    }

    vector<int> tmp_del;
    // 把邊數小於k的使用者先存起來  totalUtility
    for (auto itr = personal_edges.begin(); itr != personal_edges.end(); itr++)
    {

        // 邊數小於k
        if (itr->second.size() <= K_capacity)
        {
            tmp_del.push_back(itr->first);

            for (auto cur = itr->second.begin(); cur != itr->second.end(); cur++)
            {
                int userU = itr->first;
                int userV = cur->first;
                dirUtoV tmp;
                tmp.userU = userU;
                tmp.userV = userV;

                live_users[tmp.userU].connectUser.insert(tmp.userV);
                live_users[tmp.userU].capacity--;

                ans_users[tmp.userU].connectUser.insert(tmp.userV);
                ans_users[tmp.userU].capacity--;

                // 這些使用者的personal preference utility加上
                totalUtility += (1 - lambdaWeight) * cur->second;
                totalPersonalutility += (1 - lambdaWeight) * cur->second;
                if (ans_users[userV].connectUser.count(userU))
                {
                    totalUtility += lambdaWeight * (social_edges[userU][userV] + social_edges[userV][userU]);
                    totalSocialutility += lambdaWeight * (social_edges[userU][userV] + social_edges[userV][userU]);
                }
                // **此外**，還必須把對面看過來的personal preference邊加上兩人的 social utility 。 (乘0.5因為後面makeheap的都有)  << adjust
                // personal_edges[userV][userU] = personal_edges[userV][userU] + lambdaWeight * (social_edges[userU][userV] + social_edges[userV][userU]);
            }
        }
    }

    cout << "Bef del user size = " << live_users.size() << "\n";
    // 刪除這些使用者
    for (auto toDel = tmp_del.begin(); toDel != tmp_del.end(); toDel++)
    {
        personal_edges.erase(*toDel);
        live_users.erase(*toDel);
    }

    cout << "After del user size = " << live_users.size() << "\n";

    cout << "After filter :\n";
    cout << "Total : " << totalUtility << ", Per : " << totalPersonalutility << ", Soc : " << totalSocialutility << "\n";
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
            // 如果對方有選我，要加上社交效益
            // if 這邊出錯了!!! 因為userVID已被刪除
            if (ans_users[userVID].connectUser.count(userUID))
            {
                single_effectiveUtility = (1 - lambdaWeight) * personal_edges[userUID][userVID] + lambdaWeight * (social_edges[userUID][userVID] + social_edges[userVID][userUID]);
            }
            else
            {
                single_effectiveUtility = (1 - lambdaWeight) * personal_edges[userUID][userVID];
            }

            // single_effectiveUtility = (1 - lambdaWeight) * personal_edges[userUID][userVID];

            swedge psw;
            psw.useru = userUID;
            psw.userv = userVID;
            psw.value = single_effectiveUtility;

            // std::cout << userUID << " , " << userVID << "\n";
            insert(false, psw, p_maxheap, user_p_in_heap_idx, p_maxheap_size);

            // 建立 social 的 heap
            // 先判定對方是否邊數小於k，若是代表都已經單選完成，不用建立social edge。(還活著代表邊數大於k)
            if (live_users.count(userVID))
            {
                double_effectiveUtility = (1 - lambdaWeight) * (personal_edges[userUID][userVID] + personal_edges[userVID][userUID]) + lambdaWeight * (social_edges[userUID][userVID] + social_edges[userVID][userUID]); //* 0.5
                // didn't make 才要建，只需建造一條即可
                if (!check_s_edge_made[userUID].count(userVID))
                {
                    swedge ssw;
                    ssw.useru = userUID;
                    ssw.userv = userVID;
                    ssw.value = double_effectiveUtility;
                    // true 因為是social heap index記錄較麻煩
                    insert(true, ssw, s_maxheap, user_s_in_heap_idx, s_maxheap_size);

                    check_s_edge_made[userUID][userVID] = true;
                    check_s_edge_made[userVID][userUID] = true;
                }
            }
        }
    }

    // social 那邊的insert 已將 sh設為true，所以當下直接反向的建回來
    // vector<vuindex> vuidxtmp;
    // for (auto z = user_s_in_heap_idx.begin(); z != user_s_in_heap_idx.end(); ++z)
    // {
    //     for (auto zz = z->second.begin(); zz != z->second.end(); ++zz)
    //     {
    //         vuindex tete;
    //         tete.useru = zz->first;
    //         tete.userv = z->first;
    //         tete.idx = zz->second;
    //         vuidxtmp.push_back(tete);
    //     }
    // }
    // // user_s_in_heap_idx 反向建造回來
    // for (auto i = vuidxtmp.begin(); i != vuidxtmp.end(); ++i)
    // {
    //     int useru = i->useru;
    //     int userv = i->userv;
    //     int idx = i->idx;
    //     user_s_in_heap_idx[useru][userv] = idx;
    // }
}
vector<int> tmp_remove_user;
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

    swedge per_edge = getMax(false, p_maxheap);
    if (p_maxheap_size < 0)
        per_edge.value = -1;
    swedge soc_edge = getMax(true, s_maxheap);
    if (s_maxheap_size < 0)
        soc_edge.value = -1;
    cout << "per_edge.value : " << per_edge.value << "\n";
    cout << "soc_edge.value : " << soc_edge.value << "\n";
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
        // cout << "after soc extractMax\n";
        useru = soc_edge.useru;
        userv = soc_edge.userv;
        maxrho = soc_edge.value / 2;
        effective_utility = soc_edge.value;
        // 看到這
        if (live_users.count(useru) && live_users.count(userv))
        {
            live_users[useru].capacity--;
            live_users[useru].connectUser.insert(userv);
            live_users[userv].capacity--;
            live_users[userv].connectUser.insert(useru);

            ans_users[useru].capacity--;
            ans_users[useru].connectUser.insert(userv);
            ans_users[userv].capacity--;
            ans_users[userv].connectUser.insert(useru);

            // cout << "bef soc remove\n";
            if (user_p_in_heap_idx.count(useru) && user_p_in_heap_idx[useru].count(userv))
            {
                heap_remove(false, user_p_in_heap_idx[useru][userv], p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            }
            if (user_p_in_heap_idx.count(userv) && user_p_in_heap_idx[userv].count(useru))
            {
                heap_remove(false, user_p_in_heap_idx[userv][useru], p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            }
            for (auto d : tmp_remove_idx)
            {
                user_p_in_heap_idx[d.userU].erase(d.userV);
            }
            tmp_remove_idx.clear();
            // cout << "aft soc remove\n";
            if (live_users[useru].capacity <= 0)
            {
                tmp_remove_user.push_back(useru); // live_users.erase(useru);
                // cout << "live_users erase useru\n";
                // cout << "find useru : " << user_p_in_heap_idx.count(useru) << "\n";
                // 把personal 邊刪光，不用更新對方看過來的邊，因為若是前面有單選已經更新，這些有機會加到，但是這些剩的代表user u 沒看過去，所以沒機會互看。
                for (auto i = user_p_in_heap_idx[useru].begin(); i != user_p_in_heap_idx[useru].end(); ++i)
                {
                    // cout << "user_u : " << useru << " , p i->first : " << i->first << "\n";
                    if (i->second != -1)
                    {
                        heap_remove(false, i->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                    }
                }
                for (auto d : tmp_remove_idx)
                {
                    // cout << "*** kill *** " << d.userU << "->" << d.userV << "\n";
                    user_p_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
                // social 邊直接移除 不用change priority
                for (auto i = user_s_in_heap_idx[useru].begin(); i != user_s_in_heap_idx[useru].end(); ++i)
                {
                    // cout << "user_u : " << useru << " , s i->first : " << i->first << "\n";
                    if (i->second != -1)
                    {
                        // 反向的邊也會刪除，因為sh = true會在ex會在extract_max裡面把他做掉
                        heap_remove(true, i->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                    }
                }
                for (auto d : tmp_remove_idx)
                {
                    // cout << "*** kill *** " << d.userU << "<->" << d.userV << "\n";
                    user_s_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
            }
            // cout << "TO USER U\n";
            if (live_users[userv].capacity <= 0)
            {
                tmp_remove_user.push_back(userv); // live_users.erase(userv);
                // cout << "live_users erase userv\n";
                // cout << "find userv : " << user_p_in_heap_idx.count(userv) << "\n";
                // 把personal 邊刪光，不用更新對方看過來的邊，因為若是前面有單選已經更新，這些有機會加到，但是這些剩的代表user u 沒看過去，所以沒機會互看。
                for (auto i = user_p_in_heap_idx[userv].begin(); i != user_p_in_heap_idx[userv].end(); ++i)
                {
                    if (i->second != -1)
                    {
                        // cout << "p i->second : " << i->second;
                        heap_remove(false, i->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                    }
                }
                for (auto d : tmp_remove_idx)
                {
                    user_p_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
                // social 邊直接移除 不用change priority
                for (auto i = user_s_in_heap_idx[userv].begin(); i != user_s_in_heap_idx[userv].end(); ++i)
                {
                    if (i->second != -1)
                    {
                        // cout << "s i->second : " << i->second;
                        // 反向的邊也會刪除，因為sh = true會在ex會在extract_max裡面把他做掉
                        heap_remove(true, i->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                    }
                }
                for (auto d : tmp_remove_idx)
                {
                    user_s_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
            }

            // 待做 -----> 改成 remove
            // sh應該是false，因為是改 personal 的 heap
            // 位置移到上方，應該先做。
            // changePriority(false, userv, useru, 0, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            // changePriority(false, useru, userv, 0, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
        }
        else
        {
            // 有人不能選了 無效選擇
            effective_utility = 0;
        }
    }
    else
    {
        // cout << "use single selection\n";
        single = true;
        // cout << "***************************** p_maxheap_size : " << p_maxheap_size << "\n";
        // cout << "***************************** s_maxheap_size : " << s_maxheap_size << "\n";
        per_edge = extractMax(false, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
        // cout << "after per extractMax\n";
        // cout << "***************************** p_maxheap_size : " << p_maxheap_size << "\n";
        // cout << "***************************** s_maxheap_size : " << s_maxheap_size << "\n";
        useru = per_edge.useru;
        userv = per_edge.userv;
        maxrho = per_edge.value;
        effective_utility = per_edge.value;
        if (live_users.count(useru))
        {
            live_users[useru].capacity--;
            live_users[useru].connectUser.insert(userv);

            ans_users[useru].capacity--;
            ans_users[useru].connectUser.insert(userv);

            // 1. update v->u per
            if (user_p_in_heap_idx.count(userv) && user_p_in_heap_idx[userv].count(useru))
            {
                int v_idx = user_p_in_heap_idx[userv][useru];
                double nu = (1 - lambdaWeight) * personal_edges[userv][useru] + lambdaWeight * (social_edges[useru][userv] + social_edges[userv][useru]); //*0.5
                // cout << "bef cPriority \n";
                changePriority(false, userv, useru, nu, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            }

            // cout << "***************************** p_maxheap_size : " << p_maxheap_size << "\n";
            // cout << "***************************** s_maxheap_size : " << s_maxheap_size << "\n";
            // cout << "bef heap_remove \n";
            // watch_per_heap_idx

            // 2. kill double sel

            if (user_s_in_heap_idx.count(userv) && user_s_in_heap_idx[userv].count(useru))
            {

                heap_remove(true, user_s_in_heap_idx[userv][useru], s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                // cout << "***************************** p_maxheap_size : " << p_maxheap_size << "\n";
                // cout << "***************************** s_maxheap_size : " << s_maxheap_size << "\n";
                // cout << "aft heap_remove \n";
                for (auto d : tmp_remove_idx)
                {
                    user_s_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
            }

            if (live_users[useru].capacity <= 0)
            {
                // cout << "\n\n\n del user u :" << useru << "\n\n";
                tmp_remove_user.push_back(useru); // live_users.erase(useru);
                // watch_heap_idx();
                // 把personal 邊刪光，不用更新對方看過來的邊，因為若是前面有單選已經更新，這些有機會加到，但是這些剩的代表user u 沒看過去，所以沒機會互看。
                // cout << "user_p_in_heap_idx[useru].size() = " << user_p_in_heap_idx[useru].size() << "\n";
                // cout << "*********************************  remove personal\n";
                for (auto i : user_p_in_heap_idx[useru])
                {
                    // cout << "user_p_in_heap_idx[useru].size() = " << user_p_in_heap_idx[useru].size() << "\n";
                    // cout << "userv : " << i.first << " idx :" << i.second << "\n";
                    if (i.second != -1)
                    {
                        // cout << "======================================> do heap remove\n";
                        heap_remove(false, i.second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                    }
                    // watch_heap_idx();
                }
                for (auto d : tmp_remove_idx)
                {
                    user_p_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
                // watch_heap_idx();
                // for (auto i = user_p_in_heap_idx[useru].begin(); i != user_p_in_heap_idx[useru].end(); ++i)
                // {
                //     cout << "user_p_in_heap_idx[useru].size() = " << user_p_in_heap_idx[useru].size() << "\n";
                //     cout << "userv : " << i->first << " idx :" << i->second << "\n";
                //     if (i->second != -1)
                //     {
                //         cout << "======================================> do heap remove\n";
                //         heap_remove(false, i->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                //     }
                //     watch_heap_idx();
                // }
                // cout << "********************************* remove social\n";
                // social 邊直接移除 不用change priority
                for (auto i = user_s_in_heap_idx[useru].begin(); i != user_s_in_heap_idx[useru].end(); ++i)
                {
                    if (i->second != -1)
                    {
                        // 反向的邊也會刪除，因為sh = true會在ex會在extract_max裡面把他做掉
                        heap_remove(true, i->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                    }
                }
                for (auto d : tmp_remove_idx)
                {
                    user_s_in_heap_idx[d.userU].erase(d.userV);
                }
                tmp_remove_idx.clear();
            }
        }
        else
        {
            effective_utility = 0;
        }
    }

    for (auto d : tmp_remove_user)
    {
        cout << "remove user : " << d << "\n";
        live_users.erase(d);
    }
    tmp_remove_user.clear();

    selection.userU = useru;
    selection.userV = userv;
    selection.utility = effective_utility;
    selection.single_sel = single;
    if (single)
    {
        selection.perutility = (1 - lambdaWeight) * personal_edges[useru][userv];
        if (ans_users[userv].connectUser.count(useru))
        {
            selection.socialutility = lambdaWeight * (social_edges[useru][userv] + social_edges[userv][useru]);
            ;
        }
        else
        {
            selection.socialutility = 0;
        }
    }
    else
    {
        selection.perutility = (1 - lambdaWeight) * (personal_edges[useru][userv] + personal_edges[userv][useru]);
        selection.socialutility = lambdaWeight * (social_edges[useru][userv] + social_edges[userv][useru]);
    }
    // selection.perutility = effective_utility * (1 - lambdaWeight);
    // selection.socialutility = effective_utility * lambdaWeight;

    std::cout << "\nTotal Max : \n";
    std::cout << "UserUID : " << selection.userU << " UserVID : " << selection.userV << " Single_Selection : " << selection.single_sel << endl;
    std::cout << "Max Rho = " << maxrho << endl;
    std::cout << "Max Effective Utility = " << effective_utility << endl;
    std::cout << "Per utility add: " << selection.perutility << " , Soc utility add: " << selection.socialutility << "\n";
    std::cout << "p_maxheap_size : " << p_maxheap_size << ", "
              << "s_maxheap_size : " << s_maxheap_size << "\n";
    return selection;
}

void compare_max_utility(string way)
{

    if (way == "SVC")
    {
        cout << "start_user_size = " << live_users.size() << "\n";
        while (live_users.size() > 0)
        {
            Rho_selection sel = compare_roho__utility2();
            std::cout << "user.size() = " << live_users.size() << endl;
            if (sel.userU == -1 || sel.userV == -1)
            {
                break;
            }
            totalUtility += sel.utility;
            totalPersonalutility += sel.perutility;
            totalSocialutility += sel.socialutility;

            std::cout << "totalUtility =" << totalUtility << "\n\n";
        }
    }
}

void test_read_file()
{
    int counter = 0;
    for (auto i = personal_edges.begin(); i != personal_edges.end(); ++i)
    {
        if (counter == 3)
            break;
        cout << "user u : " << i->first << "\n";
        for (auto j = i->second.begin(); j != i->second.end(); ++j)
        {
            cout << "---> " << j->first << ", : " << j->second << "\n";
        }
        counter++;
    }
}

exeTime timeoutcome;
TotalUtility outcome;
void watch_per_heap_idx()
{
    // watch per heap index
    cout << "\n\n\n-------------------------------------------------------\n";
    for (auto z = user_p_in_heap_idx.begin(); z != user_p_in_heap_idx.end(); ++z)
    {
        std::cout << "userU : " << z->first
                  << "\n";
        for (auto zz = z->second.begin(); zz != z->second.end(); ++zz)
        {
            std::cout << "userV : " << zz->first << " , idx : " << zz->second << "\n";
        }
        cout << "\n";
    }
    cout << "-------------------------------------------------------\n\n\n";
    // std::cout << "max size u:" << s_maxheap[s_maxheap_size].useru << "max user v:" << s_maxheap[s_maxheap_size].userv << "\n";
    // std::cout << getMax(s_maxheap).useru << "\n";
    // std::cout << " s_maxheap_size : " << s_maxheap_size << "\n";
}

void watch_soc_heap_idx()
{
    // watch soc heap index
    cout << "\n\n\n-------------------------------------------------------\n";
    for (auto z = user_s_in_heap_idx.begin(); z != user_s_in_heap_idx.end(); ++z)
    {
        std::cout << "userU : " << z->first
                  << "\n";
        for (auto zz = z->second.begin(); zz != z->second.end(); ++zz)
        {
            std::cout << "userV : " << zz->first << " , idx : " << zz->second << "\n";
        }
        cout << "\n";
    }
    cout << "-------------------------------------------------------\n\n\n";
    // std::cout << "max size u:" << s_maxheap[s_maxheap_size].useru << "max user v:" << s_maxheap[s_maxheap_size].userv << "\n";
    // std::cout << getMax(s_maxheap).useru << "\n";
    // std::cout << " s_maxheap_size : " << s_maxheap_size << "\n";
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
    cout << "read_file\n";
    read_file(dataset);
    // ans_users = live_users;
    // test read file
    // test_read_file()
    cout << "theta_filter\n";
    theta_filter();
    // cout << "\n\n\n ---------- after theta filter  --------------\n\n\n";
    // test_read_file()

    // 建heap
    //  cout << "makeheap\n";
    makeSWheap();
    cout << "***************************** p_maxheap_size : " << p_maxheap_size << "\n";
    cout << "***************************** s_maxheap_size : " << s_maxheap_size << "\n";
    // watch_heap(true);
    // watch_heap(false);

    // watch_per_heap_idx();
    // watch_soc_heap_idx();

    // std::cout << "max size u:" << s_maxheap[s_maxheap_size].useru << "max user v:" << s_maxheap[s_maxheap_size].userv << "\n";
    // std::cout << getMax(s_maxheap).useru << "\n";
    // std::cout << " s_maxheap_size : " << s_maxheap_size << "\n";

    // // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    auto SVC_begin = std::chrono::high_resolution_clock::now();
    cout << "start SVC\n";
    compare_max_utility("SVC");
    cout << "Configuration : \n";
    for (auto i = ans_users.begin(); i != ans_users.end(); ++i)
    {
        cout << "user id : " << i->first << " => ";
        for (auto j : i->second.connectUser)
        {
            cout << j << " ";
        }
        cout << "\n";
    }
    auto SVC_end = std::chrono::high_resolution_clock::now();
    timeoutcome.svc_elapsed_seconds = SVC_end - SVC_begin;
    std::cout << "Time taken by program is : " << timeoutcome.svc_elapsed_seconds.count() << " sec" << endl;
    outcome.svc.totU = totalUtility;
    outcome.svc.perU = totalPersonalutility;
    outcome.svc.socU = totalSocialutility;
    std::cout << "total utility : " << outcome.svc.totU << endl;
    std::cout << "total p utility : " << totalPersonalutility << endl;
    std::cout << "total s utility : " << totalSocialutility << endl;
    cout << "finish\n";
    write_data(dataset, timeoutcome, outcome);

    return 0;
}