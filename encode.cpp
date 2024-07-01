#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

const std::string rockyou_path = "/home/nabuki/datasets/rockyou.txt";

// Define the character set used in passwords
const std::string characters = 
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

std::unordered_map<char, int> char_to_index;

std::vector<int> tokenize(const std::string &s) {
    std::vector<int> tokens;
    for (char ch : s) {
        if (char_to_index.find(ch) != char_to_index.end()) {
            tokens.push_back(char_to_index[ch]);
        }
    }
    return tokens;
}

class Node {
public:
    std::unordered_map<int, Node*> _forward;
    int _visits;

    Node() : _visits(0) {}

    Node* visit(int token) {
        if (_forward.find(token) == _forward.end()) {
            _forward[token] = new Node();
        }
        _forward[token]->_visits++;
        return _forward[token];
    }

    std::vector<float> normalized_transition_weights() {
        int s = 0;
        for (const auto& pair : _forward) {
            s += pair.second->_visits;
        }

        if (s == 0) {
            return std::vector<float>();
        }

        std::vector<float> res(characters.size(), 0.0);
        for (const auto& pair : _forward) {
            res[pair.first] = static_cast<float>(pair.second->_visits) / s;
        }

        return res;
    }

    void write_normalized(std::ofstream &sink) {
        auto transitions = normalized_transition_weights();

        if (transitions.empty()) {
            sink.write(std::vector<char>(characters.size(), 0).data(), characters.size());
            return;
        }

        std::vector<char> quantized_transitions(characters.size(), 0);
        std::transform(transitions.begin(), transitions.end(), quantized_transitions.begin(), [](float v) {
            return static_cast<char>(v * 255);
        });

        sink.write(quantized_transitions.data(), quantized_transitions.size());

        for (size_t i = 0; i < quantized_transitions.size(); ++i) {
            if (quantized_transitions[i] != 0) {
                _forward[i]->write_normalized(sink);
            }
        }
    }
};

int main() {
    for (size_t i = 0; i < characters.size(); ++i) {
        char_to_index[characters[i]] = i;
    }

    Node root;
    std::ifstream file(rockyou_path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << rockyou_path << std::endl;
        return 1;
    }

    std::cout << "Enter the number of lines to process: ";
    size_t max_lines;
    std::cin >> max_lines;

    std::string line;
    size_t line_count = 0;
    auto start_time = std::chrono::steady_clock::now();
    auto last_time = start_time;

    while (std::getline(file, line) && line_count < max_lines) {
        Node* curr = &root;
        auto tokens = tokenize(line);
        for (int token : tokens) {
            curr = curr->visit(token);
        }
        line_count++;

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_time).count();
        if (elapsed_seconds >= 1) {
            std::cout << "Processed lines: " << line_count << std::endl;
            last_time = current_time;
        }
    }
    file.close();

    std::cout << "Writing..." << std::endl;
    std::ofstream out("res.pfx", std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: res.pfx" << std::endl;
        return 1;
    }

    root.write_normalized(out);
    out.close();

    return 0;
}
