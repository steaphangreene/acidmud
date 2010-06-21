#include <string>

using namespace std;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

//String Functions
void replace_all(string &str, const string &oldt, const string &newt);
void replace_all(string &str, const string &oldt, int newn);
void trim_string(string &str);	//Remove extra whitespace from string
