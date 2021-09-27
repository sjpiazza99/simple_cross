#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include "boost/lexical_cast.hpp"

typedef std::list<std::string> results_t;

typedef struct Order
{
  unsigned short fill_qty;
  unsigned short open_qty;
  double fill_px;
  double ord_px;
  unsigned int oid;
  std::string symbol;
  char side;
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

struct PriceTimeOrder {
  bool operator()(order_t const& ord1, order_t const& ord2)
  {
      return ord1.ord_px > ord2.ord_px;
  }
};

struct SortedOrder {
  bool operator()(order_t const& ord1, order_t const& ord2)
  {
      return ord1.ord_px >= ord2.ord_px;
  }
};

class SimpleCross
{
  private:
    std::map<std::string, std::map<char, std::vector<order_t>>> order_book_m; 
    //map[symbol][side] = vector<order_t> (max heap / priority queue for price-time fifo)
    std::map<unsigned int, order_t> oids_m;
    //map[oid] = order_t (needed to parse order book)
  public:
    request_t handle_request(const std::string& line);
    results_t action(const std::string& line); 
    results_t print_orders(); 
    void erase_order(order_t order); 
    void handle_cross(request_t rq); 
};
