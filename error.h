#include <string>

// print error message
void error(std::string_view message);

// print error message and error line
// line_num will indicate error line index
// pos will indicate error position
void error(std::string_view message, std::string_view line,
           std::size_t line_num, std::size_t pos);
