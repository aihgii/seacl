#ifndef EUI48_T_H
#define EUI48_T_H

#include <string>

namespace seacl {

  class eui48_t
  {
  private:
    uint8_t value[6];
    std::string exp_eui48;
    bool _empty;
  public:
     eui48_t();
    ~eui48_t();

    bool parse(std::string);
    bool empty();
    void clear();
    std::string str();
  };

}

#endif
