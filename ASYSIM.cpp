#include<bits/stdc++.h>
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
#include <utility>
#include <set>
#include <fstream>

//DEBUG
// #define DEBUG
// #ifdef DEBUG
// #define debug(...) printf(_VA_ARGS_)
// #else
// #define debug(...)
// #endif

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

int depth = 0; //循环层数
int participate_num = 0; //对象类型数量
int main_function_begin = 0;

string tag1="// participator-";
string tag2="// Iterate over multiple-"; 
string tag3="// Repeat Round";  
string tag4="// id-";

vector<pair<int,int> >func_bound; //对象线程函数的行号范围
vector<string> file_name;    //待处理的文件名
vector<string> par_name;    //参与者名
vector<string> source_file; //输入源代码
vector<vector<string>> cutted_file; //裁剪结果
map<string, int> parname_to_id;
vector<set<string> >ref_list;
set<string> var_name;
map<string, vector<int> > var_occur_col,var_decl_col;
map<string, string> F;
map<string, bool> remain_var;
set<string> unuse_var;
unordered_map<int, int> mark;

//用结点结构体表示中间结构中的每一句语句，最后的结构用节点链表串联实现

struct node
{
	int loop_depth;
	string tex;
	node * nex;
	node * pre;
	node(int depth,string contant):loop_depth(depth),tex(contant),nex(NULL),pre(NULL){}
	bool contain(string x);
};
vector<node *> cur_node_id;
vector<int> sendrecv_col;
vector<int> participate_col;


string trim(string x);
string GetF(string x);
void Init();
void Link(string a,string b );
//简化send和recv
void SimSendRecv();
void Cut(ClangTool Tool);

string toUpperCase(string s);
string toLowerCase(string s);

//生成各参与者的节点序列
void GenerateNodeList(int participate_num);
int GetDestination(node * x);
//串联合并节点序列成一个节点链表
node * LinkNode(int participate_num);

//定义抽象语法树匹配器
StatementMatcher SendMatcher =
callExpr(callee(functionDecl(hasName("Send")))).bind("sendStmt");
StatementMatcher RecvMatcher =
callExpr(callee(functionDecl(hasName("Recv")))).bind("recvStmt");
StatementMatcher RelateForMatcher = forStmt().bind("relateForStmt");
StatementMatcher RelateIfMatcher = ifStmt().bind("relateIfStmt");
DeclarationMatcher VarDeclMatcher =namedDecl(anyOf(functionDecl(isDefinition()), fieldDecl(),varDecl(isDefinition()), enumConstantDecl(),labelDecl(), typedefDecl(),enumDecl(isDefinition()),recordDecl(isDefinition()))).bind("varDecl");

DeclarationMatcher FuncDeclMatcher = functionDecl(isDefinition()).bind("func");

class SendPrinter : public MatchFinder::MatchCallback {
	public:
		virtual void run(const MatchFinder::MatchResult &Result) {
			ASTContext *Context = Result.Context;
			const CallExpr *Send = Result.Nodes.getNodeAs<clang::CallExpr>("sendStmt");
			// const Expr *Arg_0 = Send->getArg(0);
			FullSourceLoc FullLocation = Context->getFullLoc(Send->getBeginLoc());
			int col = FullLocation.getSpellingLineNumber();
			mark[col] = 1;
			sendrecv_col.push_back(col);
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
			mark[col] = 10;
			sendrecv_col.push_back(col);
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

			for (int i = col_begin; i <= col_end; i++) {

				if (mark[i]) {
					break;
				}
				if(i == col_end) return; //说明不是关键语句，故不做处理
			}
			//接下来进行简化循环

			if(source_file[col_begin-1].find(tag2)!= source_file[col_begin-1].npos)
			{
				string temp = source_file[col_begin-1];

				int id = temp.find(tag2);
				string name = temp.substr(id+tag2.length(),temp.length()-id-tag2.length());

				source_file[col_begin-1] = "Loop_Begin("+name+")";
				source_file[col_end-1] = "Loop_End("+name+")";
				mark[col_begin] = 3;
				mark[col_end] = 3;
			}
			//else if(source_file[col_begin].find(tag3)!= temp.npos)
			//{

			//}
			else
			{
				//没有任何标记则为普通循环，只保留循环头尾
				const Stmt *ForBody = RelateFor->getBody();
				FullLocation_begin =
					Context->getFullLoc(ForBody->getBeginLoc());
				FullLocation_end =
					Context->getFullLoc(ForBody->getEndLoc());
				int body_begin = FullLocation_begin.getSpellingLineNumber();
				int body_end = FullLocation_end.getSpellingLineNumber();

				for(int i = col_begin; i<body_begin;++i){
					mark[i]=3;

				}
				for(int i = body_end+1; i<=col_end;++i){
					mark[i]=3;
				}

			}


		}
};
/*
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
   */
