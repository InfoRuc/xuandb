#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstring>
#include "test_data.h"

using namespace std;


void SplitString(const string& s, vector<string>& v, const string& c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

void str2int(int &int_temp,const string &string_temp)
{
    stringstream stream(string_temp);
    stream>>int_temp;
}

void str2float(float &float_temp, const string &string_temp)
{
    stringstream stream(string_temp);
    stream>>float_temp;
}

void parse_supplier_file(vector<Supplier> &supps)
{
    ifstream fin("benchmark_data/supplier.tbl");
    
    if (!fin.is_open())
    {
        cout << "Error opeing file" << endl;
        return;
    }

    for (int i = 0; i < 10000; i++)
    {
        Supplier s;
        vector<string> v;
        string temp;
        getline(fin, temp);
        SplitString(temp, v, "|");
        str2int(s.supp_key, v[0]);
        s.name = v[1];
        s.address = v[2];
        str2int(s.nation_key, v[3]);
        s.phone = v[4];
        str2float(s.acct_bal, v[5]);
        s.comment = v[6];
        supps.push_back(s);
        //cout << s.supp_key << " " << s.name << " " << s.address << " " << s.nation_key << " " <<  s.phone << " " <<  s.acct_bal << " " << s.comment << endl;
    }
}

void parse_customer_file(vector<Customer> &custs, string file_path)
{
    ifstream fin(file_path.c_str());
    
    if (!fin.is_open())
    {
        cout << "[TestUtil Error]: can not opeing file" << endl;
        return;
    }

    for (int i = 0; i < 10000; i++)
    {
        Customer s;
        vector<string> v;
        string temp;
        getline(fin, temp);
        SplitString(temp, v, "|");
        str2int(s.cust_key, v[0]);
        s.name = v[1];
        s.address = v[2];
        str2int(s.nation_key, v[3]);
        s.phone = v[4];
        str2float(s.acct_bal, v[5]);
        s.mktsegment = v[6];
        s.comment = v[7];
        custs.push_back(s);
    }
}

void flushPages(int id)
{
    cout << "[RecordMgr Test]: " << "#" << id << " page is dirty" << endl;
    cout << "[RecordMgr Test]: " << "Writing page to disk." << endl;
}

