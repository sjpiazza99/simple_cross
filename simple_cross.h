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

/*
 * Price-Time FIFO ordering for the heaps used in the order book
 *
 * ASSUMPTION: order ids destined for map[symbol][side] are larger than previous
 *             order ids already in the destination.
 * 
 * This is the comparator function for the heap. It prioritizes high buy prices
 * and low sell prices. If two orders have the same price, the lower order id
 * is prioritized to ensure FIFO ordering
 *
 * @param  ord1 - parent of ord2
           ord2 - child of ord1
 * @return bool - if the nodes the nodes in the heap should be swapped
*/
struct PriceTimeOrder {
  bool operator()(std::shared_ptr<order_t> ord1, std::shared_ptr<order_t> ord2)
  {
    if(ord1->ord_px == ord2->ord_px){
      //Prioritize lower oid
      if(ord1->oid > ord2->oid){
        std::swap(ord1->idx, ord2->idx);
        return true;
      }
      return false;
    }
    if(ord1->side == 'B'){
      //Prioritize higher buy price
      if(ord1->ord_px < ord2->ord_px){
        std::swap(ord1->idx, ord2->idx);
        return true;
      }
      return false;
    }
    //Prioritize lower sell price
    if(ord1->ord_px > ord2->ord_px){
      std::swap(ord1->idx, ord2->idx);
      return true;
    }
    return false;
  }
};

/*
 * Sorted Order for printing order books
 *
 * This is the comparator function for print_orders(). It prioritizes high
 * order price across all maps.
 *
 * @param  ord1 - parent of ord2
           ord2 - child of ord1
 * @return bool - if the nodes the nodes should be swapped
*/
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
    std::unordered_map<unsigned int, std::shared_ptr<order_t>> oids_m;
    std::unordered_map<unsigned int, bool> used_oids_m;
    results_t print_orders(); 
    void erase_order(std::shared_ptr<order_t> order); 
    void erase_top(std::shared_ptr<order_t> order); 
    void create_order(request_t rq); 
    results_t handle_cross(std::string symbol); 
    request_t handle_request(const std::string& line);
  public:
    results_t action(const std::string& line); 
};
