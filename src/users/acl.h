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
    typedef uint8_t flags;
  protected:
    typedef std::vector <std::wstring> wRow;
    typedef std::deque  <wRow>         wTable;

    mysqlpp::Connection db;
    flags table_flags;

    std::wstring get_wstring (const char *);
    std::string  get_string  (const wchar_t *);
    wTable*      get_table   (mysqlpp::UseQueryResult &);
  public:
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

    static const flags F_EUI48 = 1;

    void setf(flags);
    void unsetf();
    void unsetf(flags);

    int  add(int,std::string);
    void del(int);
    void mod(int,int,std::string);
    void mod_eui48(int,std::string);

    bool exist(int);
    std::string table();
    int gid(int);
    std::string username(int);
    std::string eui48(int);
  };

}

#endif