class VarDeclPrinter : public MatchFinder::MatchCallback {
	public:
		virtual void run(const MatchFinder::MatchResult &Result) {
			ASTContext *Context = Result.Context;
			const NamedDecl *VarDecl =
				Result.Nodes.getNodeAs<clang::VarDecl>("varDecl");


			if (!VarDecl || !Context->getSourceManager().isWrittenInMainFile(
						VarDecl->getBeginLoc()))
				return;

			FullSourceLoc FullLocation =
				Context->getFullLoc(VarDecl->getBeginLoc());
			int col = FullLocation.getSpellingLineNumber();
			//只考虑主函数外
			if(col>=func_bound[0].first&&col<=func_bound[0].second)return;
			// if(col <= main_function_begin)
			// 	return;
			SourceLocation SL = VarDecl->getLocation();
			std::string Name = VarDecl->getNameAsString ();
			if(Name == "")
				return;
			for(auto it:unuse_var)
			{
				if(it == Name)
					return;
			}
			var_name.insert(Name);
			var_occur_col[Name].push_back(col);
			var_decl_col[Name].push_back(col);
		}
};
class VarRefPrinter : public MatchFinder::MatchCallback {
	public:
		virtual void run(const MatchFinder::MatchResult &Result) {
			ASTContext *Context = Result.Context;
			const DeclRefExpr *RelateVarDecl =
				Result.Nodes.getNodeAs<clang::DeclRefExpr>("varDeclRef");
			if (!RelateVarDecl )
				return;
			FullSourceLoc FullLocation =
				Context->getFullLoc(RelateVarDecl->getBeginLoc());
			const NamedDecl *VarDecl = RelateVarDecl->getFoundDecl();
			int col = FullLocation.getSpellingLineNumber();
			if(col>=func_bound[0].first&&col<=func_bound[0].second)
				return;
			std::string Name = VarDecl->getNameAsString();
			if(mark[col] == 1||mark[col] == 10)
			{
				string tem = source_file[col-1];
				int id = tem.find(',');
				if(tem.find(Name,id+1)==tem.npos)return;
			}
			var_occur_col[Name].push_back(col);
			for (auto it : ref_list[col])
				Link(it,Name);
			ref_list[col].insert(Name);
		}
};
class FuncDeclPrinter : public MatchFinder::MatchCallback{	//找出各个参与者的语料范围
	public:
		virtual void run(const MatchFinder::MatchResult &Result) {
			ASTContext *Context = Result.Context;
			const FunctionDecl*FuncDecl =
				Result.Nodes.getNodeAs<clang::FunctionDecl>("func");
			if (!FuncDecl || !Context->getSourceManager().isWrittenInMainFile(
						FuncDecl->getBeginLoc()))
				return;

			FullSourceLoc FullLocation =
				Context->getFullLoc(FuncDecl->getBeginLoc());
			int col = FullLocation.getSpellingLineNumber();
			if(FuncDecl->isMain())
			{
				int end_col = (Context->getFullLoc(FuncDecl->getEndLoc())).getSpellingLineNumber();
				func_bound[0].first=col;
				func_bound[0].second=end_col;

			}
			if(participate_col.size()&&col == participate_col.back() )
			{
				int end_col = (Context->getFullLoc(FuncDecl->getEndLoc())).getSpellingLineNumber();
				func_bound.push_back(make_pair(col,end_col));
				participate_col.pop_back();
			}

		} 
};

