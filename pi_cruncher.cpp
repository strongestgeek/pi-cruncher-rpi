#include <iostream>
#include <fstream>
#include <chrono>
#include <future>
#include <gmpxx.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>

const mpz_class C3_OVER_24("10939058860032000");
const mpz_class L_START("13591409");
const mpz_class L_STEP("545140134");

struct BSResult {
    mpz_class P, Q, R;
};

// Helper to handle data received from libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_time_string() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", std::localtime(&now_c));
    return std::string(buf);
}

BSResult binary_split(unsigned long a, unsigned long b) {
    BSResult res;
    if (b - a == 1) {
        if (a == 0) {
            res.P = 1; res.Q = 1;
        } else {
            res.P = mpz_class(6 * a - 5) * (2 * a - 1) * (6 * a - 1);
            mpz_class a3 = mpz_class(a) * a * a;
            res.Q = a3 * C3_OVER_24; 
        }
        res.R = res.P * (L_START + L_STEP * a); 
        if (a % 2 != 0) res.R = -res.R;
        return res;
    }

    unsigned long m = (a + b) / 2;
    if (b - a > 5000) {
        auto left_future = std::async(std::launch::async, binary_split, a, m);
        BSResult right = binary_split(m, b);
        BSResult left = left_future.get(); 
        
        res.P = left.P * right.P; res.Q = left.Q * right.Q;
        res.R = right.Q * left.R + left.P * right.R;
    } else {
        BSResult left = binary_split(a, m);
        BSResult right = binary_split(m, b);
        
        res.P = left.P * right.P; res.Q = left.Q * right.Q;
        res.R = right.Q * left.R + left.P * right.R;
    }
    return res;
}

int main() {
    unsigned long places;
    std::cout << "How many decimal places of Pi do you want to calculate? ";
    std::cin >> places;

    std::string start_date = get_time_string();
    auto start_time = std::chrono::high_resolution_clock::now();

    unsigned long terms = places / 14 + 1;
    BSResult res = binary_split(0, terms);

    mpf_set_default_prec(places * 3.33 + 10); 
    mpf_class C(10005);
    C = sqrt(C) * 426880;
    mpf_class pi = (C * mpf_class(res.Q)) / mpf_class(res.R);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::string end_date = get_time_string();
    std::chrono::duration<double> duration = end_time - start_time;

    struct utsname os_info;
    uname(&os_info);
    
    struct sysinfo mem_info;
    sysinfo(&mem_info);
    double total_ram_gb = (double)mem_info.totalram * mem_info.mem_unit / (1024 * 1024 * 1024);

    mp_exp_t exp;
    std::string pi_str = pi.get_str(exp, 10, places + 5);
    
    std::string last_50 = "Calculation too short for 50 digit spot check.";
    if (places >= 50) {
        last_50 = pi_str.substr(places - 49, 50); 
    }

    std::string api_status = "Verification Skipped (Calculation too short)";
    std::string api_digits = "N/A";

    // --- NEW: AUTO VERIFICATION VIA GOOGLE API ---
    if (places >= 50) {
        std::cout << "Calculating complete! Contacting Google Pi Delivery API for verification...\n";
        CURL* curl;
        CURLcode curl_res;
        std::string readBuffer;

        curl = curl_easy_init();
        if(curl) {
            unsigned long start_index = places - 49;
            std::string url = "https://api.pi.delivery/v1/pi?start=" + std::to_string(start_index) + "&numberOfDigits=50";
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if(curl_res == CURLE_OK) {
                // Quick and dirty JSON parsing to extract the 'content' string
                size_t pos = readBuffer.find("\"content\":\"");
                if (pos != std::string::npos) {
                    api_digits = readBuffer.substr(pos + 11, 50);
                    if (api_digits == last_50) {
                        api_status = "PASSED (Matches Google API)";
                    } else {
                        api_status = "FAILED (Mismatch Detected)";
                    }
                } else {
                    api_status = "API Error: Could not parse response.";
                }
            } else {
                api_status = "API Error: Network connection failed.";
            }
        }
    }

    std::string filename = "pi_validation_" + std::to_string(places) + ".txt";
    std::ofstream outfile(filename);
    
    outfile << "========================================================\n";
    outfile << "       Custom Pi Cruncher Validation File\n";
    outfile << "========================================================\n\n";
    
    outfile << "Operating System:      " << os_info.sysname << " " << os_info.release << "\n";
    outfile << "Hardware Node:         " << os_info.machine << " (Raspberry Pi)\n";
    outfile << "Total Memory:          " << std::fixed << std::setprecision(2) << total_ram_gb << " GB\n\n";

    outfile << "Constant:              Pi\n";
    outfile << "Algorithm:             Chudnovsky (1988)\n";
    outfile << "Decimal Digits:        " << places << "\n";
    outfile << "Threading Mode:        std::async (Dynamic Multi-threading)\n\n";

    outfile << "Start Date:            " << start_date << "\n";
    outfile << "End Date:              " << end_date << "\n";
    outfile << "Total Computation Time:" << std::fixed << std::setprecision(4) << duration.count() << " seconds\n\n";

    outfile << "--- Online Spot Check (Last 50 Digits) ---\n";
    outfile << "Calculated: " << last_50 << "\n";
    outfile << "Google API: " << api_digits << "\n";
    outfile << "Status:     " << api_status << "\n";
    
    outfile.close();

    std::cout << "\nDone! Calculated in " << duration.count() << " seconds.\n";
    std::cout << "Verification Status: " << api_status << "\n";
    std::cout << "Validation report saved to " << filename << "\n";

    return 0;
}
