#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>

//--------------------------------------------------------
//--------------------------------------------------------
static const std::string start_msg  =
  "  ┌───────────────────────────────────────────────────────────┐\n"
  "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ Start   ░░░░░░░░░░░░░░░░░░░░░░░│\n"
  "  └───────────────────────────────────────────────────────────┘\n\n";
//--------------------------------------------------------
static const std::string end_msg =
  "  ┌───────────────────────────────────────────────────────────┐\n"
  "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ End     ░░░░░░░░░░░░░░░░░░░░░░░│\n"
  "  └───────────────────────────────────────────────────────────┘\n\n";
//--------------------------------------------------------
//--------------------------------------------------------

namespace fs = std::filesystem;

std::time_t to_time_t(const fs::file_time_type& ftime)
{
  auto sctp = std::chrono::time_point_cast<
    std::chrono::system_clock::duration>(
      ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

  return std::chrono::system_clock::to_time_t(sctp);
}
//--------------------------------------------------------
std::string get_latest_log_file()
{
  namespace fs = std::filesystem;

  static const std::string directory   = ".";
               std::string latest_file = "jot.log";
               std::time_t latest_time = 0;

  for (const auto& entry : fs::directory_iterator(directory))
  {
    if (entry.is_regular_file() && entry.path().extension() == ".log")
    {
      auto ftime = fs::last_write_time(entry);
      auto sctp = to_time_t(ftime);
      if (sctp > latest_time)
      {
        latest_time = sctp;
        latest_file = entry.path().string();
      }
    }
  }
  return latest_file;
}
//--------------------------------------------------------
std::string format_time(const char* format, const std::time_t& t = std::time(nullptr))
{
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), format);

  return ss.str();
}
//--------------------------------------------------------
bool is_file_open_by_any_process(const std::string& filename)
{
  const std::string command = "lsof " + filename + " 2>/dev/null";
  FILE* pipe = popen(command.c_str(), "r");
  if (!pipe)
  {
    std::cerr << "Failed to run lsof command" << std::endl;
    return false;
  }

  char buffer[128];
  bool is_open = false;
  while (fgets(buffer, sizeof(buffer), pipe))
    if (strstr(buffer, filename.c_str()))
    {
      is_open = true;
      break;
    }

  pclose(pipe);

  return is_open;
}
//--------------------------------------------------------
void rotate_log_file(const std::string& filename)
{
  const auto        ftime       = fs::last_write_time(filename);
  const std::time_t mod_time    = to_time_t(ftime);
  const auto        pos         = filename.find_last_of('.');
  std::string       simple_name = (pos == filename.npos) ? filename :
                                                           filename.substr(0, pos);
  const std::string format   = simple_name + "-%Y%m%d-%H%M%S.log";
  const std::string new_name = format_time(format.c_str(), mod_time);

  fs::rename(filename, new_name);
  std::cout << "Renamed last log to: " << new_name << std::endl;
}

//--------------------------------------------------------
auto is_yes = [](auto s)
{
  return s == "y"   || s == "Y" || s == "yes" || s == "YES" || s == "Yes";
};
//--------------------------------------------------------
bool should_exit(const std::string& s)  { return s == "exit"; }
//--------------------------------------------------------
bool ask_if_new()
{
  std::string choice;
  std::cout << "Is this new material? " << std::endl;
  std::cin  >> choice;
  return is_yes(choice);
}

//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
int main(int argc, char** argv)
{
  std::string input;
  std::string filename;
  std::string buffer;

  //------ Configure
  std::cout << "Append to last log file? ";
  std::cin >> input;

  if (is_yes(input))
  {
    filename = get_latest_log_file();
    if (is_file_open_by_any_process(filename))
      filename.clear();
    else
    if(ask_if_new())
      buffer += '\n' + end_msg + '\n' + '\n' + start_msg;
  }

  if (filename.empty())
  {
    std::cout << "Filename still not set. Using default" << std::endl;

    filename = "jot.log";
    if (fs::exists(filename))
    {
      if (is_file_open_by_any_process(filename))
        filename = format_time("jot-%Y%m%d-%H%M%S.log");
      else
        rotate_log_file(filename);

      buffer = start_msg + '\n' + '\n';
    }
  }

  buffer += format_time("\n%Y-%m-%d %H:%M:%S\n\n");

  std::cout << "Filename set to " << filename << "\n\n" << buffer << std::endl;

  std::ofstream logfile(filename, std::ios_base::app);
  for (;;)
  {
    std::getline(std::cin, input);
    if (should_exit(input))
      break;

    buffer += input + "\n";

    logfile << buffer;

    buffer.clear();
  }

  std::cout << '\n' + end_msg << std::endl;

  logfile.close();

  return 0;
}