int main(int argc, const char **argv){
	string path_file = "result.cpp"; //默认输出路径
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
	for (auto it : sourceList) //循环t处理所有待转化文件
	{
		Init();
		fstream file;
		file.open(it);
		string temp;
		int line_count = 0;
		int par_num[]={0,1,5};
		unuse_var.insert("par");
		while (getline(file, temp))
		{
			source_file.push_back(temp);
			line_count++;
			if(temp.find(tag1)!=temp.npos)
			{
				participate_col.push_back(line_count);
				int id = temp.find(tag1);
				string temp_name = temp.substr(id+tag1.length(),temp.length()-id-tag1.length());
				//cout<<"temp_name : "<<temp_name<<endl;
				par_name.push_back(temp_name);
				parname_to_id[temp_name]=par_name.size();
			}

		}
		participate_num = participate_col.size();

		reverse(participate_col.begin(),participate_col.end());	

		ClangTool Tool(OptionsParser.getCompilations(),it);

		func_bound.push_back(make_pair(0,0));
		cutted_file.resize(participate_num+2);
		Cut(Tool);
	//	Output cutted file	
	//	for(int i = 0;i<=participate_num;++i)
	//	{
	//		for(auto its :cutted_file[i])
	//			cout<<its<<endl;
	//		cout<<"\n------------------------\n";
	//	}
		puts("Successfully cut the file!");
		cur_node_id.resize(participate_num+2);
		GenerateNodeList(participate_num);
		puts("Successfully generate node list!");
		node * start_node = LinkNode(participate_num);

		node * node_iter = start_node;
		int loop_depth = 0;
		int mx_depth = 0;
		while(node_iter!=NULL)
		{
			if(node_iter->contain("Loop_Begin"))
			{
				loop_depth++;
				node_iter->loop_depth=loop_depth;
				mx_depth=max(mx_depth,loop_depth);
			}
			else if(node_iter->contain("Loop_End"))
			{
				node_iter->loop_depth=loop_depth;
				loop_depth--;
			}
			node_iter = node_iter->nex;

		}

		cout<<"#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <pthread.h>\n#include <assert.h>\n#include <stdbool.h>";

		for(int i = 1;i<=participate_num;++i)
		{
			cout<<"\nstruct "<<toUpperCase(par_name[i-1])<<"{";

			for(auto var:var_name)
			{
				for(auto decl_col : var_decl_col[var])
				{
					if(decl_col>=func_bound[i].first&&decl_col<=func_bound[i].second)
						cout<<"\n\t"<<source_file[decl_col-1];

				}
			}

			cout<<"\n}";
			for(int j =1;j<=par_num[i];++j)
			{
				if(j!=1)cout<<",";
				cout<<toLowerCase(par_name[i-1])<<j;
			}
			cout<<";\n";
		}
		for(int i=1;i<=par_num[2];++i)
		{
			cout<<"\nvoid* Thread_"<<i<<"(void * par){";

			for(int j = 1;j<=par_num[1];++j)
			{
				node * node_iter = start_node;
				while(node_iter!=NULL)
				{
					if(node_iter->contain("send("))
					{
						string sends = node_iter->tex;
						node_iter = node_iter->nex;
						if(!node_iter->contain("recv"))
							cout<<"Error:send without recv!";
						string recvs = node_iter->tex;
						int sid1 = sends.find("(");
						int sid2 = sends.find(",",sid1+1);
						int sid3 = sends.find(")",sid2+1);
						int rid1 = recvs.find("(");
						int rid2 = recvs.find(",",rid1+1);
						int rid3 = recvs.find(")",rid2+1);
						int pos[3]={0,j,i};
						string recv_par = trim(sends.substr(sid1+1,sid2-sid1-1));
						string Lpar = trim(recvs.substr(rid2+1,rid3-rid2-1));
						if(var_name.find(Lpar)!=var_name.end() || unuse_var.find(Lpar)!=unuse_var.end())
							Lpar = toLowerCase(recv_par)+to_string(pos[parname_to_id[recv_par]])+"."+Lpar;
						string send_par = trim(recvs.substr(rid1+1,rid2-rid1-1));
						string Rpar = trim(sends.substr(sid2+1,sid3-sid2-1));
						if(var_name.find(Rpar)!=var_name.end() || unuse_var.find(Rpar)!=unuse_var.end())
							Rpar = toLowerCase(send_par)+to_string(pos[parname_to_id[send_par]])+"."+Rpar;

						cout<<"\n\t"<<Lpar<<" = "<<Rpar<<";";
					}
					else if(node_iter->contain("Loop_Begin("))
					{
						if(node_iter->loop_depth == mx_depth)
							cout<<"\n\t__VERIFIER_atomic_begin();";
					}
					else if(node_iter->contain("Loop_End("))
					{
						if(node_iter->loop_depth == mx_depth)
							cout<<"\n\t__VERIFIER_atomic_end();";
					}
					else if (node_iter->contain("par"))
					{
						string name;
						int k;
						int pos[3]={0,j,i};
						for(k=1;k<=participate_num;++k)
							if(find(cutted_file[k].begin(),cutted_file[k].end(),node_iter->tex)!=cutted_file[k].end())break;
						name = toLowerCase(par_name[k-1])+to_string(pos[k])+".";
						cout<<"\n\t"<<name<<node_iter->tex;
					}
					else if(node_iter->tex!="")
						cout<<"\n\t"<<node_iter->tex;
					node_iter = node_iter->nex;
				}


			}

			cout<<"\n\treturn 0;\n}";
		}

		cout<<"\nint main(){\n\tpthread_t thread["<<par_num[2]<<"];";

		for(int i = 1;i<=par_num[2];++i)
			printf("\n\tpthread_create(&thread[%d], NULL, Thread_%d, NULL);",i,i);	

		cout<<"\n\tfor( int i = 1; i <= "<<par_num[2]<<"; i++)\n\t\tpthread_join(thread[i], &i);\n\treturn EXIT_SUCCESS;\n}";
	}
}

