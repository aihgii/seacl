#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <mysql++.h>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;

int main(int argc,char* argv[]){
  try
  {
    int gid = -1;
    string config_path = "/etc/conf.d/seacl";
    boost::property_tree::ptree config;
    boost::property_tree::ptree subcfg;
    map <string,int> arg;
    mysqlpp::Connection db;
    mysqlpp::Query query = db.query();
    mysqlpp::UseQueryResult res;

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

    query << "SELECT `UID` FROM `Users`";
    if(gid >= 0)
      query << " WHERE `GID` = '" << gid << "'";
    res = query.use();

    while(mysqlpp::Row row = res.fetch_row())
      cout << "uid_" << row[0] << endl;

    return EXIT_SUCCESS;
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
