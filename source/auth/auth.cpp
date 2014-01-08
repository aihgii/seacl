/*
 * Copyright (C) 2013—2014, Evgeny Vais <vais.evgeny@gmail.com>
 *
 * This file is part of SEACL.
 *
 *  SEACL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.

 *  SEACL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with SEACL.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    string config_path = "/etc/seacl/seacl.conf";
    boost::property_tree::ptree config;
    boost::property_tree::ptree subcfg;
    boost::property_tree::ptree myauth;
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
    boost::property_tree::read_ini(subcfg.get<std::string>("auth"),myauth);
    db.connect(myauth.get<std::string>("database").c_str(),
               myauth.get<std::string>("server").c_str(),
               myauth.get<std::string>("user").c_str(),
               myauth.get<std::string>("password").c_str());

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
