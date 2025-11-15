#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <functional>
#include <map>

using namespace std;

class Invariant {
private:
    function<int(const string&)> test_word;
    string name;

public:
    Invariant(function<int(const string&)> test_func, const string& name) 
        : test_word(test_func), name(name) {}
    
    static Invariant from_linear_combination(const vector<char>& alphabet, 
                                           const vector<int>& linear_combination, 
                                           const string& name) {
        if (alphabet.size() != linear_combination.size()) {
            throw invalid_argument("The alphabet and the linear combination must have the same length");
        }
        
        function<int(const string&)> test_func = [alphabet, linear_combination](const string& word) {
            int total = 0;
            for (size_t i = 0; i < alphabet.size(); i++) {
                int count_char = count(word.begin(), word.end(), alphabet[i]);
                total += count_char * linear_combination[i];
            }
            return total;
        };
        
        return Invariant(test_func, name);
    }
    static Invariant get_matrix_invariant(const string& name) {
        vector<vector<int>> zero_matrix = {{0,0,0}, {0,0,0}, {0,0,0}};
        vector<vector<int>> identity_matrix = {{1,0,0}, {0,1,0}, {0,0,1}};
        vector<vector<int>> S1 = {{0,1,0}, {1,0,0}, {0,0,1}};
        vector<vector<int>> S2 = {{1,0,0}, {0,0,1}, {0,1,0}};
        
        map<char, vector<vector<int>>> char_to_matrix = {
            {'a', zero_matrix}, {'c', zero_matrix},
            {'b', identity_matrix}, {'q', identity_matrix},
            {'d', S1}, {'p', S2}
        };
        
        function<int(const string&)> test_func = [char_to_matrix](const string& word) {
            vector<vector<int>> result = {{1,0,0}, {0,1,0}, {0,0,1}};
            
            auto multiply_matrices = [](const vector<vector<int>>& a, 
                                      const vector<vector<int>>& b) -> vector<vector<int>> {
                vector<vector<int>> c(3, vector<int>(3, 0));
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        for (int k = 0; k < 3; k++) {
                            c[i][j] += a[i][k] * b[k][j];
                        }
                    }
                }
                return c;
            };
            
            for (char c : word) {
                auto it = char_to_matrix.find(c);
                if (it != char_to_matrix.end()) {
                    result = multiply_matrices(result, it->second);
                }
            }
        
            int matrix_trace = 0;
            for (int i = 0; i < 3; i++) {
                matrix_trace+=result[i][i];
            }
            return matrix_trace;
        };
        
        return Invariant(test_func, name);
    }
    
    bool test_rewriting_system(const vector<pair<string, string>>& rules, const string& name_system) const {
        for (const auto& rule : rules) {
            if (test_word(rule.first) != test_word(rule.second)) {
                cout << "In system " << name_system << " the invariant " << name << " is not satisfied!" << endl;
                return false;
            }
        }
        cout << "In system " << name_system << " the invariant " << name << " is satisfied." << endl;
        return true;
    }
    
    static bool test_system_list(const vector<Invariant>& invariants, 
                                const vector<pair<string, string>>& rules, 
                                const string& name_system) {
        for (const auto& invariant : invariants) {
            if (!invariant.test_rewriting_system(rules, name_system)) {
                return false;
            }
        }
        return true;
    }
    
    bool test_rewrite_path(const vector<string>& path) const {
        for (size_t i = 0; i < path.size() - 1; i++) {
            const string& l = path[i];
            const string& r = path[i + 1];
            if (test_word(l) != test_word(r)) {
                cout << "In the rewrite chain the invariant " << name << " is not satisfied!" << endl;
                cout << l << " -> " << r << endl;
                cout << name << "(" << l << ") = " << test_word(l) << endl;
                cout << name << "(" << r << ") = " << test_word(r) << endl;
                return false;
            }
        }
        cout << "In the rewrite chain the invariant " << name << " is satisfied." << endl;
        return true;
    }
    
    static bool test_path_list(const vector<Invariant>& invariants, const vector<string>& path) {
        for (const auto& invariant : invariants) {
            if (!invariant.test_rewrite_path(path)) {
                return false;
            }
        }
        return true;
    }
};

class CompleteRewritingSystem {
private:
    vector<char> alphabet = {'a', 'b', 'c', 'd', 'p', 'q'};
    
    vector<pair<string, string>> original_rules = {
        {"apbc", "caqdbapbap"},
        {"paqd", "daqdbapbap"}, 
        {"ccpp", "adaqdqa"},
        {"dpd", "pdp"}
    };
    
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
    
    vector<pair<string, string>> get_original_system() const {
        return original_rules;
    }
    
    vector<pair<string, string>> get_modified_system() const {
        auto result = modified_rules;
        result.insert(result.end(), pattern_rules.begin(), pattern_rules.end());
        return result;
    }
    
    unordered_set<string> neighbors(const string& word, 
                                    const vector<pair<string, string>>& rules) const {
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
                                         const vector<pair<string, string>>& rules) const {
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
    
    void invariant_test(int count_words, int min_len_word, int max_len_word) {
        vector<Invariant> invariants;
        invariants.push_back(Invariant::from_linear_combination(alphabet, {2,0,3,-2,-2,0}, "I1"));
        invariants.push_back(Invariant::from_linear_combination(alphabet, {2,-2,1,0,0,-2}, "I2"));
        invariants.push_back(Invariant(
            [](const string& word) { return count(word.begin(), word.end(), 'c') % 2; }, 
            "I3"
        ));
        invariants.push_back(Invariant::get_matrix_invariant("I4"));

        Invariant::test_system_list(invariants, get_original_system(), "T");
        cout << endl;
        Invariant::test_system_list(invariants, get_modified_system(), "T'");

        int index_word = 1;
        bool invariant_satisfied = true;
        
        while (count_words >= index_word) {
            string start_word = generate_random_word(min_len_word, max_len_word);
            
            string current = start_word;
            int steps = random_int(4, 7);
            vector<string> path = {start_word};
            
            for (int i = 0; i < steps; ++i) {
                auto current_neighbors = neighbors(current, get_modified_system());
                if (current_neighbors.empty()) {
                    break;
                }
                
                vector<string> neighbors_vec(current_neighbors.begin(), current_neighbors.end());
                current = neighbors_vec[random_int(0, neighbors_vec.size() - 1)];
                path.push_back(current);
            }
            
            if (path.size() > 1) {
                cout << endl;
                cout << index_word << ") ";
                for (size_t i = 0; i < path.size(); ++i) {
                    if (i > 0) cout << " -> ";
                    cout << path[i];
                }
                cout << endl;
                
                invariant_satisfied = Invariant::test_path_list(invariants, path) && invariant_satisfied;
                index_word++;
            }
        }
        
        if (invariant_satisfied) {
            cout << "\nAll chains passed the test." << endl;
        } else {
            cout << "\nNot all chains passed the test!" << endl;
        }
    }
};

int main() {
    CompleteRewritingSystem system;
    system.invariant_test(5,15,20);
    return 0;
}