#include <iostream>
#include <string>
#include <curl/curl.h>
#include <sstream>
#include <regex>
#include <iomanip>

// Callback function to write response data
// size_t unsigned integer data type that is used to represent the size of objects in bytes. 
// WriteCallback: functions to be passed as arguments
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to get latest BTC price from Bitquery API
double getLatestBTCPrice() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    double price = 0.0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "https://streaming.bitquery.io/eap");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer ory_at_");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        const char *data = "{\"query\":\"{\\n  Trading {\\n    Tokens(\\n      where: {Currency: {Id: {is: \\\"bid:bitcoin\\\"}}, Interval: {Time: {Duration: {eq: 1}}}}\\n      limit: {count: 1}\\n      orderBy: {descending: Block_Time}\\n    ) {\\n      Token {\\n        Address\\n        Id\\n        IsNative\\n        Name\\n        Network\\n        Name\\n        Symbol\\n        TokenId\\n      }\\n      Block {\\n        Date\\n        Time\\n        Timestamp\\n      }\\n      Interval {\\n        Time {\\n          Start\\n          Duration\\n          End\\n        }\\n      }\\n      Volume {\\n        Base\\n        Quote\\n        Usd\\n      }\\n      Price {\\n        IsQuotedInUsd\\n        Ohlc {\\n          Close\\n          High\\n          Low\\n          Open\\n        }\\n        Average {\\n          ExponentialMoving\\n          Mean\\n          SimpleMoving\\n          WeightedSimpleMoving\\n        }\\n      }\\n    }\\n  }\\n}\\n\",\"variables\":\"{}\"}";
        
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        if(res == CURLE_OK) {
            // Parse JSON response to extract Close price
            // Look for "Close" field in the Price.Ohlc section
            std::regex priceRegex("\"Close\"\\s*:\\s*([0-9]+\\.?[0-9]*)");
            std::smatch match;
            
            if (std::regex_search(readBuffer, match, priceRegex)) {
                try {
                    price = std::stod(match[1].str());
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing price: " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Price not found in response. Response: " << readBuffer << std::endl;
            }
        } else {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return price;
}

int main() {
    std::cout << "Fetching latest BTC price from Bitquery API...\n" << std::endl;
    
    double btcPrice = getLatestBTCPrice();
    
    if (btcPrice > 0.0) {
        std::cout<<"latest price $ "<<btcPrice << std::endl;
    } else {
        std::cout << "Failed to fetch BTC price." << std::endl;
        return 1;
    }
    
    return 0;
}

