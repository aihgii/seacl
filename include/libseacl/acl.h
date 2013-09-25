#ifndef ACL_H
#define ACL_H

#include <deque>
#include <vector>
#include <string>
#include <mysql++.h>

namespace seacl {

  class acl
  {
  public:
    typedef uint8_t flag;
    typedef std::vector <const char*> Row;
    typedef std::deque  <Row>         Table;

    acl();
    acl(std::string,
        std::string,
        std::string,
        std::string);
    ~acl();

    bool connect(std::string,
                 std::string,
                 std::string,
                 std::string);

    static const flag F_EUI48	= 1 << 0;

    void setf(flag);
    void unsetf();
    void unsetf(flag);

    int  add(int,std::string);
    void del(int);
    void mod(int,int,std::string);
    void mod_eui48(int,std::string);

    bool exist(int);
    int gid(int);
    std::string username(int);
    std::string eui48(int);
    Table* table();
  protected:
    mysqlpp::Connection db;
    flag table_flags;
  };

}

#endif
