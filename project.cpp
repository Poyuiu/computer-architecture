#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <set>
#include <algorithm>
#define G greater<int>

using namespace std;

void Select(int AddrBits, int OffsetBitCount,
            int IndexingBitCount, set<int, G> &Set);

int main(int argc, char *argv[])
{
    ifstream fin1, fin2;
    fin1.open(argv[1], ios::in);
    fin2.open(argv[2], ios::in);

    ofstream fout;
    fout.open(argv[3], ios::out);

    string str;
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
    set<int, G> Set; // set of indexing bits
    set<int, G>::iterator iter;
    fout << "Offset bit count: " << OffsetBitCount << endl;
    fout << "Indexing bit count: " << IndexingBitCount << endl;
    fout << "Indexing bits:";
    Select(AddrBits, OffsetBitCount,
           IndexingBitCount, Set);
    for (iter = Set.begin(); iter != Set.end(); iter++)
        fout << " " << *iter;
    fout << endl
         << endl;

    fin2 >> str, fout << str << " ";
    fin2 >> str, fout << str << endl; // .benchmark testcase1

    // below is the simulation of cache
    string ID, Tag;
    int MissCount = 0;
    multimap<string, string> MappingCache;
    multimap<string, string>::iterator iterMap, LRU;
    while (fin2 >> str && str != ".end")
    {
        fout << str << " ";
        // reverse for convinience
        // note that tag and id will also reverse
        reverse(str.begin(), str.end());
        ID = Tag = "";
        for (int i = OffsetBitCount; i < str.length(); i++)
        {
            if (Set.count(i))
                ID += str[i];
            else
                Tag += str[i];
        }
        // for debug
        // fout << "ID: " << ID << " Tag: " << Tag << endl;
        int hit = 0;
        int count = MappingCache.count(ID);
        LRU = iterMap = MappingCache.find(ID);
        for (int i = 0; i < count; i++, iterMap++)
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
    fout << str << endl;
    fout << endl;
    fout << "Total cache miss count: " << MissCount << endl;
    return 0;
}
void Select(int Addrbits, int OffsetBitCount,
            int IndexingBitCount, set<int, G> &Set)
{
    for (int i = 0; i < IndexingBitCount; i++)
        Set.insert(OffsetBitCount + i);
    return;
}