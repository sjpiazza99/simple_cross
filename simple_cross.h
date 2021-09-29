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
  unsigned int idx;
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
  bool operator()(std::shared_ptr<order_t> ord1, std::shared_ptr<order_t> ord2)
  {
    if(ord1->side == 'B'){
      if(ord1->ord_px < ord2->ord_px){
        std::swap(ord1->idx, ord2->idx);
        return true;
      }
      return false;
    }
    if(ord1->ord_px > ord2->ord_px){
      std::swap(ord1->idx, ord2->idx);
      return true;
    }
    return false;
  }
};

struct SortedOrder {
  bool operator()(std::shared_ptr<order_t> ord1, std::shared_ptr<order_t> ord2)
  {
    return ord1->ord_px >= ord2->ord_px;
  }
};

class SimpleCross
{
  private:
    std::unordered_map<std::string, std::unordered_map<char, std::vector<std::shared_ptr<order_t>>>> order_book_m; 
    //map[symbol][side] = vector<order_t> (max heap / priority queue for price-time fifo)
    std::unordered_map<unsigned int, std::shared_ptr<order_t>> oids_m;
    //map[oid] = order_t (needed to parse order book)
    std::unordered_map<unsigned int, bool> used_oids_m;
    results_t print_orders(); 
    void print_heap(); 
    void erase_order(std::shared_ptr<order_t> order); 
    results_t handle_cross(request_t rq); 
    request_t handle_request(const std::string& line);
  public:
    results_t action(const std::string& line); 
};
