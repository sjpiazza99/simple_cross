// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <regex>
#include "boost/lexical_cast.hpp"

typedef std::list<std::string> results_t;

typedef struct Order
{
  unsigned short qty;
  double px;
} order_t;

typedef struct Request
{
  char action;
  unsigned int oid;
  std::string symbol;
  char side;
  unsigned short qty;
  double px;
} request_t;

class SimpleCross
{
  private:
    std::map<std::string, std::map<char, std::map<unsigned int, order_t>>> order_book_m; 
    //map[symbol][side][oid] => order_t
  public:
    request_t handle_request(const std::string& line);
    results_t action(const std::string& line); 
};