bool node::contain(string x)
{
	return this->tex.find(x)!=this->tex.npos;
}
string GetF(string x)
{
	return (F[x]==x?x:F[x]=GetF(F[x]));
}
string trim(string x)
{
	int len = x.length();
	int L;
	for (L = 0; L < len; ++L)
		if (x[L] != ' '&&x[L]!='\t'&&x[L]!='&'&&x[L]!='*')
			break;
	int R;
	for (R=len-1; R >= 0;R--)
		if (x[R] != ' '&&x[R]!='\t')
			break;
	return x.substr(L, R - L + 1);

}
void Init()
{
	source_file.clear();
	sendrecv_col.clear();
	ref_list.clear();
	var_name.clear();
	var_occur_col.clear();
	var_decl_col.clear();
	F.clear();
	remain_var.clear();
	unuse_var.clear();
	mark.clear();
	main_function_begin = 0;
	depth = 0;
}
void Link(string a, string b)
{
	string Fa = GetF(a);
	string Fb = GetF(b);
	if (Fa!= Fb)
		F[Fa] = Fb;
}
//简化send和recv
string SimSendRecv(int col,int cur_file_id)
{
	string x = source_file[col];
	int id1 = x.find('(');
	int id2 = x.find(',', id1+1);
	int id3 = min(x.find(',', id2 + 1),x.find(')',id2+1));
	string to;
	string para = trim(x.substr(id2 + 1, id3 - id2 - 1));
	if(participate_num == 2)
	{
		int to_id = !(cur_file_id-1);
		to = par_name[to_id];
	}
	else
	{
		int id4 = x.find("//to ", id3+1);
		to = trim(x.substr(id4 + 5, x.length() - id4 - 5));
	}
	transform(to.begin(),to.end(),to.begin(),::tolower);
	if(mark[col+1]==1)
		return "send(" + to + "," + para + ")";
	else
		return "recv(" + to + "," + para + ")";
}
void Cut(ClangTool Tool)
{

	int sz = source_file.size();
	ref_list.resize(sz+3);
	SendPrinter sendPrinter;
	RecvPrinter recvPrinter;
	RelateForPrinter relateForPrinter;
	VarDeclPrinter varDeclPrinter;
	// RelateIfPrinter relateIfPrinter;
	VarRefPrinter varRefPrinter;
	FuncDeclPrinter funcDeclPrinter;

	MatchFinder Finder1, Finder2,Finder3,Finder4;
	Finder1.addMatcher(SendMatcher, &sendPrinter);
	Finder1.addMatcher(RecvMatcher, &recvPrinter);
	Finder1.addMatcher(FuncDeclMatcher, &funcDeclPrinter); 
	Tool.run(newFrontendActionFactory(&Finder1).get());
	Finder2.addMatcher(VarDeclMatcher, &varDeclPrinter);
	Tool.run(newFrontendActionFactory(&Finder2).get());
	for (auto var : var_name)
	{
		StatementMatcher VarRefMatcher = declRefExpr(to(varDecl(hasName(var)))).bind("varDeclRef");
		Finder3.addMatcher(VarRefMatcher, &varRefPrinter);
		F[var] = var;
	}

	Tool.run(newFrontendActionFactory(&Finder3).get());


	for (auto sc : sendrecv_col)
	{
		if(ref_list[sc].size()==0)continue;
		auto first_element = ref_list[sc].begin();
		remain_var[GetF(*first_element)] = 1;
	}

	for (auto iter = var_name.begin(); iter != var_name.end();)
	{
		if (remain_var[GetF(*iter)] == 0)
			var_name.erase(iter++);
		else
			iter++;
	}

	for(auto var :var_name)
	{
		for(auto it:var_decl_col[var])
		{
			if(!mark[it])
				mark[it] = 12;
		}
		for(auto it:var_occur_col[var])
		{
			if(!mark[it])
				mark[it] = 2;
		}
	}

	Finder4.addMatcher(RelateForMatcher, &relateForPrinter);
	Tool.run(newFrontendActionFactory(&Finder4).get());

	//将不同对象的裁剪语料分开存储，全局变量和主函数的关键语句存储在0中
	int iter =0;
	for (int i = 1; i <=participate_num; ++i)
	{
		int l = func_bound[i].first;
		int r = func_bound[i].second;
		while(iter+1<l)
		{
			if(mark[iter+1]==2||mark[iter+1]==3)
				cutted_file[0].push_back(trim(source_file[iter]));
			else if(mark[iter+1]==1||mark[iter+1]==10)
				cutted_file[0].push_back(trim(SimSendRecv(iter,participate_num+1)));
			iter++;
		}
		while(iter+1<r)
		{
			if(mark[iter+1]==2||mark[iter+1]==3)
				cutted_file[i].push_back(trim(source_file[iter]));
			else if(mark[iter+1]==1||mark[iter+1]==10)
				cutted_file[i].push_back(trim(SimSendRecv(iter,i)));
			iter++;
		}
	}
	while(iter+1<sz)
	{
		if(mark[iter+1]==2)
			cutted_file[0].push_back(trim(source_file[iter]));
		else if(mark[iter+1]==1||mark[iter+1]==10)
			cutted_file[0].push_back(trim(SimSendRecv(iter,participate_num+1)));
		iter++;

	}
}
//生成各参与者的节点序列
void GenerateNodeList(int participate_num)
{
	for (int i = 1; i <= participate_num; i++)
	{
		node* las_node;
		for(int j = 0;j < cutted_file[i].size();++j)
		{
			if(j == 0)
			{
				las_node = new node(depth, cutted_file[i][j]);
				cur_node_id[i] = las_node;
			}
			else
			{
				node* node_temp = new node(depth, cutted_file[i][j]);
				if (node_temp->contain("Loop_Begin"))
				{
					depth++;
				}
				else if(node_temp->contain("Loop_End"))
				{
					depth--;
				}
				//串联节点
				las_node->nex = node_temp;
				node_temp->pre = las_node;
				las_node = node_temp;

			}
		}
	}
}
int GetDestination(node * x)
{
	string tem = x->tex;
	int id1 = tem.find('(', 0);
	int id2 = tem.find(',', id1 + 1);
	return parname_to_id[trim(tem.substr(id1+1,id2-id1-1))];
}
node * FindRecv(int id,int from)
{
	node * node_iter = cur_node_id[id];
	for (;node_iter!=NULL;node_iter = node_iter->nex)
	{
		if(node_iter->contain("recv(")) 
		{
			if(from == GetDestination(node_iter))//是否是要找的recv
			{
				cur_node_id[id]=node_iter->nex;//纪录一下已经遍历的位置，下次从此处开始遍历
				return node_iter;
			}
			else
				puts("format error!");
		}
	}
}		
//check if the end is reached
bool CheckIsEnd(node * x,int id)
{
	return (x == NULL||((x->contain("Loop_")||x->contain("send(")||x->contain("recv("))&&GetDestination(x)!=id));
}
string toUpperCase(string s)
{
	transform(s.begin(),s.end(),s.begin(),::toupper);
	return s;
}
string toLowerCase(string s)
{
	transform(s.begin(),s.end(),s.begin(),::tolower);
	return s;
}
//串联合并节点序列成一个节点链表
//send recv尽量贴合 recv前的多余语句置于send前，send后的多余语句置于recv后，这样可互不影响
//需严格保证每一段只存在两两间的交互，如果不是也可低成本修改
void LinkPair(int A,int B)
{
	bool is_ended_A = false;
	bool is_ended_B = false;
	node *tem_iter = cur_node_id[A];
	for (;tem_iter!=NULL;tem_iter = tem_iter->nex)
	{
		if(tem_iter->contain("send("))break;
		else if(tem_iter->contain("recv("))
		{
			swap(A,B);	
			break;
		}
	}

	while(!is_ended_A||!is_ended_B)
	{
		if(!is_ended_A)
		{

			node * node_iter = cur_node_id[A];
			for (;node_iter!=NULL;node_iter = node_iter->nex)
			{
				if (node_iter->contain("send("))
				{
					cur_node_id[A]=node_iter->nex;//纪录一下已经遍历的位置，下次从此处开始遍历

					node * head_iter=cur_node_id[B];
					node * recv_iter=FindRecv(B,A);
					if(head_iter!=recv_iter) //若recv前有多余部分则至于send语句前
					{
						(node_iter->pre)->nex = head_iter;
						head_iter->pre = node_iter->pre;

					}
					(recv_iter->pre)->nex = node_iter;
					node_iter->pre = recv_iter->pre;
					recv_iter->nex = node_iter->nex;
					(node_iter->nex)->pre = recv_iter;
					node_iter->nex = recv_iter; //串联send和recv
					recv_iter->pre = node_iter;

					break;

				}
				if (CheckIsEnd(node_iter->nex,B))
				{
					cur_node_id[A] = node_iter->nex;
					is_ended_A = true;
					break;
				}
			}
			if(node_iter == NULL)
			{
				is_ended_A = true;
			}
		}
		if(!is_ended_B)
		{
			node * node_iter = cur_node_id[B];
			for (;node_iter!=NULL;node_iter = node_iter->nex)
			{
				if (node_iter->contain("send("))
				{
					cur_node_id[B]=node_iter->nex;//纪录一下已经遍历的位置，下次从此处开始遍历

					node * head_iter=cur_node_id[A];
					node * recv_iter=FindRecv(A,B);
					if(head_iter!=recv_iter) //若recv前有多余部分则至于send语句前
					{
						(node_iter->pre)->nex = head_iter;
						head_iter->pre = node_iter->pre;

					}
					(recv_iter->pre)->nex = node_iter;
					node_iter->pre = recv_iter->pre;
					recv_iter->nex = node_iter->nex;
					(recv_iter->nex)->pre = recv_iter;
					node_iter->nex = recv_iter; //串联send和recv
					recv_iter->pre = node_iter;

					break;

				}
				if (CheckIsEnd(node_iter->nex,A))
				{
					cur_node_id[B] = node_iter->nex;
					is_ended_B = true;
					break;
				}
			}
			if(node_iter == NULL)
			{
				is_ended_B = true;
			}
		}
	}

}
node * LinkNode(int participate_num)
{
	bool is_completed = false;
	vector<node *>head_node;
	for( int i=1;i<=participate_num;i++)
		head_node.push_back(cur_node_id[i]);

	while(!is_completed)
	{
		int pair_to[participate_num+2]={0};
		bool is_linked[participate_num+2]={0};
		for(int i=1;i<=participate_num;i++)	//遍历所有对象查找接下来进行链接的对
		{
			node* tem_iter = cur_node_id[i];
			//cout<<"par: "<<i<<endl;
			if(tem_iter == NULL)continue;
			for(;tem_iter!=NULL;tem_iter = tem_iter->nex)
			{
				if(tem_iter->contain("send(")||tem_iter->contain("recv("))
				{	
					pair_to[i] = GetDestination(tem_iter);
					//cout<<tem_iter->tex<<"  dest: "<<pair_to[i]<<endl;
					break;
				}
			}
			//cout<<i<<" "<<pair_to[i]<<endl;
			if(pair_to[pair_to[i]]==i&&is_linked[pair_to[i]]==false)	//配对成功
			{
				LinkPair(i,pair_to[i]);	
				is_linked[i] = true;
				is_linked[pair_to[i]] = true;
			}
		}
		is_completed = true;
		for(int i=1;i<=participate_num;i++)	//遍历所有对象看是否已经到达末尾
		{
			is_completed = (is_completed&&(cur_node_id[i]==NULL));
		}
	}
	for( int i=1;i<=participate_num;i++)
		if(head_node[i-1]->pre==NULL)
			return head_node[i-1];

}

