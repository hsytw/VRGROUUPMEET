Rho_selection compare_roho__utility2(map<int, Edge> &person, map<int, Edge> &social, map<int, Edge> &dirEdges, map<int, User> &users, map<int, User> &ans)
{
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
    Rho_selection selection;

    swedge max_edge;

    if (rho_soc >= rho_per && s_maxheap_size >= 0)
    {
        // std::cout << "soc is bigger : \n";
        maxrho = rho_soc;
        max_edge = extractMax(s_maxheap, user_s_in_heap_idx, s_maxheap_size);

        if (users.count(max_edge.useru) && users.count(max_edge.userv))
        {
            users[max_edge.useru].connectUser.insert(max_edge.userv);
            users[max_edge.useru].capacity--;
            if (users[max_edge.useru].capacity == 0)
            {

                users.erase(max_edge.useru);
                //把邊從heap移除掉 沒機會選別人 我自己單向的移掉 雙向的也移掉(別人沒機會跟我互看 so 單向 utility不用更新)
                for (auto zz = user_p_in_heap_idx[max_edge.useru].begin(); zz != user_p_in_heap_idx[max_edge.useru].end(); ++zz)
                {
                    heap_remove(zz->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                }

                // for (auto zz = user_s_in_heap_idx[max_edge.useru].begin(); zz != user_s_in_heap_idx[max_edge.useru].end(); ++zz)
                // {
                //     if (zz->second != -1)
                //     {
                //         heap_remove(zz->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                //         user_s_in_heap_idx[zz->first][max_edge.useru] = -1;
                //     }
                // }
            }
            else
            {
                //  單純更新 (移掉) 我看他 他看我在下面他改
                int u_v_i = user_p_in_heap_idx[max_edge.useru][max_edge.userv];
                heap_remove(u_v_i, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            }

            users[max_edge.userv].connectUser.insert(max_edge.useru);
            users[max_edge.userv].capacity--;
            if (users[max_edge.userv].capacity == 0)
            {
                users.erase(max_edge.userv);
                // 把邊從heap移除掉 沒機會選別人 我自己單向的移掉 雙向的也移掉(別人沒機會跟我互看 so 單向 utility不用更新)
                for (auto zz = user_p_in_heap_idx[max_edge.userv].begin(); zz != user_p_in_heap_idx[max_edge.userv].end(); ++zz)
                {
                    heap_remove(zz->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                }

                // std::cout << "2 remove d edges~~~~~~~~~~~~~~~~~~ : \n";
                // for (auto zz = user_s_in_heap_idx[max_edge.userv].begin(); zz != user_s_in_heap_idx[max_edge.userv].end(); ++zz)
                // {
                //     if (zz->second != -1)
                //     {
                //         heap_remove(zz->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                //         user_s_in_heap_idx[zz->first][max_edge.userv] = -1;
                //     }
                // }
            }
            else
            {
                // std::cout << "2 remove  edges~~~~~~~~~~~~~~~~~~ : \n";
                int v_u_i = user_p_in_heap_idx[max_edge.userv][max_edge.useru];
                heap_remove(v_u_i, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            }
        }
        else
        {
            max_edge.value = 0;
        }
    }
    else
    {
        if (p_maxheap_size >= 0)
        {
            // std::cout << "per is bigger : \n";
            maxrho = rho_per;
            max_edge = extractMax(p_maxheap, user_p_in_heap_idx, p_maxheap_size);
            //  change priority 還沒做這邊要做
            int v_u_i = user_p_in_heap_idx[max_edge.useru][max_edge.userv];
            int u_v_i = user_p_in_heap_idx[max_edge.userv][max_edge.useru];
            // 移除雙向
            heap_remove(v_u_i, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
            // 單向改權重
            double new_utility = (1 - lambdaWeight) * dirEdges[max_edge.userv].edges[max_edge.useru] + lambdaWeight * (social[max_edge.useru].edges[max_edge.userv] + social[max_edge.userv].edges[max_edge.useru]) * 1 / 2;
            changePriority(max_edge.userv, max_edge.useru, new_utility, p_maxheap, user_p_in_heap_idx, p_maxheap_size);

            //檢查是否選滿了
            users[max_edge.useru].connectUser.insert(max_edge.userv);
            users[max_edge.useru].capacity--;
            if (users[max_edge.useru].capacity == 0)
            {
                users.erase(max_edge.useru);
                //把邊從heap移除掉 沒機會選別人 我自己單向的移掉 雙向的也移掉(別人沒機會跟我互看 so 單向 utility不用更新)
                for (auto zz = user_p_in_heap_idx[max_edge.useru].begin(); zz != user_p_in_heap_idx[max_edge.useru].end(); ++zz)
                {
                    heap_remove(zz->second, p_maxheap, user_p_in_heap_idx, p_maxheap_size);
                }

                // for (auto zz = user_s_in_heap_idx[max_edge.useru].begin(); zz != user_s_in_heap_idx[max_edge.useru].end(); ++zz)
                // {
                //     if (zz->second != -1)
                //     {
                //         heap_remove(zz->second, s_maxheap, user_s_in_heap_idx, s_maxheap_size);
                //         user_s_in_heap_idx[zz->first][max_edge.useru] = -1;
                //     }
                // }
            }
        }
    }

    selection.userU = max_edge.useru;
    selection.userV = max_edge.userv;
    selection.utility = max_edge.value;
    selection.single_sel = rho_per > rho_soc ? 1 : 0;
    selection.perutility = max_edge.value * (1 - lambdaWeight);
    selection.socialutility = max_edge.value * lambdaWeight;

    std::cout << "\nTotal Max : \n";
    std::cout << "UserUID : " << selection.userU << " UserVID : " << selection.userV << " Single_Selection : " << selection.single_sel << endl;
    std::cout << "Max Rho = " << maxrho << endl;
    std::cout << "Max Effective Utility = " << max_edge.value << endl;
    std::cout << "p_maxheap_size : " << p_maxheap_size << ", "
              << "s_maxheap_size : " << s_maxheap_size << "\n";

    return selection;
}