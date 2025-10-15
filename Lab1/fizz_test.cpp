#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>

using namespace std;

class CompleteRewritingSystem {
private:
    vector<char> alphabet = {'a', 'b', 'c', 'd', 'p', 'q'};
    
    // Исходная система правил
    vector<pair<string, string>> original_rules = {
        {"apbc", "caqdbapbap"},
        {"paqd", "daqdbapbap"}, 
        {"ccpp", "adaqdqa"},
        {"dpd", "pdp"}
    };
    
    // Модифицированная система с учетом порядка
    vector<pair<string, string>> modified_rules = {
        {"caqdbapbap", "apbc"},
        {"daqdbapbap", "paqd"},
        {"adaqdqa", "ccpp"},
        {"pdp", "dpd"}
    };
    
    vector<pair<string, string>> pattern_rules;

    int random_int(int min, int max) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> distrib(min, max);
        return distrib(gen);
    }

public:
    CompleteRewritingSystem() {
        generate_pattern_rules(10);
    }
    
    void generate_pattern_rules(int max_n) {
        pattern_rules.clear();
        for (int n = 1; n <= max_n; ++n) {
            string lhs = "p" + string(n + 1, 'd') + "pd";
            string rhs = "dpdd" + string(n, 'p');
            pattern_rules.emplace_back(lhs, rhs);
        }
    }
    
    vector<pair<string, string>> get_original_system() {
        return original_rules;
    }
    
    vector<pair<string, string>> get_modified_system() {
        auto result = modified_rules;
        result.insert(result.end(), pattern_rules.begin(), pattern_rules.end());
        return result;
    }
    
    unordered_set<string> neighbors(const string& word, 
                                    const vector<pair<string, string>>& rules) {
        unordered_set<string> neighbors;
        
        for (const auto& rule : rules) {
            const string& lhs = rule.first;
            const string& rhs = rule.second;
            
            size_t pos = 0;
            while ((pos = word.find(lhs, pos)) != string::npos) {
                string new_word = word.substr(0, pos) + rhs + word.substr(pos + lhs.length());
                neighbors.insert(new_word);
                pos += 1;
            }
        }
        
        return neighbors;
    }
    
    pair<bool, vector<string>> find_path(const string& start_word, 
                                         const string& target_word,
                                         const vector<pair<string, string>>& rules) {
        if (start_word == target_word) {
            return {true, {start_word}};
        }
        
        queue<string> q;
        unordered_map<string, string> parent;
        unordered_set<string> visited;
        
        q.push(start_word);
        parent[start_word] = "";
        visited.insert(start_word);
        
        while (!q.empty()) {
            string current = q.front();
            q.pop();
            
            auto current_neighbors = neighbors(current, rules);
            for (const auto& neighbor : current_neighbors) {
                if (visited.find(neighbor) != visited.end()) {
                    continue;
                }
                
                visited.insert(neighbor);
                parent[neighbor] = current;
                
                if (neighbor == target_word) {
                    vector<string> path;
                    string node = neighbor;
                    while (!node.empty()) {
                        path.push_back(node);
                        node = parent[node];
                    }
                    reverse(path.begin(), path.end());
                    return {true, path};
                }
                
                q.push(neighbor);
            }
        }
        
        return {false, {}};
    }
    
    string generate_random_word(int min_len, int max_len) {
        int length = random_int(min_len, max_len);
        string word;
        for (int i = 0; i < length; ++i) {
            word += alphabet[random_int(0, alphabet.size() - 1)];
        }
        return word;
    }
    
    void fuzz_test(int count_words, int min_len_word, int max_len_word) {
        int index_word = 1;
        while (count_words >= index_word) {
            string start_word = generate_random_word(15, 20);
            
            string current = start_word;
            int steps = random_int(4, 7);
            
            auto original_system = get_original_system();
            for (int i = 0; i < steps; ++i) {
                auto current_neighbors = neighbors(current, original_system);
                if (current_neighbors.empty()) {
                    break;
                }
                
                vector<string> neighbors_vec(current_neighbors.begin(), current_neighbors.end());
                current = neighbors_vec[random_int(0, neighbors_vec.size() - 1)];
            }
            
            string target_word = current;
            
            if (start_word != target_word) {
                auto modified_system = get_modified_system();
                
                // Проверяем прямую достижимость
                auto [found_forward, path_forward] = find_path(start_word, target_word, modified_system);
                
                // Проверяем обратную достижимость
                auto [found_backward, path_backward] = find_path(target_word, start_word, modified_system);
                
                cout << "Test " << index_word << ":\n";
                string state = "-";
                
                if (found_forward) {
                    state += ">";
                    cout << "The word forward is achievable w -> w': ";
                    for (size_t i = 0; i < path_forward.size(); ++i) {
                        if (i > 0) cout << " -> ";
                        cout << "'" << path_forward[i] << "'";
                    }
                    cout << "\n";
                }
                
                if (found_backward) {
                    state = "<" + state;
                    cout << "The word backward is achievable w' -> w: ";
                    for (size_t i = 0; i < path_backward.size(); ++i) {
                        if (i > 0) cout << " -> ";
                        cout << "'" << path_backward[i] << "'";
                    }
                    cout << "\n";
                }
                
                cout << "'" << start_word << "' " << state << " '" << target_word << "'\n\n";
                index_word++;
            }
        }
    }
};

int main() {
    CompleteRewritingSystem system;
    system.fuzz_test(5, 15,20);
    return 0;
}