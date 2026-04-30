#include <bits/stdc++.h>
#include <sys/wait.h>
using namespace std;

//lex it like u mean it
enum class State{
  NORMAL,
  IN_QUOTE,
};

vector<vector<string>> parse(vector<string> t){
  vector<vector<string>> commando_pepprino={};
  vector<string> sts;
      for (auto &to : t) {
        if (to != "|"){
          if (to != ""){
          sts.push_back(to);}
        }
        else{
          commando_pepprino.push_back(sts);
          sts.clear();

        }
    }
      if (!sts.empty()){
      commando_pepprino.push_back(sts);}
      return commando_pepprino;

}

int main(){
  while (1){
    string line;
    cout<<"your wish is my command>>";
    //read input
    getline(cin, line);
    vector<string> tokens;
    string current;
    State s = State::NORMAL;

    for (char c:line){
      if (s==State::NORMAL && c==' ' && !current.empty()){
        tokens.push_back(current);
        current="";
        s=State::NORMAL;
      }
      else if(s==State::NORMAL && c=='"'){
        s=State::IN_QUOTE;
      }
      else if(s==State::IN_QUOTE && c=='"'){
        s=State::NORMAL;
        tokens.push_back(current);
        current="";
      }
      else if(s==State::NORMAL && c=='|' && !current.empty()){
        tokens.push_back(current);
        tokens.push_back(string(1, c));
        s=State::NORMAL;
        current="";
      }
      else if(s==State::NORMAL && c==' '){
        continue;
      }
      else{
        current+=c;
      }

    }
    if (!current.empty()) { tokens.push_back(current);}

    auto cmd = parse(tokens);
    if (tokens[0]=="exit"){
      break;
    }
    else{
     if (cmd.size() == 2){
        int fd[2];
        pipe(fd);

        vector<string> cmd1=cmd[0];
        vector<string> cmd2=cmd[1];

        pid_t p1=fork();

        if (p1==0){
          dup2(fd[1], STDOUT_FILENO);

          close(fd[0]);
          close(fd[1]);

          vector<char*> argv;
          for (auto &s: cmd1){
            argv.push_back(s.data());
          }
          argv.push_back(nullptr);
          execvp(argv[0], argv.data());
        }
        pid_t p2=fork();

        if (p2==0){
          dup2(fd[0], STDIN_FILENO);

          close(fd[0]);
          close(fd[1]);

          vector<char*> argv;
          for (auto &s: cmd2){
            argv.push_back(s.data());
          }
          argv.push_back(nullptr);
          execvp(argv[0], argv.data());
        }

        close(fd[0]);
        close(fd[1]);

         wait(nullptr);
         wait(nullptr);

      }
     else if (cmd.size()==1){

    pid_t p=fork();
    if (p==0){
     vector<char*> argv;
     for (auto &s: cmd[0]){
       argv.push_back(const_cast<char*>(s.c_str()));
     }
     argv.push_back(nullptr);
     execvp(argv[0], argv.data());
     
     }
    wait(nullptr);
     }
     else{
       printf("WORKING ON MORE FEATURES COMMING SOON");
     }

    }
}
return 0;
}


