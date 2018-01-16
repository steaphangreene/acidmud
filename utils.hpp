#include <string>

using namespace std;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// String Functions
void replace_all(string& str, const string& oldt, const string& newt, size_t st = 0);

void replace_all(string& str, const string& oldt, int newn, size_t st = 0);

size_t skip_line( // IDX of next line start, skipping leading whitespace
    const string& str,
    size_t pos);

size_t prev_line( // IDX of prev line start, skipping leading whitespace
    const string& str,
    size_t pos);

void trim_string(string& str); // Remove extra whitespace from string

int phrase_match(const string& str, const string& phrase);
int words_match(const string& str, const string& words);
