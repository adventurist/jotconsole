#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <ctime>

static const std::string WELCOME_TITLE  = {
  "  ┌───────────────────────────────────────────────────────────┐\n"
  "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ Start   ░░░░░░░░░░░░░░░░░░░░░░░│\n"
  "  └───────────────────────────────────────────────────────────┘\n\n"};

static const std::string FAREWELL_TITLE = {
  "  ┌───────────────────────────────────────────────────────────┐\n"
  "  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ End     ░░░░░░░░░░░░░░░░░░░░░░░│\n"
  "  └───────────────────────────────────────────────────────────┘\n\n"};

namespace fs = std::filesystem;

std::time_t to_time_t(const fs::file_time_type& ftime)
{
  auto sctp = std::chrono::time_point_cast<
    std::chrono::system_clock::duration>(
      ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

  return std::chrono::system_clock::to_time_t(sctp);
}

std::string get_latest_log_file()
{
  namespace fs = std::filesystem;

  static const std::string directory   = ".";
               std::string latest_file = "chat.log";
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

std::string get_timestamped_filename(const std::string& filename)
{
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::stringstream ss;
  ss << filename << '-'
      << std::put_time(&tm, "%Y%m%d-%H%M%S");

  return ss.str();
}

void rotate_log_file(const std::string& filename) {
  auto ftime = fs::last_write_time(filename);
  std::time_t mod_time = to_time_t(ftime);
  std::tm* tm = std::localtime(&mod_time);

  std::stringstream ss;

  ss << "chat-" << std::put_time(tm, "%Y%m%d-%H%M%S") << ".log";

  const std::string new_name = ss.str();
  fs::rename(filename, new_name);

  std::cout << "Renamed last log to: " << new_name << std::endl;
}


static bool should_exit(const std::string& s)  { return s == "exit"; }

bool ask_if_new()
{
  std::string choice;
  std::cout << "Is this new material?" << std::endl;
  std::cin  >> choice;
  return
    choice == "y"   || choice == "Y"   ||
    choice == "yes" || choice == "YES" || choice == "Yes";
}

int main(int argc, char** argv)
{
  std::string user_choice;
  std::string filename;
  std::string buffer;

  std::cout << "Append to last log file?";
  std::cin >> user_choice;

  bool append_mode = (user_choice == "y"   || user_choice == "Y" || user_choice == "yes" ||
                      user_choice == "Yes" || user_choice == "YES");

  if (append_mode)
  {
    filename = get_latest_log_file();
    if (ask_if_new())
      buffer += '\n' + FAREWELL_TITLE + '\n' + '\n' + WELCOME_TITLE;
  }
  else
  {
    filename = "chat.log";

    if (fs::exists(filename))
    {
      rotate_log_file(filename);
      buffer = WELCOME_TITLE + '\n' + '\n';
    }
  }

  std::cout << buffer << std::endl;

  std::ofstream logfile(filename, std::ios_base::app);
  std::string   input;
  for (;;)
  {
    std::getline(std::cin, input);
    if (should_exit(input))
      break;

    buffer += input + "\n";

    logfile << buffer;

    buffer.clear();
  }

  std::cout  << '\n' + FAREWELL_TITLE << std::endl;

  logfile.close();

  return 0;
}

