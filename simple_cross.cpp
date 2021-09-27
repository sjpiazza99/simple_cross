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
  if(rq.action == 'P'){ //O(nlogn)
    std::vector<order_t> sorted_orders;
    for(auto symbol_book : order_book_m){
      sorted_orders = symbol_book.second['B'];
      sorted_orders.insert(sorted_orders.end(), symbol_book.second['S'].begin(), symbol_book.second['S'].end());
      std::sort(sorted_orders.begin(), sorted_orders.end(), SortedOrder());
      for(auto order : sorted_orders){
        res.push_back(
            "P " + std::to_string(order.oid) + " " + order.symbol + " " + 
            order.side + " " + std::to_string(order.open_qty - order.fill_qty) + 
            " " + std::to_string(order.ord_px)
        );
      } 
    }
    return res;
  }
  if(rq.action == 'X'){ //Maybe i can do O(logn) ~ OR O(1) if u keep indexes;
    if(oids_m.count(rq.oid) == 0){
      res.push_back("E "+ std::to_string(rq.oid) + " OID does not exist");
      return res;
    }
    //auto order = oids_m[rq.oid];
    //auto order_queue = order_book_m[order.symbol][order.side];
    //order_queue.erase(std::find(order_queue.begin(), order_queue.end(), order));
    oids_m.erase(rq.oid);
    res.push_back("X "+ std::to_string(rq.oid));
    return res;
  }
  //O(logn)
  //Check if oid already exists
  if(oids_m.count(rq.oid)){
    res.push_back("E " + std::to_string(rq.oid) + " Duplicate order id");
    return res;
  }
  oids_m[rq.oid] = {0, rq.qty, 0, rq.px, rq.oid, rq.symbol, rq.side};
  auto order_heap = order_book_m[rq.symbol][rq.side];
  order_heap.push_back(oids_m[rq.oid]);
  std::push_heap(order_heap.begin(), order_heap.end(), PriceTimeOrder());
  order_book_m[rq.symbol][rq.side] = order_heap;
  //res.push_back(std::to_string(rq.oid));

  return res; 
}

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
  if(!std::regex_match(in[2], std::regex("[A-Z]{1,8}")))
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
