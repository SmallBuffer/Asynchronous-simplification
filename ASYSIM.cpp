#pragma GCC optimize(2)
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "clang/AST/ASTContext.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <stdbool.h>
#include <string>
#include <vector>
#include<set>
#include <fstream>

// std::vector<std::pair<int, string>> media_file;
using namespace clang::tooling;
using namespace llvm;

// 将自定义类别应用于所有命令行选项，以便仅显示它们。
static cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser使用与编译数据库和输入文件相关的公共命令行选项的描述声明
// HelpMessage。 在所有工具中都有这个帮助消息，这很好。
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// 稍后可以添加此特定工具的帮助消息。
static cl::extrahelp MoreHelp("\nMore help text...\n");

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

struct node {
  string content;
  int begincnt;
  vector<string> endcontent;
  int nex;
  int to;
  node(string a, int b = 0, vector<string> d = vector<string>(0), int c = -1,
       int f = -1)
      : content(a), begincnt(b), endcontent(d), nex(c), to(f) {}
  bool contain(string a) { return content.find(a, 0) != content.npos; }
  vector<string> getparam() {
    vector<string> ans;
    if (this->contain("send(")) {
      int temid = content.find(",", 0);
      if (content.find("(", temid + 1) != content.npos)
      //如果带括号有多个参数
      {
        int temid2 = content.find("(", temid + 1); //"("的下标
        string param = content.substr(
            temid2 + 1, content.find(")", temid2 + 1) - temid2 - 1);
        while (param.find(",", 0) != param.npos) {
          int temid3 = param.find(",", 0);
          ans.push_back(param.substr(0, temid3));
          param = param.substr(temid3 + 1, param.length() - temid3 - 1);
        }
        ans.push_back(param);
      } else {
        int temid2 = content.find(")", temid + 1);
        ans.push_back(content.substr(temid + 1, temid2 - temid - 1));
      }
    } else if (this->contain("<-receive(")) {
      int temid = content.find("<-", 0);
      string param = content.substr(0, temid);
      if (param.find(",", 0) != param.npos) //有多个逗号分隔的参数
      {
        while (param.find(",", 0) != param.npos) {
          int temid3 = param.find(",", 0);
          ans.push_back(param.substr(0, temid3));
          param = param.substr(temid3 + 1, param.length() - temid3 - 1);
        }
        ans.push_back(param);
      } else
        ans.push_back(param);
    }
    return ans;
  }
};
int mark[10000];
string path_file; //结果输出路径
vector<string> source_file, intermediate_structure;
vector<node> P;           //存储所有参与者的语料
vector<int> start_index;  //每个参与者语料的起始下标
vector<int> par_position; //每个参与者语料当前读取到的下标
vector<bool> ismulti;     //标识每个参与者是否有多个
vector<string> par_name;  //参与者名
int stid = -1;
bool iserror = false; //标识语料输入是否有错误
string lasstore = ""; //存储上一次receive内容的变量
stack<string> Loopstmt;
map<string, int> mp;     //迭代变量和迭代集合编号映射
map<string, int> struct_id;  //变量所属结构体编号
map<string, string> trans_var;
int participate_num = 0; //参与者数量
vector<string> result, transf_intermediate;
stack<int> emptyid;
map<int, int> dep;
int mxde = 0;
bool is_in_Loop = 0;
bool is_roundN = 0;
vector<string> lasrecv;
vector<string> head_declare,thread,main_function;
set<string> var_name,include_header, name_space_declare;
stack<int> lasbegincnt;
int firstSenderId;

const int maxn = 1e5;

bool compa(string A, string B);
void link(int parid);
void transformToAssignment(int stid);
int findTheFirstSender();
void output();
void solve();
string trim(string x);
string getVarName(string temp);
string changeVar(string it);
string getParam(int col, int para_id) {
  string temp = source_file[col - 1];
  int id1 = temp.find('(', 0);
  int id2 = temp.find(',', 0);
  int id3 = temp.find(',', id2 + 1);
  if (para_id == 0) {
    temp = temp.substr(id1 + 1, id2 - id1 - 1);
  } else if (para_id == 1) {
    temp = temp.substr(id2 + 1, id3 - id2 - 1);
  }
  if (temp.find('[', 0) != temp.npos)
    temp = temp.substr(0, temp.find('[', 0));
  if(temp.find('(',0)!=temp.npos&&temp.find(')',0)!=temp.npos)
  {
    int k1 = temp.find('(', 0);
    int k2 = temp.find(')', 0);
    temp = temp.substr(0,k1)+temp.substr(k2 + 1, temp.length() - k2 - 1);
  }
  if(temp.find('*',0)!=temp.npos)
  {
    int k = temp.find('*');
    temp = temp.substr(0,k)+temp.substr(k + 1, temp.length() - k - 1);
  }
  if(temp.find('&')!=temp.npos)
  {
    int k = temp.find('&');
    temp = temp.substr(0,k)+temp.substr(k + 1, temp.length() - k - 1);
  }
  int i = 0, j = temp.length() - 1;
  while (temp[i] == ' ' && i <= j)
    i++;
  while (temp[j] == ' ' && i <= j)
    j--;
  return temp.substr(i, j - i + 1);
}
// Declares clang::SyntaxOnlyAction.
string Name = "bufRecv";
StatementMatcher SendMatcher =
    callExpr(callee(functionDecl(hasName("write")))).bind("sendStmt");
