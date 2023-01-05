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
#include <time.h>
using namespace std;

struct swedge
{
    int useru;
    int userv;
    double value;
};
vector<swedge> p_maxheap;
vector<swedge> s_maxheap;
map<int, int> user_p_in_heap_idx;
map<int, int> user_s_in_heap_idx;

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

// Function to shift up the node in order
// to maintain the heap property
void shiftUp(int useru, int i, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx)
{
    while (i > 0 && maxheap[parent(i, maxheap)].value < maxheap[i].value)
    {

        // Swap parent and current node
        swap(maxheap[parent(i, maxheap)], maxheap[i]);

        //更新edge的 map id
        user_in_heap_idx[useru] = parent(i, maxheap);
        user_in_heap_idx[maxheap[i].useru] = i;

        // Update i to parent of i
        i = parent(i, maxheap);
    }
}

// Function to shift down the node in
// order to maintain the heap property
void shiftDown(int useru, int i, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
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
        user_in_heap_idx[useru] = maxIndex;
        user_in_heap_idx[maxheap[maxIndex].useru] = i;

        // cout << "swap : "
        //      << "  maxheap[i] = " << maxheap[i].useru << "  maxheap[maxIndex] " << maxheap[maxIndex].useru << "\n";
        swap(maxheap[i], maxheap[maxIndex]);

        shiftDown(useru, maxIndex, maxheap, user_in_heap_idx, maxheap_size);
    }
}

// Function to insert a new element
// in the Binary Heap
void insert(swedge p, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
{
    maxheap_size = maxheap_size + 1;

    // maxheap[size] = p;
    maxheap.push_back(p);

    // Shift Up to maintain heap property
    shiftUp(p.useru, maxheap_size, maxheap, user_in_heap_idx);
}

// Function to extract the element with
// maximum priority
swedge extractMax(vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
{
    // 抓最大的操作
    swedge max_result = maxheap[0];
    double result = max_result.value;
    int useru = max_result.useru;
    // Replace the value at the root
    // with the last leaf
    maxheap[0] = maxheap[maxheap_size];
    maxheap_size = maxheap_size - 1;
    user_in_heap_idx[useru] = -1;
    // Shift down the replaced element
    // to maintain the heap property
    cout << "maxheap[0].useru = " << maxheap[0].useru << "\n";
    shiftDown(maxheap[0].useru, 0, maxheap, user_in_heap_idx, maxheap_size);
    return result;
}

// Function to change the priority
// of an element
void changePriority(int useru, double minus_utility, vector<swedge> &maxheap, map<int, int> &user_in_heap_idx, int &maxheap_size)
{
    int i = user_in_heap_idx[useru];
    double oldp = maxheap[i].value;
    //計算新的utility
    maxheap[i].value -= minus_utility;

    //應該只會跑下降
    if (maxheap[i].value > oldp)
    {
        shiftUp(useru, i, maxheap, user_in_heap_idx);
    }
    else
    {
        shiftDown(useru, i, maxheap, user_in_heap_idx, maxheap_size);
    }
}

// Function to get value of the current
// maximum element
swedge getMax(vector<swedge> &maxheap)
{

    return maxheap[0];
}
// Driver Code

int main()
{

    int edge_size = 10;

    for (int i = 0; i < edge_size; ++i)
    {
        swedge edge;
        edge.useru = i;
        edge.userv = i + 1;
        edge.value = 0.1 + i;
        insert(edge, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
    }

    int i = 0;

    // Priority queue before extracting max
    cout << "Priority Queue : ";
    while (i <= p_maxheap_size)
    {
        cout << "(" << p_maxheap[i].useru << ", " << p_maxheap[i].value << ") ";
        i++;
    }

    cout << "\n";
    for (auto z = user_p_in_heap_idx.begin(); z != user_p_in_heap_idx.end(); ++z)
    {
        cout << "user : " << z->first << " , idx : " << z->second << "\n";
    }

    // Node with maximum priority
    cout << "Node with maximum priority : "
         << extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size) << "\n";
    cout << "Node with maximum priority : "
         << extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size) << "\n";
    // cout << "Node with maximum priority : "
    //      << extractMax(p_maxheap) << "\n";

    // Priority queue after extracting max
    cout << "Priority queue after "
         << "extracting maximum : ";
    int j = 0;
    while (j <= p_maxheap_size)
    {
        cout << "(" << p_maxheap[j].useru << ", " << p_maxheap[j].value << ") ";
        j++;
    }

    cout << "\n";
    for (auto z = user_p_in_heap_idx.begin(); z != user_p_in_heap_idx.end(); ++z)
    {
        cout << "user : " << z->first << " , idx : " << z->second << "\n";
    }

    // Change the priority of element
    changePriority(2, 5.0, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
    cout << "Priority queue after "
         << "priority change : ";

    int k = 0;
    while (k <= p_maxheap_size)
    {
        cout << "(" << p_maxheap[k].useru << ", " << p_maxheap[k].value << ") ";
        k++;
    }

    cout << "\n";
    for (auto z = user_p_in_heap_idx.begin(); z != user_p_in_heap_idx.end(); ++z)
    {
        cout << "user : " << z->first << " , idx : " << z->second << "\n";
    }

    cout << "Node with maximum priority : "
         << extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size) << "\n";
    cout << "Node with maximum priority : "
         << extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size) << "\n";
    cout << "Node with maximum priority : "
         << extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size) << "\n";
    // cout << "Node with maximum priority : "
    //      << extractMax(p_maxheap) << "\n";
    // cout << "Node with maximum priority : "
    //      << extractMax(p_maxheap) << "\n";
    // cout << "Node with maximum priority : "
    //      << extractMax(p_maxheap) << "\n";

    k = 0;
    while (k <= p_maxheap_size)
    {
        cout << "(" << p_maxheap[k].useru << ", " << p_maxheap[k].value << ") ";
        k++;
    }

    cout << "\n";
    for (auto z = user_p_in_heap_idx.begin(); z != user_p_in_heap_idx.end(); ++z)
    {
        cout << "user : " << z->first << " , idx : " << z->second << "\n";
    }

    return 0;
}