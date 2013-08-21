#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libintl.h>
#include <map>
#include <string.h>
#include <mysql++.h>

#define _(s) gettext(s)
#define BUF_SIZE 4*1024

using namespace std;

int main(int argc,char* argv[]){
try
{
  char* buf = new char [BUF_SIZE];
  int pid,uid,gid = 0;
  int pipeRead[2];
  int pipeWrite[2];
  bool redirect = false;
  map <string,int> arg;
  string mac,url,src,method,myaddr,myport;
  stringstream request,q;
  mysqlpp::Connection db;
  mysqlpp::Query* chk;

  arg["--gid"]			= 1;
  arg["-g"]				= 1;
  arg["--redirection"]	= 2;
  arg["-r"]				= 2;

  for (int i=1;i<argc;i++)
  {
    switch(arg[argv[i]])
    {
      case 1:
        gid = atoi(argv[++i]);
        break;
      case 2:
        redirect = true;
        pipe(pipeRead);
        pipe(pipeWrite);
        pid = fork();

        if (pid == 0)
        {
          close(pipeRead[0]);
          close(pipeWrite[1]);
          dup2(pipeRead[1],STDOUT_FILENO);
          dup2(pipeWrite[0],STDIN_FILENO);
          execl("/usr/bin/squidGuard","(squidGuard)",NULL);
        }

        close(pipeRead[1]);
        close(pipeWrite[0]);
        break;
      default:
        throw (string)"Unknown argument";
    }
  }
  
  string database = "seacl";
  string server = "localhost";
  string user = "seacl";
  string password = "seacl";
  db.connect(database.c_str(),server.c_str(),user.c_str(),password.c_str());

  while (true){
    fgets (buf,BUF_SIZE,stdin);

    mac = strtok(buf,"\n ");

    if (redirect)
    {
      url = strtok(NULL,"\n ");
      src = strtok(NULL,"\n ");
      method = strtok(NULL,"\n ");
      myaddr = strtok(NULL,"\n ");
      myport = strtok(NULL,"\n ");
      request.str("");
      request << url << ' ' << src << "/- GID_" << gid << ' ' << method << " myip=" << myaddr << " myport=" << myport << '\n';
      write(pipeWrite[1],request.str().c_str(),request.str().length());
      read(pipeRead[0],buf,BUF_SIZE);
      request.str("");
      if (buf[0] != '\n') request << strtok(buf," ");
    }

    q << "SELECT `Users`.`UID` FROM `Users`,`EUI48` WHERE ( `Users`.`UID` = `EUI48`.`UID` AND `EUI48`.`Value` = '" << mac << "' AND `Users`.`GID` = '" << gid << "' )";
    chk = new mysqlpp::Query(db.query(q.str()));
    if (mysqlpp::StoreQueryResult res = chk->store())
    {
      if (!res.empty())
      {
        uid = atoi(res[0]["UID"]);
        if (request.str().length())
          fprintf (stdout,"ERR user=UID_%d\n",uid);
        else
          fprintf (stdout,"OK user=UID_%d\n",uid);
      }
      else
        fprintf (stdout,"ERR\n");
    }
    else
      throw (string)"Request to database failed";
    fflush (stdout);

    q.str("");
    delete chk;
    if (getppid() == 1){
      db.disconnect();
      return 0;}
  }
}

catch(string& str)
{
  map <string,int> err;

  err["Unknown argument"]			= 2;
  err["Request to database failed"]	= 3;

  exit(err[str]);
}

catch (mysqlpp::ConnectionFailed excp)
{
  exit(4);
}

catch(...)
{
  exit(1);
}

}