StatementMatcher RecvMatcher =
    callExpr(callee(functionDecl(hasName("read")))).bind("recvStmt");
StatementMatcher RelateForMatcher = forStmt().bind("relateForStmt");
StatementMatcher RelateIfMatcher = ifStmt().bind("relateIfStmt");
class SendPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const CallExpr *Send = Result.Nodes.getNodeAs<clang::CallExpr>("sendStmt");
    // const Expr *Arg_0 = Send->getArg(0);
    FullSourceLoc FullLocation = Context->getFullLoc(Send->getBeginLoc());
    int col = FullLocation.getSpellingLineNumber();
    mark[col] = 3;
    string tempname = trim(getParam(col, 1));
    var_name.insert(getVarName(tempname));
  }
};
class RecvPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const CallExpr *Recv = Result.Nodes.getNodeAs<clang::CallExpr>("recvStmt");
    // const Expr *Arg_0 = Recv->getArg(0);
    // const Expr *Arg_1 = Recv->getArg(1);
    FullSourceLoc FullLocation = Context->getFullLoc(Recv->getBeginLoc());
    int col = FullLocation.getSpellingLineNumber();
    mark[col] = 4;
    string tempname = trim(getParam(col, 1));
    var_name.insert(getVarName(tempname));
  }
};
class RelateForPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const ForStmt *RelateFor =
        Result.Nodes.getNodeAs<clang::ForStmt>("relateForStmt");
    if (!RelateFor || !Context->getSourceManager().isWrittenInMainFile(
                          RelateFor->getForLoc()))
      return;
    FullSourceLoc FullLocation_begin =
        Context->getFullLoc(RelateFor->getBeginLoc());
    FullSourceLoc FullLocation_end =
        Context->getFullLoc(RelateFor->getEndLoc());
    int col_begin = FullLocation_begin.getSpellingLineNumber();
    int col_end = FullLocation_end.getSpellingLineNumber();

    if(source_file[col_begin-1].find("round",0)!=source_file[col_begin-1].npos)
    {
      if(is_roundN)return;
      is_roundN = 1;
    }
    for (int i = col_begin; i <= col_end; i++) {
      
      if (mark[i]) {
        mark[col_begin] = 2;
        mark[col_end] = 5;
        break;
      }
    }
  }
};
class RelateIfPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const IfStmt *RelateIf =
        Result.Nodes.getNodeAs<clang::IfStmt>("relateIfStmt");
    if (!RelateIf || !Context->getSourceManager().isWrittenInMainFile(
                          RelateIf->getIfLoc()))
      return;
    FullSourceLoc FullLocation_begin =
        Context->getFullLoc(RelateIf->getBeginLoc());
    FullSourceLoc FullLocation_end =
        Context->getFullLoc(RelateIf->getEndLoc());
    int col_begin = FullLocation_begin.getSpellingLineNumber();
    int col_end = FullLocation_end.getSpellingLineNumber();
    for (int i = col_begin; i <= col_end; i++) {
      if (mark[i]) {
        for (int j = col_begin; j <= col_end; j++)
        {
          if(mark[j]==0)
            mark[j] = 1;
        }
        break;
      }
    }
  }
};
class RelateVarDeclPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const DeclRefExpr *RelateVarDecl =
        Result.Nodes.getNodeAs<clang::DeclRefExpr>("varDeclRef");
    const VarDecl *RelateVar =
        Result.Nodes.getNodeAs<clang::VarDecl>("relateVar");
    if (!RelateVarDecl || !Context->getSourceManager().isWrittenInMainFile(
                              RelateVarDecl->getBeginLoc()))
      return;
    if (!RelateVar || !Context->getSourceManager().isWrittenInMainFile(
                          RelateVar->getBeginLoc()))
      return;
    FullSourceLoc FullLocation =
        Context->getFullLoc(RelateVarDecl->getBeginLoc());
    int col = FullLocation.getSpellingLineNumber();
    if (mark[col] == 0)
      mark[col] = 1;
    FullLocation = Context->getFullLoc(RelateVar->getBeginLoc());
    col = FullLocation.getSpellingLineNumber();
    if (mark[col] == 0)
      mark[col] = 6;
  }
};

