#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

static const char* WELCOME_TITLE  = {"  ┌───────────────────────────────────────────────────────────┐\n"
                                     "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ Welcome ░░░░░░░░░░░░░░░░░░░░░░░│\n"
                                     "  └───────────────────────────────────────────────────────────┘\n\n"};

static const char* FAREWELL_TITLE = {"  ┌───────────────────────────────────────────────────────────┐\n"
                                     "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ Goodbye ░░░░░░░░░░░░░░░░░░░░░░░│\n"
                                     "  └───────────────────────────────────────────────────────────┘\n\n"};

#define QUIT_PHRASE "exit"
#define PRINT_REQUEST "print this"

static bool shouldQuit(const std::string& s)  { return strcmp(s.c_str(), QUIT_PHRASE) == 0; }

void writeToFile(std::string s)
{
  std::ofstream output("chat.log", std::ios_base::app);
  output << s;
  output.close();
}

int main(int argc, char** argv)
{
  std::string buffer{WELCOME_TITLE};
  std::string input;

  std::cout << buffer << std::endl;

  for (;;)
  {
    std::getline(std::cin, input);
    if (shouldQuit(input))
      break;
    buffer += input + "\n";
  }

  std::cout << FAREWELL_TITLE << std::endl;
  buffer    += '\n';
  buffer    += FAREWELL_TITLE;
  writeToFile(buffer);

  return 0;
}

