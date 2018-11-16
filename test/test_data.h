#pragma once
#include <string>

using std::string;

struct Supplier
{
    int supp_key;
    string name;
    string address;
    int nation_key;
    string phone;
    float acct_bal;
    string comment;
};

struct Customer
{
    int cust_key;
    string name;
    string address;
    int nation_key;
    string phone;
    float acct_bal;
    string mktsegment;
    string comment;
};
