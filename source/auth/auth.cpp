#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <mysql++.h>
#include <boost/property_tree/ini_parser.hpp>

#define BUF_SIZE 1*1024

using namespace std;

int main(int argc,char* argv[]){
  try
  {
    int uid,gid = 0;
    char* buf = new char [BUF_SIZE];
    string config_path = "/etc/conf.d/seacl";
    boost::property_tree::ptree config;
    boost::property_tree::ptree subcfg;
    map <string,int> arg;
    string mac;
    mysqlpp::Connection db;
    mysqlpp::Query query = db.query();
    vector <mysqlpp::Row> res;

    arg["-c"]		= 1;
    arg["--config"]	= 1;
    arg["-g"]		= 2;
    arg["--gid"]	= 2;

    for (int i=1;i<argc;i++)
    {
      switch(arg[argv[i]])
      {
        case 1:
          if(argc <= i+1) break;
          config_path = argv[++i];
          break;
        case 2:
          gid = atoi(argv[++i]);
          break;
        default:
          throw invalid_argument("Unknown argument");
      }
    }

    boost::property_tree::read_ini(config_path,config);
    subcfg = config.get_child("MySQL");
    db.connect(subcfg.get<std::string>("database").c_str(),
               subcfg.get<std::string>("server").c_str(),
               subcfg.get<std::string>("user").c_str(),
               subcfg.get<std::string>("password").c_str());

    while(true){
      fgets(buf,BUF_SIZE,stdin);

      mac = strtok(buf,"\n");

      query << "SELECT `Users`.`UID` FROM `Users`,`EUI48` WHERE ( `Users`.`UID` = `EUI48`.`UID` AND `EUI48`.`Value` = '" << mac << "' AND `Users`.`GID` = '" << gid << "' )";
      res.clear();
      query.storein(res);
      if (!res.empty()){
        uid = atoi(res[0]["UID"]);
        fprintf (stdout,"OK user=uid_%d\n",uid);
      }
      else
        fprintf(stdout,"ERR\n");
      fflush(stdout);

      if (getppid() == 1){
        db.disconnect();
        return EXIT_SUCCESS;}
    }
  }

  catch(exception& e)
  {
    int code;
    map <string,int> excp;

    excp["Unknown argument"]			= 100;

    code = excp[e.what()];

    switch(code)
    {
      case 100:
        break;
      default:
        ++code;
    }

    exit(code);
  }

  catch(...)
  {
    exit(2);
  }

}
