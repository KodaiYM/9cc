#include <string>

// print error message
void error(std::string_view message);

// print error message and error line
// pos will indicate error position
void error(std::string_view message, std::string_view line, std::size_t pos);
