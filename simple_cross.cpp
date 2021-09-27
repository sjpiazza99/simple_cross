// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include "simple_cross.h"

results_t SimpleCross::action(const std::string& line){ 
  request_t rq;
  results_t res;
  try {
    rq = handle_request(line);
  }
  catch(std::invalid_argument& e) {
    res.push_back(e.what());
    return res;
  }
  if(rq.action == 'P'){
    //res.push_back(std::to_string(order_book));
    res.push_back("HII");
    return res;
  }
  order_book_m[rq.symbol][rq.side][rq.oid] = {rq.qty, rq.px};
  res.push_back(std::to_string(rq.oid));
  return res; 
}

//TODO fix segfault and better formatting with OIDs
request_t SimpleCross::handle_request(const std::string& line){
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
  if(std::regex_match(in[1], std::regex("-[0-9]+")))
    throw std::invalid_argument("E OID must be positive");
  try {
    rq.oid = boost::lexical_cast<unsigned int>(in[1]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E OID must be an unsigned int");
  }
  if(rq.action == 'X')
    return rq;
  if(!std::regex_match(in[2], std::regex("[A-Z]{1,8}")))
    throw std::invalid_argument("E Invalid symbol: "+ in[2]);
  rq.symbol = in[2];
  if(!std::regex_match(in[3], std::regex("[BS]")))
    throw std::invalid_argument("E Invalid side: " + in[3]);
  rq.side = in[3].at(0);
  if(std::regex_match(in[4], std::regex("-[0-9]+")))
    throw std::invalid_argument("E QTY must be positive");
  try {
    rq.qty = boost::lexical_cast<unsigned int>(in[4]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E QTY must be an unsigned short");
  }
  if(std::regex_match(in[5], std::regex("-[0-9]+(.*)")))
    throw std::invalid_argument("E PX must be positive");
  try {
    rq.px = boost::lexical_cast<double>(in[5]);
  }
  catch(boost::bad_lexical_cast &) {
    throw std::invalid_argument("E PX must be a double");
  }
  return rq;
}
