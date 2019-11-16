#include <string>

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

int phrase_match(const std::string& str, const std::string& phrase /* lowercase! */);
int phrase_match_sensitive(const std::string& str, const std::string& phrase /* non-empty! */);
int words_match(const std::string& str, const std::string& words);

constexpr int8_t ascii_tolower(int8_t chr) {
  return (chr >= 'A' && chr <= 'Z') ? (chr - 'A') + 'a' : chr;
}

// First 9 character substring *must* be unique!
constexpr int64_t tokenize_string(const char str[]) {
  int64_t ret = int8_t(ascii_tolower(str[0]));
  if (str[1] != 0) {
    ret <<= 7;
    ret |= int8_t(ascii_tolower(str[1]));
    if (str[2] != 0) {
      ret <<= 7;
      ret |= int8_t(ascii_tolower(str[2]));
      if (str[3] != 0) {
        ret <<= 7;
        ret |= int8_t(ascii_tolower(str[3]));
        if (str[4] != 0) {
          ret <<= 7;
          ret |= int8_t(ascii_tolower(str[4]));
          if (str[5] != 0) {
            ret <<= 7;
            ret |= int8_t(ascii_tolower(str[5]));
            if (str[6] != 0) {
              ret <<= 7;
              ret |= int8_t(ascii_tolower(str[6]));
              if (str[7] != 0) {
                ret <<= 7;
                ret |= int8_t(ascii_tolower(str[7]));
                if (str[8] != 0) {
                  ret <<= 7;
                  ret |= int8_t(ascii_tolower(str[8]));
                }
              }
            }
          }
        }
      }
    }
  }
  return ret;
}
