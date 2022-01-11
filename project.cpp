#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <set>
#include <algorithm>
#define G greater<int> // for pass the makefile's linter detect '>>'
#define V vector<int>
#define Vd vector<double>

using namespace std;

int main(int argc, char *argv[])
{
    ifstream fin1, fin2;
    fin1.open(argv[1], ios::in);
    fin2.open(argv[2], ios::in);

    ofstream fout;
    fout.open(argv[3], ios::out);

    string str, Bench, Testcase;
    int AddrBits, BlockSize, CachesSets, Associativity;
    fin1 >> str >> AddrBits;
    fin1 >> str >> BlockSize;
    fin1 >> str >> CachesSets;
    fin1 >> str >> Associativity;

    fout << "Address bits: " << AddrBits << endl;
    fout << "Block size: " << BlockSize << endl;
    fout << "Cache sets: " << CachesSets << endl;
    fout << "Associativity: " << Associativity << endl
         << endl;

    int OffsetBitCount = log2(BlockSize);
    int IndexingBitCount = log2(CachesSets);

    // create the bits list we can use and we need to output
    vector<V> List;
    vector<string> ForOutPut;
    int BenchCnt = 0;
    fin2 >> Bench >> Testcase;
    while (fin2 >> str && str != ".end")
    {
        ForOutPut.push_back(str);
        BenchCnt++;
        reverse(str.begin(), str.end());
        vector<int> tmp;
        for (int i = OffsetBitCount; i < AddrBits; i++)
            tmp.push_back(str[i] - '0');
        List.push_back(tmp);
    }

    // create Qtable
    vector<double> Qtable;
    for (int j = 0; j < AddrBits - OffsetBitCount; j++)
        Qtable.push_back(0.0);
    for (int i = 0; i < BenchCnt; i++)
        for (int j = 0; j < AddrBits - OffsetBitCount; j++)
            if (List[i][j] == 1)
                Qtable[j] += 1.0;
    for (int j = 0; j < AddrBits - OffsetBitCount; j++)
    {
        if (Qtable[j] > BenchCnt / 2)
            Qtable[j] = (BenchCnt - Qtable[j]) / BenchCnt;
        else
            Qtable[j] = Qtable[j] / (BenchCnt - Qtable[j]);
    }

    // create Ctable
    vector<Vd> Ctable;
    for (int i = 0; i < AddrBits - OffsetBitCount; i++)
    {
        vector<double> tmp;
        for (int j = 0; j < AddrBits - OffsetBitCount; j++)
            tmp.push_back(0.0);
        Ctable.push_back(tmp);
    }
    for (int i = 0; i < AddrBits - OffsetBitCount - 1; i++)
    {
        for (int j = i + 1; j < AddrBits - OffsetBitCount; j++)
        {
            double E = 0.0, D = 0.0;
            for (int b = 0; b < BenchCnt; b++)
                if (List[b][i] == List[b][j])
                    E += 1.0;
            D = BenchCnt - E;
            Ctable[i][j] = Ctable[j][i] = min(E, D) / max(E, D);
        }
    }

    // select the best bits
    set<int, G> Set; // set of indexing bits
    vector<int> used(AddrBits - OffsetBitCount);
    double maxQ = -1.0;
    // find the start bit
    int start;
    for (int i = 0; i < AddrBits - OffsetBitCount; i++)
    {
        if (Qtable[i] > maxQ)
        {
            maxQ = Qtable[i];
            start = i;
        }
    }
    used[start] = 1;
    // go
    for (int sel = 1; sel < IndexingBitCount; sel++)
    {
        // update Qtable
        for (int i = 0; i < AddrBits - OffsetBitCount; i++)
            if (used[i] == 0)
                Qtable[i] = Qtable[i] * Ctable[start][i];
        // keep select
        maxQ = -1.0;
        int minbit;
        for (int i = 0; i < AddrBits - OffsetBitCount; i++)
        {
            if (used[i] == 0 && Qtable[i] > maxQ)
            {
                maxQ = Qtable[i];
                minbit = i;
            }
        }
        used[minbit] = 1;
    }

    // print
    fout << "Offset bit count: " << OffsetBitCount << endl;
    fout << "Indexing bit count: " << IndexingBitCount << endl;
    fout << "Indexing bits:";
    // use a greater set to store
    for (int i = AddrBits - OffsetBitCount - 1; i >= 0; i--)
    {
        if (used[i] == 1)
        {
            Set.insert(i);
            fout << " " << i + OffsetBitCount;
        }
    }
    fout << endl
         << endl;

    // below is the simulation of cache
    string ID, Tag;
    fout << Bench << " " << Testcase << endl;
    int MissCount = 0;
    multimap<string, string> MappingCache;
    multimap<string, string>::iterator iterMap, LRU;
    for (int i = 0; i < BenchCnt; i++)
    {
        fout << ForOutPut[i] << " ";
        // reverse for convinience
        // note that tag and id will also reverse
        reverse(ForOutPut[i].begin(), ForOutPut[i].end());
        ID = Tag = "";
        for (int j = OffsetBitCount; j < AddrBits; j++)
        {
            if (Set.count(j - OffsetBitCount))
                ID += ForOutPut[i][j];
            else
                Tag += ForOutPut[i][j];
        }
        // for debug
        // fout << "ID: " << ID << " Tag: " << Tag << endl;
        int hit = 0;
        int count = MappingCache.count(ID);
        LRU = iterMap = MappingCache.find(ID);
        for (int j = 0; j < count; j++, iterMap++)
        {
            if (iterMap->second == Tag)
                hit = 1;
        }
        if (hit)
            fout << "hit" << endl;
        else
        {
            fout << "miss" << endl;
            MissCount++;
            if (count == Associativity)
                MappingCache.erase(LRU);
            MappingCache.insert(pair<string, string>(ID, Tag));
        }
    }
    fout << ".end" << endl;
    fout << endl;
    fout << "Total cache miss count: " << MissCount << endl;
    return 0;
}