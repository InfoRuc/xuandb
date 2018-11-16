#include "test_util.h"
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
    vector<Customer> custs;
    string fp = "benchmark_data/customer.tbl";
    if (argc > 1)
    {
        fp = argv[1];
    }
    parse_customer_file(custs, fp); 

    for (int i = 0; i < custs.size(); i++)
    {
        cout << "#" << i << " Record\n" << endl;
        cout << "Key: " << custs[i].cust_key << endl;
        cout << "name: " << custs[i].name << endl;
        cout << "address: " << custs[i].address << endl;
        cout << "nation key: " << custs[i].nation_key << endl;
        cout << "phone: " << custs[i].phone << endl;
        cout << "acct bal: " << custs[i].acct_bal << endl;
        cout << "mkt segment: " << custs[i].mktsegment << endl;
        cout << "comment: " << custs[i].comment << endl;
    }

    return 0;
}
