#include <string>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// String Functions
void replace_all(std::string& str, const std::string& oldt, const std::string& newt, size_t st = 0);

void replace_all(std::string& str, const std::string& oldt, int newn, size_t st = 0);

size_t skip_line( // IDX of next line start, skipping leading whitespace
    const std::string& str,
    size_t pos);

size_t prev_line( // IDX of prev line start, skipping leading whitespace
    const std::string& str,
    size_t pos);

void trim_string(std::string& str); // Remove extra whitespace from string

int phrase_match(const std::string& str, const std::string& phrase);
int words_match(const std::string& str, const std::string& words);
