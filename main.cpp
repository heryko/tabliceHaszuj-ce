#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <string>
#include <algorithm>
#include <random>
#include <iomanip>
#include <numeric>
#include <stdexcept>
#include <climits>

template <typename T>
class HashTable {
private:
    int m;  // Hash table size
    int p;  // Prime number for universal hashing
    double A;
    int b, c;
    std::vector<std::vector<T>> table;

public:
    HashTable(int size) : m(size), table(size) {
        if (size <= 0) {
            throw std::invalid_argument("Hash table size must be > 0");
        }
        p = findNextPrime(m);

        std::random_device rd;
        std::mt19937 gen(rd());
        A = 0.6180339887;
        b = std::uniform_int_distribution<>(1, p-1)(gen);
        c = std::uniform_int_distribution<>(0, p-1)(gen);
    }

    static int findNextPrime(int n) {
        if (n <= 1) return 2;
        int candidate = n % 2 == 0 ? n + 1 : n;
        while (candidate < INT_MAX) {
            if (isPrime(candidate)) return candidate;
            candidate += 2;
        }
        throw std::overflow_error("Cannot find prime number");
    }

    static bool isPrime(int num) {
        if (num <= 1) return false;
        if (num <= 3) return true;
        if (num % 2 == 0 || num % 3 == 0) return false;
        for (int i = 5; i*i <= num; i += 6) {
            if (num % i == 0 || num % (i+2) == 0) return false;
        }
        return true;
    }

    int hashDivision(const T& key) const {
        return (key % m + m) % m;
    }

    int hashMultiplication(const T& key) const {
        double value = std::fmod(static_cast<double>(key) * A, 1.0);
        return static_cast<int>(m * value) % m;
    }

   int hashMidSquare(const T& key) const {
    try {
        // U¿ycie uint64_t -> brak przepełnienia
        uint64_t square = static_cast<uint64_t>(key) * static_cast<uint64_t>(key);
        std::string str = std::to_string(square);

        // Zabezpieczenie: minimum 5 cyfr
        while (str.length() < 5) {
            str = '0' + str;
        }

        int mid = str.length() / 2;
        int start = std::max(0, mid - 1);
        int len = std::min(3, static_cast<int>(str.length()) - start);

        int mid_val = std::stoi(str.substr(start, len));
        return mid_val % m;

    } catch (...) {
        return 0;
    }
}

    int hashUniversal(const T& key) const {
        long long hash = (static_cast<long long>(b) * key + c) % p;
        return (hash % m + m) % m;
    }

    void addElement(T key, int (HashTable::*hashFunc)(const T&) const) {
        int index = (this->*hashFunc)(key);
        if (index < 0 || index >= m) {
            throw std::out_of_range("Invalid hash index");
        }
        table[index].push_back(key);
    }

    bool removeElement(T key, int (HashTable::*hashFunc)(const T&) const) {
        int index = (this->*hashFunc)(key);
        if (index < 0 || index >= m) return false;

        auto& bucket = table[index];
        auto it = std::find(bucket.begin(), bucket.end(), key);
        if (it != bucket.end()) {
            bucket.erase(it);
            return true;
        }
        return false;
    }

    void clear() {
        for (auto& bucket : table) {
            bucket.clear();
        }
    }

};

std::vector<int> generateOptData(int size) {
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    return data;
}

std::vector<int> generateMiddleData(int size) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, std::max(1, size/10));
    for (auto& val : data) {
        val = dis(gen);
    }
    return data;
}

std::vector<int> generateWorstData(int size) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, std::numeric_limits<int>::max());
    for (auto& val : data) {
        val = dis(gen);
    }
    return data;
}

template <typename T>
void benchmarkHashFunction(HashTable<T>& table, const std::vector<T>& data,int (HashTable<T>::*hashFunc)(const T&) const,const std::string& funcName,int repetitions) {

    using namespace std::chrono;

    double totalInsertTime = 0.0;
    double totalRemoveTime = 0.0;

for (int i = 0; i < repetitions; ++i) {
    HashTable<T> table(data.size() * 3);

    auto startInsert = high_resolution_clock::now();
    for (const auto& val : data) {
        table.addElement(val, hashFunc);
    }
    auto endInsert = high_resolution_clock::now();
    totalInsertTime += duration_cast<duration<double>>(endInsert - startInsert).count();

    auto startRemove = high_resolution_clock::now();
    for (const auto& val : data) {
        table.removeElement(val, hashFunc);
    }
    auto endRemove = high_resolution_clock::now();
    totalRemoveTime += duration_cast<duration<double>>(endRemove - startRemove).count();

}


    double avgInsertTimeMs = (totalInsertTime / repetitions) * 1000.0;
    double avgRemoveTimeMs = (totalRemoveTime / repetitions) * 1000.0;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << funcName << " | Avg Insert: " << avgInsertTimeMs << " ms"
              << " | Avg Remove: " << avgRemoveTimeMs << " ms\n";
}

int main() {
    std::vector<int> sizes = {10'000, 100'000, 1'000'000, 10'000'000};

    std::vector<std::string> dataTypes = {"opt", "mid", "worst"};

    for (int size : sizes) {
        std::cout << "\n==== SIZE: " << size << " ====\n";

        for (const auto& type : dataTypes) {
            std::cout << "Data type: " << type << "\n";

            std::vector<int> data;
            if (type == "opt") {
                data = generateOptData(size);
            } else if (type == "mid") {
                data = generateMiddleData(size);
            } else if (type == "worst") {
                data = generateWorstData(size);
            } else {
                std::cerr << "Unknown data type: " << type << std::endl;
                continue;
            }

            HashTable<int> table(size * 3);

            int repetitions = 100;

            benchmarkHashFunction(table, data, &HashTable<int>::hashDivision, "hashDivision     ", repetitions);
            benchmarkHashFunction(table, data, &HashTable<int>::hashMultiplication, "hashMultiplication", repetitions);
            benchmarkHashFunction(table, data, &HashTable<int>::hashMidSquare, "hashMidSquare    ", repetitions);
            benchmarkHashFunction(table, data, &HashTable<int>::hashUniversal, "hashUniversal    ", repetitions);
        }
    }

    return 0;
}

