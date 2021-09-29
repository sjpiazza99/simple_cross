// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include "simple_cross.h"

results_t SimpleCross::action(const std::string& line){ 
  request_t rq;
  results_t res;
  //Ensure no malformed input
  try {
    rq = handle_request(line);
  }
  catch(std::invalid_argument& e) {
    res.push_back(e.what());
    return res;
  }

  //Perform action requested
  switch(rq.action){
    case 'P':
      res = print_orders();
      break;
    case 'X':
      //Check if oid exists
      if(oids_m[rq.oid] == nullptr){
        res.push_back("E "+ std::to_string(rq.oid) + " OID does not exist");
        break;
      }
      if(oids_m[rq.oid]->open_qty == 0){
        res.push_back("E " + std::to_string(rq.oid) + " Order already fully filled");
        break;
      }
      erase_order(*oids_m[rq.oid]);
      res.push_back("X "+ std::to_string(rq.oid));
      //print_heap();
      break;
    case 'O':
      //Check if oid already exists
      if(oids_m[rq.oid] != nullptr){
        res.push_back("E " + std::to_string(rq.oid) + " Duplicate order id");
        break;
      }
      res = handle_cross(rq);
      //print_heap();
  }
  return res;
}

//O(logn)
results_t SimpleCross::handle_cross(request_t rq){
  results_t res;
  oids_m[rq.oid] = std::shared_ptr<order_t>(new Order());
  auto& order_heap = order_book_m[rq.symbol][rq.side];
  *oids_m[rq.oid] = {
    0, rq.qty, 0, rq.px, rq.oid, 
    rq.symbol, rq.side, static_cast<unsigned int>(order_heap.size())
  };
  order_heap.push_back(oids_m[rq.oid]);
  std::push_heap(order_heap.begin(), order_heap.end(), PriceTimeOrder());
  
  //Ensure book holds orders for cross side
  char op_side = rq.side == 'B' ? 'S' : 'B';
  if(order_book_m[rq.symbol].count(op_side) == 0)
    return res;
  
  auto& cross_heap = order_book_m[rq.symbol][op_side];
  auto sames_ord = order_heap.front();
  auto cross_ord = cross_heap.front();
  
  //Look for a fill
  if((sames_ord->side == 'B' && sames_ord->ord_px >= cross_ord->ord_px) ||
     (sames_ord->side == 'S' && sames_ord->ord_px <= cross_ord->ord_px)){    
    if(cross_ord->open_qty >= sames_ord->open_qty){
      cross_ord->fill_qty = sames_ord->open_qty;
      cross_ord->open_qty -= sames_ord->open_qty;
      sames_ord->fill_qty = sames_ord->open_qty;
      sames_ord->open_qty = 0;
    }
    else{
      sames_ord->fill_qty = cross_ord->open_qty;
      sames_ord->open_qty -= cross_ord->open_qty;
      cross_ord->fill_qty = cross_ord->open_qty;
      cross_ord->open_qty = 0;
    }
    if(sames_ord->side == 'B'){//possibly buying at lower price
      sames_ord->fill_px = cross_ord->ord_px;
      cross_ord->fill_px = cross_ord->ord_px;
    }
    else{//cross may be buying at lower price
      sames_ord->fill_px = sames_ord->ord_px;
      cross_ord->fill_px = sames_ord->ord_px;
    }
    res.push_back(
      "F " + std::to_string(sames_ord->oid) +
      " " + sames_ord->symbol +
      " " + std::to_string(sames_ord->fill_qty) +
      " " + std::to_string(sames_ord->fill_px)
    );
    res.push_back(
      "F " + std::to_string(cross_ord->oid) +
      " " + cross_ord->symbol +
      " " + std::to_string(cross_ord->fill_qty) +
      " " + std::to_string(cross_ord->fill_px)
    );
  }
  
  //Check if full fill
  if(cross_ord->open_qty == 0){
    std::pop_heap(cross_heap.begin(), cross_heap.end(), PriceTimeOrder());
    cross_heap.pop_back();
  }
  if(sames_ord->open_qty == 0){
    std::pop_heap(order_heap.begin(), order_heap.end(), PriceTimeOrder());
    order_heap.pop_back();
  }
  return res;
}

