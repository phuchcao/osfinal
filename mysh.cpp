#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <vector>

#include "mysh.hpp"
#include "tokenizer.hpp"
#include "builtin.hpp"
#include "utilities.hpp"

using namespace std;

Tokenizer g_tokenizer(DELIM, SPECIAL_DELIM);
int FAT[65526];
ft_entry ftable[MAXFTSIZE];
int datareg;
int diskfd;

// SIGNAL

void resetSignalHandler(void) {
  for (int i = SIGHUP; i<= SIGUSR2; i++) {
    signal(i, SIG_DFL);
  }
}



// COMMAND 

Command::Command(void) {
  argv.clear();
  bg_flag = false;
}

Command::~Command(void) {}

ostream& operator<<(ostream& os, Command const& cmd) {
  for (int i = 0; i < cmd.argv.size(); i++) {
    os << cmd.argv[i] << " ";
  }
  return os << endl;
}



string readLine(void) {
  char* line = NULL;
  size_t size = 0;
  if (getline(&line, &size, stdin) == -1) {
    //error handling
  }
  line[strcspn(line, "\r\n")] = 0;
  return string(line);
}



void parseLine(string& line, vector<Command>& output) {
  g_tokenizer.setString(line);

  Command cmd;
  string token;

  while ((token = g_tokenizer.getNextToken()) != "") {
    if (token == "&") {
      cmd.bg_flag = true;
    }
		
    if (token == "&" || token == ";") {
      output.push_back(cmd);

      cmd.argv.clear();
      cmd.bg_flag = false;

      continue;
    }

    cmd.argv.push_back(token);
  }

  if (! cmd.argv.empty()) {
    output.push_back(cmd);
  }
}



int executeCommand(Command const& cmd) {
  if (cmd.argv.empty() || cmd.argv[0] == "") {
    return 1;
  }

  BuiltinFunc func = g_builtinList.findBuiltinFunc(cmd.argv[0]);
  if (func) {
    return (*func)(cmd.argv);
  }

  return executeSystem(cmd);
}



int executeSystem(Command const& cmd) {
  if (cmd.argv.empty() || cmd.argv[0] == "") {
    return 1;
  }

  pid_t pid;
  int status;

  pid = fork();
  if (! pid) {
    resetSignalHandler();

    char** argv = stringVec2CharDoublePtr(cmd.argv);

    if (execvp(argv[0], argv) == -1) {
      //error handling
    }
  }
  else if (pid > 0) {
    do {
      if (waitpid(pid, &status, WUNTRACED) == -1) {
	//error handling
      }
    } while (! WIFEXITED(status) && ! WIFSIGNALED(status));
  }
  else {
    //error handling				
  }

  return 1;
}



void mainLoop(void) {
  string line;
  vector<Command> commands;
  int status;

  do {
    string curPath = string(getcwd(NULL, 0));
    cout << curPath << ": ";
    fflush(stdout);

    line = readLine();

    parseLine(line, commands);

    for (int i = 0; i < commands.size(); i++) {
      status = executeCommand(commands[i]);
    }

    line.clear();
    commands.clear();
  } while (status >= 0);
}




int main(void) {

  printf("Welcome to S&J's Shell!"
	 " You can type in 'help' to look for some instructions for this shell\n");

  mainLoop();


  return 0;
}
