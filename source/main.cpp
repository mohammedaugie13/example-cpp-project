#include <iostream>
#include <nlohmann/json.hpp>


#include "HttpClient.hpp"
using json = nlohmann::json;
int main() {
    json j = "{ \"happy\": true, \"pi\": 3.141 }"_json;
    std::string s = j.dump();
    std::cout << j.dump(4) << std::endl;
    std::cout <<  "aaa" << "\n";



}