int main(int argc, const char **argv) {
  path_file = "result.cpp"; //默认输出路径
  for (int counts = 0; counts < argc;++counts)
  {
    if(strcmp(argv[counts],"-p")==0)
    {
      path_file = argv[counts + 1];
      for (int i = counts; i <argc-2; ++i)
      {
        argv[i] = argv[i + 2];
      }
        argc -= 2;
        counts -= 2;
    }
    
  }
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  const vector<string> sourceList = OptionsParser.getSourcePathList();

  bool badcase1 = 0;
  bool is_equal = 1;
  //建图
  string mc, nc;
  //获取参与者数量
  participate_num = sourceList.size();
  //获取参与者名
  ismulti.resize(participate_num);
  for (auto it : sourceList) {
    int id = it.rfind("/");
    string filename = it.substr(id + 1, it.find('.', id) - id - 1);
    par_name.push_back(filename);
  }

  string struct_name[3] = {"P", "Q", "M"};
  string iterator_name[3] = {"p", "q", "m"};
  vector<int> insert_line_id;
  for (int j = 0; j < participate_num; ++j) {
    ClangTool Tool(OptionsParser.getCompilations(), sourceList[j]);
    fstream file;
    file.open(sourceList[j]);
    string temp;
    while (getline(file, temp))
      source_file.push_back(temp);

    int sz = source_file.size();
    for (int i = 0; i <= sz + 5; ++i)
      mark[i] = 0;
    SendPrinter sendPrinter;
    RecvPrinter recvPrinter;
    RelateForPrinter relateForPrinter;
    RelateVarDeclPrinter relateVarDeclPrinter;
    RelateIfPrinter relateIfPrinter;
    MatchFinder Finder1, Finder2,Finder3;
    Finder1.addMatcher(SendMatcher, &sendPrinter);
    Finder1.addMatcher(RecvMatcher, &recvPrinter);
    Tool.run(newFrontendActionFactory(&Finder1).get());

    for (auto var : var_name) 
      struct_id[var] = j+1;
    for(auto temp:source_file){
      if(temp.find("=",0)!=temp.npos&&temp.find("==",0)==temp.npos&&temp.find("for")==temp.npos&&temp.find("if")==temp.npos){
        int id = temp.find('=', 0);
        string lvar = temp.substr(0, id);
        string rvar = temp.substr(id + 1, temp.length() - id - 1);
        lvar = getVarName(trim(lvar));
        rvar = getVarName(trim(rvar));
        if((struct_id[lvar]==0&&struct_id[rvar]==0)||(lvar[0]=='\"'&&lvar[lvar.length()-1]=='\"')||(rvar[0]=='\"'&&rvar[rvar.length()-1]=='\"'))
          continue;
        if(lvar=="r"||lvar=="i"||lvar=="j"||lvar=="k"||rvar=="r"||rvar=="i"||rvar=="j"||rvar=="k")
          continue;
        var_name.insert(lvar);
        var_name.insert(rvar);
      }
    }
    for (auto var : var_name) {
      struct_id[var] = j+1;
      StatementMatcher RelateVarDecl =
          declRefExpr(to(varDecl(hasName(var)).bind("relateVar")))
              .bind("varDeclRef");
      Finder2.addMatcher(RelateVarDecl, &relateVarDeclPrinter);
    }
    Finder3.addMatcher(RelateForMatcher, &relateForPrinter);
    Finder3.addMatcher(RelateIfMatcher, &relateIfPrinter);
    Tool.run(newFrontendActionFactory(&Finder2).get());
    Tool.run(newFrontendActionFactory(&Finder3).get());

    
    //生成结构体声明首部
    bool has_iter = false;
    string las_recvfrom,las_sendto;
    head_declare.push_back("struct " + par_name[j] + "{");
    for (int i = 1; i <= sz; ++i)
        if (mark[i] == 1)
        {
          int k;
          for (k = 0; k < source_file[i - 1].length(); ++k)
            if (source_file[i - 1][k] != ' ')
              break;
          intermediate_structure.push_back(source_file[i - 1].substr(k, source_file[i - 1].length() - k));
        } else if (mark[i] == 2) {
        intermediate_structure.push_back("Loop_Begin("+iterator_name[!j]+":" + par_name[!j] + ")");
        has_iter = true;
      } else if (mark[i] == 3) {
        string name=(has_iter?iterator_name[!j]:par_name[!j]);
        int id1 = source_file[i - 1].find("(");
        int id2 = source_file[i - 1].find(",", id1 + 1);
        las_sendto = trim(source_file[i - 1].substr(id1 + 1, id2 - id1 - 1));
        if(has_iter)
        {
          if(las_sendto!=las_recvfrom&&las_recvfrom!="")
            is_equal = false;
        }
        intermediate_structure.push_back("send("+name+"," + getParam(i, 1) + ")");
      } else if (mark[i] == 4) {
        string name=(has_iter?iterator_name[!j]:par_name[!j]);
        intermediate_structure.push_back(getParam(i, 1) + "<-receive("+name+")");
        int id1 = source_file[i - 1].find("(");
        int id2 = source_file[i - 1].find(",", id1 + 1);
        las_recvfrom = trim(source_file[i - 1].substr(id1+1,id2-id1-1));
        if(has_iter)
        {
          if(las_sendto!=las_recvfrom&&las_sendto!="")
            is_equal = false;
        }
      } else if (mark[i] == 5) {
        intermediate_structure.push_back("Loop_End");
        has_iter = false;
      } else if (mark[i] == 6) {
        head_declare.push_back("\t"+trim(source_file[i - 1]));//生成结构体内部成员定义
      } else{
        if(source_file[i-1].find("#include")!=source_file[i-1].npos)
        {
          string unuse_header[] = {"unistd", "arpa/inet", "sys/socket", "iostream", "pthread", "cstring","netinet","assert"};
          bool flag = 0;
          for (int k = 0; k < 8;++k)
          {
            if(source_file[i-1].find(unuse_header[k],0)!=source_file[i-1].npos)
            {
              flag = 1;
              break;
            }
          }
          if(flag==0)
            include_header.insert(source_file[i - 1]);
        }
        if(source_file[i-1].find("assert(")!=source_file[i-1].npos)
        {
          intermediate_structure.push_back(trim(source_file[i - 1]));
        }
        if(source_file[i-1].find("using namespace")!=source_file[i-1].npos)
        {
          int k;
          for (k = 0; k < source_file[i - 1].length(); ++k)
            if (source_file[i - 1][k] != ' ')
              break;
          name_space_declare.insert(source_file[i - 1].substr(k, source_file[i - 1].length() - k));
        }
      }

    insert_line_id.push_back(head_declare.size());
    head_declare.push_back("");

    file.close();
    source_file.clear();
    var_name.clear();
    if (j != participate_num - 1)
      intermediate_structure.push_back("||");


  }
  // for(auto it:intermediate_structure)
  //   cout << it << endl;
  // puts("-------------------------");


  int lasto = -1;
  int loopheadnum = 0; //循环头数
  for (auto mc : intermediate_structure) {
    if (mc == "||" || start_index.size() == 0) {
      start_index.push_back(P.size());
      par_position.push_back(P.size());
      loopheadnum = 0;
    }
    if (mc == "||")
      continue;
    if (mc.find("Loop_Begin", 0) != mc.npos) {
      is_in_Loop = 1;
      int id1 = mc.find("(", 0);
      int id2 = mc.find(":", 0);
      int id3 = mc.find(")", 0);
      string key = mc.substr(id1 + 1, id2 - id1 - 1);
      string value = mc.substr(id2 + 1, id3 - id2 - 1);
      // cout<<key<<" "<<value<<endl;
      for (int i = 0; i < participate_num; i++) {
        if (value == par_name[i]) {
          mp[key] = i + 1;
          ismulti[i] = 1;
          break;
        }
      }
      loopheadnum++;
      Loopstmt.push(mc);
      continue;
    } else if (mc == "Loop_End" && P.size()) {
      is_in_Loop = 0;
      lasrecv.clear();
      P.back().endcontent.push_back(Loopstmt.top());
      string loophead = Loopstmt.top();
      int id1 = loophead.find("(", 0);
      int id2 = loophead.find(":", 0);
      string key = loophead.substr(id1 + 1, id2 - id1 - 1);
      Loopstmt.pop();
      mp[key] = 0;
      continue;
    }
    node newm = node(mc);
    newm.begincnt = loopheadnum;
    loopheadnum = 0;
    if (newm.contain("send(")) {
      int id1 = newm.content.find("(", 0);
      int id2 = newm.content.find(",", 0);
      string temp = newm.content.substr(id1 + 1, id2 - id1 - 1);
      if (is_in_Loop && lasrecv.size()&&is_equal==false) {
        badcase1 = 1;
        for (auto I : lasrecv) {
          if (I == temp) {
            badcase1 = 0;
            break;
          }
        }
        if (badcase1)
          iserror = true;
      }
      if (mp[temp] == 0) {
        for (int i = 0; i < participate_num; i++) {
          if (temp == par_name[i]||temp == iterator_name[i]) {
            newm.to = i;
            break;
          }
        }
      } else
        newm.to = mp[temp] - 1;
    } else if (newm.contain("<-receive(")) {
      vector<string> tempar = newm.getparam();
      if (is_in_Loop)
        lasrecv.push_back(tempar[0]);
      for (auto it : tempar) {
        if (it == "id") {
          int id1 = newm.content.find("(", 0);
          int id2 = newm.content.find(")", id1);
          string recvfrom = newm.content.substr(id1 + 1, id2 - id1 - 1);
          if (mp[recvfrom])
            mp["id"] = mp[recvfrom];
          for (int i = 0; i < participate_num; i++) {
            if (recvfrom == par_name[i]) {
              mp["id"] = i + 1;
              break;
            }
          }
        }
      }
    }
    if (P.size() && P.size() != start_index.back())
      P.back().nex = P.size();
    P.push_back(newm);
  }
  for (int j = 0; j < participate_num;++j)
  {
    if(ismulti[j])
      head_declare[insert_line_id[j]] = "}" + struct_name[j] + "[100];";
    else
      head_declare[insert_line_id[j]] = "}" + struct_name[j] + ";";
  }
  start_index.push_back(P.size());
  //串联语句并转化成赋值
  solve();
  //有错误
  
  if (iserror) {
 
    if (badcase1)
      puts("Error:Communicate with different qs in Q in one iteration!");
  } else { 
    if(ismulti[0]&&ismulti[1])
      head_declare.push_back("struct Pair{\n\tint id_p;\n\tint id_q;\n};\n");
    transformToAssignment(start_index[firstSenderId]);
    //输出转化后结果
    output();
    //生成主函数
    main_function.push_back("int main()\n{\n");
    int loop_round = 0;
    int atomic_block_num = 0;
    bool in_thread = false;
    string iter_var[4] = {"r","i", "j", "k"};
    string Tab = "\t";
    if(is_roundN==0)
      loop_round++;
    for(auto it:transf_intermediate)
    {
      if(it.find("Atomic_Begin",0)!=it.npos)
      {
        atomic_block_num++;
        if(in_thread==false)
        {
          main_function.push_back(Tab+"for( int " + iter_var[loop_round] + " ; " + iter_var[loop_round] + " < 100 ; ++" + iter_var[loop_round] + ")\n" + Tab + "{\n");
          loop_round++;
          Tab += "\t";
          in_thread = true;
          
          if(ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n" + Tab + "Pair par;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_p = ((Pair *)par)->id_p;\n\tint id_q = ((Pair *)par)->id_q;\n");
            main_function.push_back(Tab+"par.id_p = "+iter_var[loop_round-2]+";\n"+Tab+"par.id_q = "+iter_var[loop_round-1]+";\n"+Tab+"ret = pthread_create(&id, NULL, Thread, &par);\n"+Tab+"pthread_join(id, NULL);\n");
          }
          else if(!ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_q = *((int *)par);\n");
            main_function.push_back(Tab+"ret = pthread_create(&id, NULL, Thread, &"+iter_var[loop_round-1]+");\n"+Tab+"pthread_join(id,NULL);\n");
          }
        }
        thread.push_back("\twhile (mutex" + to_string(atomic_block_num) + " == true);\n\tmutex" + to_string(atomic_block_num) + " = true;\n");
        head_declare.push_back("bool mutex" + to_string(atomic_block_num) + " = false;\n");

      }
      else if(it.find("Atomic_End",0)!=it.npos)
      {
        thread.push_back("\tmutex"+to_string(atomic_block_num)+" = false;\n");
      }
      else if(it.find("Loop_Begin",0)!=it.npos)
      {
        if(iter_var[loop_round]=="r")
        {
          main_function.push_back(Tab+"for( r=0 ; r < 100 ; ++r)\n" + Tab + "{\n");
          head_declare.push_back("int r;\n");
        }
        
        else
        main_function.push_back(Tab+"for( int " + iter_var[loop_round] + " ; " + iter_var[loop_round] + " < 100 ; ++" + iter_var[loop_round] + ")\n" + Tab + "{\n");
        loop_round++;
        Tab += "\t";
      }
      else if(it.find("Loop_End",0)!=it.npos)
      {
        if(in_thread)
        {
          in_thread = false;
          Tab.pop_back();
          main_function.push_back(Tab + "}\n");
          loop_round--;
        }
        Tab.pop_back();
        main_function.push_back(Tab + "}\n");
        loop_round--;
      }
      else if(it.find('=',0)!=it.npos&&it.find("==",0)==it.npos)
      {
        if(in_thread==false)
        {
          main_function.push_back(Tab+"for( int " + iter_var[loop_round] + " ; " + iter_var[loop_round] + " < 100 ; ++" + iter_var[loop_round] + ")\n" + Tab + "{\n");
          loop_round++;
          Tab += "\t";
          in_thread = true;
          
          if(ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n" + Tab + "Pair par;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_p = ((Pair *)par)->id_p;\n\tint id_q = ((Pair *)par)->id_q;\n");
            main_function.push_back(Tab+"par.id_p = "+iter_var[loop_round-2]+";\n"+Tab+"par.id_q = "+iter_var[loop_round-1]+";\n"+Tab+"ret = pthread_create(&id, NULL, Thread, &par);\n"+Tab+"pthread_join(id, NULL);\n");
          }
          else if(!ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_q = *((int *)par);\n");
            main_function.push_back(Tab+"ret = pthread_create(&id, NULL, Thread, &"+iter_var[loop_round-1]+");\n"+Tab+"pthread_join(id,NULL);\n");
          }
        }
        int id = it.find('=', 0);
        string lvar = it.substr(0, id);
        string rvar = it.substr(id + 1, it.length() - id - 1);
        string lmember = "";
        string rmember = "";
        lvar = trim(lvar);
        rvar = trim(rvar);
        if(lvar.find('.',0)!=lvar.npos)
        {
          int did = lvar.find('.', 0);
          lmember=lvar.substr(did,lvar.length()-did);
          lvar=lvar.substr(0, did);
        }
          
        if(rvar.find('.',0)!=rvar.npos)
        {
          int did = rvar.find('.', 0);
          rmember=rvar.substr(did,rvar.length()-did);
          rvar=rvar.substr(0, did);
        }
         if(lvar.find('[',0)!=lvar.npos)
        {
          int did = lvar.find('[', 0);
          lmember=lvar.substr(did,lvar.length()-did)+lmember;
          lvar=lvar.substr(0, did);
        }
          
        if(rvar.find('[',0)!=rvar.npos)
        {
          int did = rvar.find('[', 0);
          rmember=rvar.substr(did,rvar.length()-did)+rmember;
          rvar=rvar.substr(0, did);
        }
        lmember=changeVar(lmember);
        rmember = changeVar(rmember);
        lvar=getVarName(lvar);
        rvar = getVarName(rvar);
        string temlvar = lvar;
        string temrvar = rvar;
        if(rvar=="i")
          rvar = "id_q", trans_var["i"] = "id_q";
        else if(rvar=="j")
          rvar = "id_p", trans_var["j"] = "id_p";
        if(struct_id[lvar])
        {
          int k = struct_id[lvar];
          if(ismulti[k-1])
            {
              if(in_thread)
                lvar = struct_name[k-1] + "[" + (k==1?"id_p":"id_q") + "]." + lvar;
              else
                lvar = struct_name[k-1] + "[" + iter_var[loop_round-2+k] + "]." + lvar;
            }
            else
            lvar = struct_name[k-1] + "." + lvar;
        }
        string x;
        trans_var[temlvar] = lvar;
        trans_var[temrvar] = rvar;
        if(struct_id[rvar])
        {
          int k = struct_id[rvar];
          if(ismulti[k-1])
          {
            if(in_thread)
              rvar = struct_name[k-1] + "[" + (k==1?"id_p":"id_q") + "]." + rvar;
            else
              rvar = struct_name[k-1] + "[" + iter_var[loop_round-2+k] + "]." + rvar;
          }
          else
          rvar = struct_name[k-1] + "." + rvar;
        }
        x = lvar + lmember + " = " + rvar + rmember + ";";
        if(in_thread)
          thread.push_back("\t"+x);
        else
          main_function.push_back(Tab + x);
      }
      else if(it.find("assert(")!=it.npos||it.find("if(")!=it.npos||it.find("else if(")!=it.npos)
      {
        string tem = changeVar(it);
        if(in_thread)
          thread.push_back("\t"+tem);
        else
          main_function.push_back(Tab + tem);
      }
      else
      {
        if(in_thread==false)
        {
          main_function.push_back(Tab+"for( int " + iter_var[loop_round] + " ; " + iter_var[loop_round] + " < 100 ; ++" + iter_var[loop_round] + ")\n" + Tab + "{\n");
          loop_round++;
          Tab += "\t";
          in_thread = true;
          
          if(ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n" + Tab + "Pair par;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_p = ((Pair *)par)->id_p;\n\tint id_q = ((Pair *)par)->id_q;\n");
            main_function.push_back(Tab+"par.id_p = "+iter_var[loop_round-2]+";\n"+Tab+"par.id_q = "+iter_var[loop_round-1]+";\n"+Tab+"ret = pthread_create(&id, NULL, Thread, &par);\n"+Tab+"pthread_join(id, NULL);\n");
          }
          else if(!ismulti[0]&&ismulti[1]){
            main_function.push_back(Tab + "pthread_t id;\n" + Tab + "int ret = 0;\n");
            thread.push_back("void* Thread(void * par)\n{\n\tint id_q = *((int *)par);\n\twhile (mutex"+to_string(atomic_block_num)+" == true);\n\tmutex"+to_string(atomic_block_num)+" = true;\n");
            main_function.push_back(Tab+"ret = pthread_create(&id, NULL, Thread, &"+iter_var[loop_round-1]+");\n"+Tab+"pthread_join(id,NULL);\n");
          }
        }
        if(in_thread)
          thread.push_back("\t"+it);
        else
          main_function.push_back(Tab + it);
      }
    }
    if(in_thread)
    {
       in_thread = false;
        Tab.pop_back();
        main_function.push_back(Tab + "}\n");
        loop_round--;
    }
    thread.push_back("\treturn 0;\n}\n");
    main_function.push_back("\treturn 0;\n}\n");
    include_header.insert("#include <iostream>");
    include_header.insert("#include <pthread.h>");
    include_header.insert("#include <cstring>");
    include_header.insert("#include <assert.h>");

    ofstream in(path_file);
    for (auto it : include_header)
      in
          << it << endl;
    for (auto it : name_space_declare)
      in
          << it << endl;
    for(auto it:head_declare)
      in << it<<endl;
    for(auto it:thread)
      in << it<<endl;
    for(auto it:main_function)
      in << it<<endl;
  }
}
bool compa(string A, string B) {
  int id1 = A.find(":", 0);
  int id2 = A.find(")", 0);
  string value1 = A.substr(id1 + 1, id2 - id1 - 1);
  int id3 = B.find(":", 0);
  int id4 = B.find(")", 0);
  string value2 = B.substr(id3 + 1, id4 - id3 - 1);
  int idA = 0, idB = 0;
  int cnt = 0;
  for (auto it : par_name) {
    if (value1 == it)
      idA = cnt;
    if (value2 == it)
      idB = cnt;
    cnt++;
  }
  return idA > idB;
}
void link(int parid) {
  int sid = par_position[parid];
  if (sid >= start_index[parid + 1])
    return;
  //找到当前参与者中的send语句
  while (sid < start_index[parid + 1] && P[sid].contain("send(") == 0)
    sid++;
  if (sid >= start_index[parid + 1])
    return;

  //找到该send语句的发送目标
  int destination = P[sid].to;
  par_position[parid] = sid + 1;
  int rid = par_position[destination];

  // cout << P[sid].content << " " << P[sid].to << endl;
  // cout << rid << " " << destination << " " << start_index[destination + 1] << endl;

  //找到send语句对应的receive语句
  while (rid < start_index[destination + 1] && P[rid].contain("<-receive(") == 0)
    rid++;
  if (rid >= start_index[destination + 1]) {
    puts("Send without Receive!");
    iserror = 1;
    return;
  }
  //连接
  P[sid].nex = rid;

  //上传recive语句的循环头至send
  if (P[rid].begincnt && ismulti[destination]) {
    P[sid].begincnt += P[rid].begincnt;
    P[rid].begincnt = 0;
  }

  if (ismulti[parid]) {
    //下传send语句的循环尾
    int temrid = rid;
    while (temrid < start_index[destination + 1] &&
           P[temrid].contain("send(") == 0 && P[temrid].endcontent.size() == 0 &&
           P[temrid].begincnt == 0)
      temrid++;
    if (temrid >= start_index[destination + 1] || P[temrid].contain("send(") ||
        P[temrid].begincnt && temrid > rid)
      temrid--;
    P[temrid].endcontent.insert(P[temrid].endcontent.begin(),
                                P[sid].endcontent.begin(),
                                P[sid].endcontent.end());
    P[sid].endcontent.clear();
  }

  par_position[destination] = rid + 1;
  link(destination);
}
void transformToAssignment(int stid) {
  while (stid != -1) {
    if (P[stid].contain("send(")) {
      int nx = P[stid].nex;
      if (nx == -1 || P[nx].contain("<-receive(") == 0) {
        cout << "Wrong Answer!place2\n";
        exit(0);
      }
      vector<string> sendpar = P[stid].getparam();
      vector<string> recvpar = P[nx].getparam();
      int sz1 = sendpar.size();
      if (sz1 != recvpar.size()) {
        cout << "Wrong Answer!place3\n";
        exit(0);
      }
      if (P[stid].begincnt) {
        int num = P[stid].begincnt;
        lasbegincnt.push(num);
        while (num--) {
          emptyid.push((int)result.size());
          result.push_back("");
        }
      }
      if (P[nx].begincnt) {
        int num = P[nx].begincnt;
        lasbegincnt.push(num);
        while (num--) {
          emptyid.push((int)result.size());
          result.push_back("");
        }
      }
      for (int i = 0; i < sz1; ++i) {
        string recv = recvpar[i];
        string send = sendpar[i];
        string ass = recvpar[i] + "=" + sendpar[i];
        result.push_back(ass);
      }
      if (P[nx].endcontent.size() > 0) {
        std::sort(P[nx].endcontent.begin(),
                  P[nx].endcontent.begin() + lasbegincnt.top(), compa);
        lasbegincnt.pop();
        for (auto it : P[nx].endcontent) {
          if (emptyid.empty()) {
            cout << "Wrong Answer!place1!\n";
            exit(0);
          }
          result[emptyid.top()] = it;
          int desz = emptyid.size();
          dep[emptyid.top()] = desz;
          mxde = max(mxde, (int)emptyid.size());
          emptyid.pop();
          dep[result.size()] = desz;
          result.push_back("Loop_End");
        }
      }
      if (P[stid].endcontent.size() > 0) {
        std::sort(P[stid].endcontent.begin(),
                  P[stid].endcontent.begin() + lasbegincnt.top(), compa);
        lasbegincnt.pop();
        for (auto it : P[stid].endcontent) {
          if (emptyid.empty()) {
            cout << "Wrong Answer!place1!\n";
            exit(0);
          }
          result[emptyid.top()] = it;
          int desz = emptyid.size();
          dep[emptyid.top()] = desz;
          mxde = max(mxde, (int)emptyid.size());
          emptyid.pop();
          dep[result.size()] = desz;
          result.push_back("Loop_End");
        }
      }
      stid = nx;
    } else if (P[stid].contain("<-receive("))
      ;
    else {
      if (P[stid].begincnt) {
        int num = P[stid].begincnt;
        lasbegincnt.push(num);
        while (num--) {
          emptyid.push((int)result.size());
          result.push_back("");
        }
      }
      result.push_back(P[stid].content);
      if (P[stid].endcontent.size() > 0) {
        std::sort(P[stid].endcontent.begin(),
                  P[stid].endcontent.begin() + lasbegincnt.top(), compa);
        lasbegincnt.pop();
        for (auto it : P[stid].endcontent) {
          if (emptyid.empty()) {
            cout << "Wrong Answer!place1!\n";
            exit(0);
          }
          result[emptyid.top()] = it;
          int desz = emptyid.size();
          dep[emptyid.top()] = desz;
          mxde = max(mxde, (int)emptyid.size());
          emptyid.pop();
          dep[result.size()] = desz;
          result.push_back("Loop_End");
        }
      }
    }

    stid = P[stid].nex;
  }
}
int findTheFirstSender() {
  int id = -1;
  bool flag = 0;
  for (int i = 0; i < start_index.size() - 1; ++i) {
    for (int j = start_index[i]; j < start_index[i + 1]; ++j) {
      if (P[j].contain("send(")) {
        if (id == -1) {
          id = i;
          break;
        } else {
          flag = 1;
          puts(
              "Error:Repeated sending and receiving!Try to modify as follows:");
        }
      }
      if (P[j].contain("<-receive(")) {
        if (flag) //出现SSRR情况
        {
          // SSRR纠正处理
          int endid = start_index[i + 1] - 1;
          int temj = j - start_index[i];
          while (temj--) {
            node tem = P[start_index[i]];
            for (int k = start_index[i]; k < endid; ++k)
              P[k] = P[k + 1];
            P[endid] = tem;
          }
          for (int T = start_index[i]; T < start_index[i + 1] - 1; ++T) {
            P[T].nex = T + 1;
          }
          // SSRR处理结束
        }
        break;
      }
    }
  }
  if (id == -1) {
    iserror = true;
    puts("Can not find the first sender!"); //没有第一个发送的参与者
  }
  return id;
}
void output() {
  int cnt = 0;
  int lit = -1;
  string atomic;
  for (auto it : result) {
    if (it.find("Loop_Begin", 0) != it.npos && dep[cnt] == mxde) {
      int id1 = it.find("(", 0);
      int id2 = it.find(")", id1);
      atomic = "||" + it.substr(id1, id2 - id1 + 1);
      for (int i = 0;; i++) {
        if (result[i].find("Loop_Begin", 0) != result[i].npos &&
            result[i] != "Loop_Begin(r:R)") {
          lit = dep[i];
          break;
        }
      }
      break;
    }
    cnt++;
  }
  cnt = 0;
  bool F = 0;
  if (result[0].find("Loop_Begin", 0) == result[0].npos) {
    lit = -2;
    transf_intermediate.push_back(atomic + "{");
    F = 1;
  }
  for (auto it : result) {
    if (it.find("Loop_Begin", 0) != it.npos) {
      string temp = "";
      if (dep[cnt] == lit)
        temp += atomic;
      if (dep[cnt] == mxde)
        temp += "Atomic_Begin";
      else
        temp += it;
      transf_intermediate.push_back(temp);
    } else if (it.find("Loop_End", 0) != it.npos && dep[cnt] == mxde) {
      transf_intermediate.push_back("Atomic_End");
      if (cnt + 1 < result.size() &&
          result[cnt + 1].find("Loop_Begcd biin", 0) != it.npos &&
          dep[cnt + 1] == mxde)
        transf_intermediate.push_back(";");
    } else
      transf_intermediate.push_back(it);
    cnt++;
  }
  if (F)
    transf_intermediate.push_back("}");
  // for(auto it:transf_intermediate)
  //   cout << it << endl;
  // puts("---------------------------");
}
void solve() {
  firstSenderId = findTheFirstSender();
  if (firstSenderId == -1)
    return;
  link(firstSenderId);
  if (iserror)
    return;
}
string trim(string x){
  int len = x.length();
   int L;
    for (L = 0; L < len; ++L)
      if (x[L] != ' ')
        break;
    int R;
    for (R=len-1; R >= 0;R--)
      if (x[R] != ' ')
        break;
    return x.substr(L, R - L + 1);
}
string getVarName(string temp){
  int L = -1;
  int R = temp.length();
  if(temp.find('&',0)!=temp.npos)
  L=max(L,(int)temp.find('&',0));
  if(temp.find('*',0)!=temp.npos)
  L=max(L,(int)temp.find('*',0));
  if(temp.find(' ',0)!=temp.npos)
  L=max(L,(int)temp.find(' ',0));
  if(temp.find('[',0)!=temp.npos)
  R=min(R,(int)temp.find('[',0));
  if(temp.find('.',0)!=temp.npos)
  R=min(R,(int)temp.find('.',0));
  if (temp.find(';', 0) != temp.npos)
    R = min(R, (int)temp.find(';', 0));
  return trim(temp.substr(L + 1, R - L - 1));
}
string changeVar(string it)
{
  int len=it.length();
  int l,r;
  string x,tem;
  tem = it;
  for (l=0; l <len;++l)
  {
    for (r = l; r < len; ++r)
    {
      x = it.substr(l, r - l + 1);
      if(trans_var[x]!=""){
        if(l>0&&isalpha(it[l-1]))
          continue;
        if(r<len-1&&isalpha(it[r+1]))
          continue;
        l += tem.length() - len;
        r += tem.length() - len;
        tem = tem.substr(0, l) + trans_var[x] + tem.substr(r + 1, tem.length() - r - 1);
      }
    }
  }
  return tem;
}