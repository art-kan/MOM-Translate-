#include <iostream>
#include <string>

#include "LibMOMTranslate.h"

constexpr auto PROGRAM_NAME = "momcli";

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::string;

bool IS_OPEN_AS_SHELL = false;

template <typename ch>
void displayWelcome(std::basic_ostream<ch>& out) {
  out << " __  __  ___  __  __     _____                    _       _       _ " << endl;
  out << "|  \\/  |/ _ \\|  \\/  |   |_   _| __ __ _ _ __  ___| | __ _| |_ ___| |" << endl;
  out << "| |\\/| | | | | |\\/| |     | || '__/ _` | '_ \\/ __| |/ _` | __/ _ \\ |" << endl;
  out << "| |  | | |_| | |  | |_    | || | | (_| | | | \\__ \\ | (_| | ||  __/_|" << endl;
  out << "|_|  |_|\\___/|_|  |_( )   |_||_|  \\__,_|_| |_|___/_|\\__,_|\\__\\___(_)" << endl;
  out << "                    |/                                              " << endl;
}

template <typename ch>
void displayLog(std::basic_ostream<ch>& out, const char* message) {
  out << "---> " << message << endl;
}

template <typename ch>
void displayError(std::basic_ostream<ch>& out, const char* message) {
  out << "Error: " << message << endl;
}

enum Command {
  HELP = 0,
  EXIT = 1,
  CONFIG = 2,
  TRANSLATE = 3,

  UNKNOWN = -1,
};

namespace CommandHandlers {

int handleExit(vector<string> operands) {
  displayLog(cout, "Bye.");
  exit(0);

  return 0;
}

int handleHelp(vector<string> operands) {
  if (!IS_OPEN_AS_SHELL) displayWelcome(cout);
  cout << endl;

  bool showAll = operands.size() == size_t{ 0 };
  bool shownSth = false;

  if (showAll || operands.front() == "config") {
    cout
      << PROGRAM_NAME << ".exe config <set/get> <section> <key> [value]" << endl
      << "\tis used to change configuration." << endl
      << endl
      << "e.g. > config set translator target_lang en  - sets your language to English" << endl
      << "     > config get microsoft rapidapi_key     - returns your RapidAPI key for "
      << "Microsoft Translator" << endl
      << endl;

    shownSth = true;
  }

  if (showAll || operands.front() == "translate") {
    cout
      << PROGRAM_NAME << ".exe translate" << endl
      << "\tit will be receiving text from CIN until you press Enter, CTRL-Z and Enter, " << endl
      << "\tthen will translate it via the specified translator service" << endl
      << endl
      << "e.g. | translate" << endl
      << "     | Ten little Soldier Boys went out to dine;" << endl
      << "     | One choked his little self and then there were nine." << endl
      << "     | Nine little Soldier Boys sat up very late;" << endl
      << "     | One overslept himselfand then there were eight." << endl
      << "     | ^Z<Enter>" << endl
      << endl;

    shownSth = true;
  }

  if (!shownSth) {
    displayError(cout, "Unknown usage. See help.");
    return 1;
  }

  return 0;
}

int handleConfig(vector<string> operands) {
  constexpr size_t outputSize = 1024;
  static char* output = (char*)malloc(sizeof(char) * outputSize);

  bool success;

  if (operands.empty()) {
    handleHelp({ "config" });
  }
  else if (operands.front() == "get") {
    if (operands.size() != 3) {
      displayError(cout, "Invalid number of operands. See help config.");
      return 1;
    }

    success = readConfigValue(operands.at(1).c_str(),
                              operands.at(2).c_str(),
                              nullptr,
                              output, outputSize);

    if (!success) {
      displayError(cout, "Could not read this config item.");
      return 1;
    }

    cout << output << endl;
  }
  else if (operands.front() == "set") {
    if (operands.size() != 4) {
      displayError(cout, "Invalid number of operands. See help conig");
      return 1;
    }

    success = writeConfigValue(operands.at(1).c_str(),
                               operands.at(2).c_str(),
                               operands.at(3).c_str());
    if (!success) {
      displayError(cout, "Failed to change configuration.");
      return 1;
    }
  }
  else {
    displayError(cout, "Unknown usage. See help config.");
    return 1;
  }

  return 0;
}

int handleTranslate(vector<string> _) {
  bool success;

  constexpr size_t bufferSize = 10240;
  static char* buffer = (char*)malloc(sizeof(char) * bufferSize);

  string text;
  string input;

  while (std::getline(cin, input)) {
    text += input;
  }

  cin.clear();

  success = translate(text.c_str(), buffer, bufferSize);

  if (!success) {
    displayError(cout, buffer);
    return 1;
  }

  cout << buffer << endl;

  return 0;
}

}

Command lookupCommand(const string &name) {
  if (name == "help") return Command::HELP;
  if (name == "exit") return Command::EXIT;
  if (name == "config") return Command::CONFIG;
  if (name == "translate") return Command::TRANSLATE;

  return Command::UNKNOWN;
}

int handleCommand(const vector<string> &tokens) {
  Command cmd = lookupCommand(tokens.empty() ? "" : tokens.front());

  vector<string> operands;
  if (!tokens.empty()) operands = vector<string>(tokens.begin() + 1, tokens.end());

  switch (cmd) {
    case Command::HELP:
      CommandHandlers::handleHelp(operands);
      break; 
    case Command::EXIT:
      CommandHandlers::handleExit(operands);
      break;
    case Command::CONFIG:
      CommandHandlers::handleConfig(operands);
      break;
    case Command::TRANSLATE:
      CommandHandlers::handleTranslate(operands);
      break;
    case Command::UNKNOWN:
      cout << "Unknown command." << endl;
      break;
    default:
      displayError(cout, "Internal error.");
      break;
  }

  return 0;
}

void split(const std::string& s, char delim, vector<string> &result) {
  string word;

  for (char x : s) {
    if (x == delim) {
      if (!word.empty()) {
        result.push_back(word);
        word = "";
      }
    } else {
      word += x;
    }
  }

  if (!word.empty()) result.push_back(word);
}

void runInteractiveShell() {
  std::string input;

  while (true) {
    cout << "> ";
    vector<string> tokens;
    std::getline(cin, input);

    split(input, ' ', tokens);

    if (tokens.empty()) continue;
    handleCommand(tokens);
  }
}

int main(int argc, char **argv)
{
  if (argc == 1) {
    displayWelcome(std::cout);
    IS_OPEN_AS_SHELL = true;
    runInteractiveShell();
    return 0;
  }

  vector<string> tokens(argv + 1, argv + argc);
  return handleCommand(tokens);
}