/*
void SimpleCross::print_heap(){
  std::vector<order_t> sorted_orders;
  std::cout << "---------------\n";
  for(auto symbol_book : order_book_m){
    sorted_orders = symbol_book.second['B'];
    for(auto order : sorted_orders){
      std::cout <<
        "P " + std::to_string(order.oid) + " " + order.symbol + " " + 
        order.side + " " + std::to_string(order.open_qty) + 
        " " + std::to_string(order.ord_px)
      << '\n';
    } 
    sorted_orders = symbol_book.second['S'];
    for(auto order : sorted_orders){
      std::cout <<
        "P " + std::to_string(order.oid) + " " + order.symbol + " " + 
        order.side + " " + std::to_string(order.open_qty) + 
        " " + std::to_string(order.ord_px)
      << '\n';
    } 
  }
  std::cout << "---------------\n";
}
*/

//O(mnlogn) - not sure how optimized this needs to be
results_t SimpleCross::print_orders(){
  results_t res;
  std::vector<std::shared_ptr<order_t>> sorted_orders;
  for(auto symbol_book : order_book_m){
    sorted_orders = symbol_book.second['B'];
    sorted_orders.insert(sorted_orders.end(), symbol_book.second['S'].begin(), symbol_book.second['S'].end());
    std::sort(sorted_orders.begin(), sorted_orders.end(), SortedOrder());
    for(auto order : sorted_orders){
      res.push_back(
        "P " + std::to_string(order->oid) + " " + order->symbol + " " + 
        order->side + " " + std::to_string(order->open_qty) + 
        " " + std::to_string(order->ord_px)
      );
    } 
  }
  return res;
}

//O(logn)
void SimpleCross::erase_order(order_t order){
  auto& order_heap = order_book_m[order.symbol][order.side];
  order_heap.erase(order_heap.begin()+order.idx);
  std::make_heap(order_heap.begin(), order_heap.end(), PriceTimeOrder()); //need to heapify again
}

/*
 * Parse string as request_t struct
 *
 * Robust error handling for reading requests from actions.txt. 
 * This could be done with a single regex, however, it would be at the 
 * expense of a single generic error message. Rather than that I've
 * prioritized being explicit.
 *
 * @param line the string that should be parsed
 * @return request_t struct describing the request made
*/
request_t SimpleCross::handle_request(const std::string& line){
  if(line == "")
    throw std::invalid_argument("E Missing arguments");
  request_t rq;
  if(line == "P"){
    rq.action = 'P';
    return rq;
  }
  std::istringstream iss(line);
  std::vector<std::string> in{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
  
  if(!std::regex_match(in[0], std::regex("[OX]")))
    throw std::invalid_argument("E Invalid action type: " + in[0]);
  rq.action = in[0].at(0);
  if((rq.action == 'X' && in.size() < 2) | (rq.action == 'O' && in.size() < 6))
    throw std::invalid_argument("E Missing arguments");
  if(std::regex_match(in[1], std::regex("-[0-9]+")))
    throw std::invalid_argument("E " + in[1] + " OID must be positive");
  try {
    rq.oid = boost::lexical_cast<unsigned int>(in[1]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E " + in[1] + " OID must be an unsigned int");
  }
  if(rq.action == 'X')
    return rq;
  if(!std::regex_match(in[2], std::regex("[A-Z0-9]{1,8}")))
    throw std::invalid_argument("E " + in[1] + " Invalid symbol: "+ in[2]);
  rq.symbol = in[2];
  if(!std::regex_match(in[3], std::regex("[BS]")))
    throw std::invalid_argument("E " + in[1] + " Invalid side: " + in[3]);
  rq.side = in[3].at(0);
  if(std::regex_match(in[4], std::regex("-[0-9]+")))
    throw std::invalid_argument("E " + in[1] + " QTY must be positive");
  try {
    rq.qty = boost::lexical_cast<unsigned int>(in[4]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E " + in[1] + " QTY must be an unsigned short");
  }
  if(std::regex_match(in[5], std::regex("-[0-9]+(.*)")))
    throw std::invalid_argument("E " + in[1] + " PX must be positive");
  try {
    rq.px = boost::lexical_cast<double>(in[5]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E " + in[1] + " PX must be a double");
  }
  return rq;
}
