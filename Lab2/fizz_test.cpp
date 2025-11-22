#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <random>
#include <regex>
#include <algorithm>
#include <utility>

using namespace std;

struct AFA {
    set<string> states;
    string start;
    map<string, map<string, set<string>>> transitions;
    set<string> accepting;
    map<string, string> state_type; 

    set<string> delta(const string& q, const string& a) {
        set<string> result;

        if (transitions.count(q)) {
            if (transitions[q].count(a))
                result.insert(transitions[q][a].begin(), transitions[q][a].end());
            if (transitions[q].count(".") && a != "eps")
                result.insert(transitions[q]["."].begin(), transitions[q]["."].end());
        }

        return result;
    }


    bool accepts_helper(const string& q, int pos, const string& word,
                        map<pair<string,int>, bool>& memo) {

        if (states.count(q) == 0) return false;

        auto key = make_pair(q,pos);
        if (memo.count(key)) return memo[key];

        bool at_end = pos >= (int)word.size();
        string typ = state_type.count(q) ? state_type[q] : "OR";

        set<string> eps_succ = delta(q, "eps");
        set<string> sym_succ;
        if (!at_end) {
            string a(1, word[pos]);
            sym_succ = delta(q, a);
        }


        if (typ == "OR") {
            if (at_end && accepting.count(q)) return memo[key] = true;

            for (auto &p : eps_succ)
                if (accepts_helper(p, pos, word, memo))
                    return memo[key] = true;
            if (!at_end) {
                for (auto &p : sym_succ)
                    if (accepts_helper(p, pos+1, word, memo))
                        return memo[key] = true;
            }
            return memo[key] = false;
        } else {
            for (auto &p : eps_succ)
                if (!accepts_helper(p, pos, word, memo))
                    return memo[key] = false;
            if (at_end) {return memo[key] = accepting.count(q);}
            else {
                if (!sym_succ.empty()) {
                    for (auto &p : sym_succ)
                        if (!accepts_helper(p, pos+1, word, memo))
                            return memo[key] = false;
                } else if (eps_succ.empty()) {
                    return memo[key] = false;
                }
            }
            return true;
        }
    }

    bool accepts(const string& word) {
        map<pair<string,int>, bool> memo;
        return accepts_helper(start, 0, word, memo);
    }
};

string random_word(const vector<char>& alphabet, int max_len, mt19937& rng) {
    uniform_int_distribution<int> len_dist(0, max_len);
    int length = len_dist(rng);
    string w;
    uniform_int_distribution<int> idx_dist(0, alphabet.size()-1);
    for (int i=0;i<length;i++)
        w.push_back(alphabet[idx_dist(rng)]);
    return w;
}

int main() {
    const string EPS = "eps";
    const string ALL = ".";
    mt19937 rng(random_device{}());

    // ---------------- DFA -------------------
    set<string> statesDFA = {"q0","q1","q2","q3","q4","q5","q6"};
    map<string,map<string,set<string>>> transitionsDFA = {
        {"q0", {{"a",{"q1"}}, {"b",{"q1"}}}},
        {"q1", {{"a",{"q2"}}, {"b",{"q3"}}}},
        {"q2", {{"a",{"q4"}}, {"b",{"q5"}}}},
        {"q3", {{"a",{"q3"}}, {"b",{"q3"}}}},
        {"q4", {{"a",{"q2"}}, {"b",{"q2"}}}},
        {"q5", {{"a",{"q2"}}, {"b",{"q6"}}}},
        {"q6", {{"a",{"q1"}}, {"b",{"q5"}}}}
    };
    set<string> acceptingDFA = {"q0","q2","q6"};
    map<string,string> state_type_empty;

    AFA dfa{statesDFA, "q0", transitionsDFA, acceptingDFA, state_type_empty};

    // ---------------- NFA -------------------
    set<string> statesNFA = {"q0","q1","q2","q3","q4","q5"};
    map<string,map<string,set<string>>> transitionsNFA = {
        {"q0", {{"a",{"q1"}}, {"b",{"q1"}}}},
        {"q1", {{"a",{"q2"}}}},
        {"q2", {{"a",{"q1","q3"}}, {"b",{"q1","q4"}}}},
        {"q3", {{"b",{"q2"}}}},
        {"q4", {{"b",{"q5"}}}},
        {"q5", {{"a",{"q1"}}, {"b",{"q1","q4"}}}}
    };
    set<string> acceptingNFA = {"q0","q2","q5"};
    AFA nfa{statesNFA, "q0", transitionsNFA, acceptingNFA, state_type_empty};

    // ---------------- AFA -------------------
    set<string> statesAFA = {"&","p0","p1","T","q0","q1","q2","q3","q4"};
    map<string,map<string,set<string>>> transitionsAFA = {
        {"&",  {{EPS,{"q0"}}, {"a",{"p0"}}, {"b",{"p0"}}}},
        {"p0", {{"a",{"p1"}}, {"b",{"T"}}}},
        {"p1", {{"a",{"p1"}}, {"b",{"p1"}}}},
        {"T",  {{"a",{"T"}},  {"b",{"T"}}}},
        {"q0", {{"a",{"q1"}}, {"b",{"q2"}}}},
        {"q1", {{"a",{"q0"}}, {"b",{"q0"}}}},
        {"q2", {{"a",{"q0"}}, {"b",{"q3"}}}},
        {"q3", {{"a",{"q4"}}, {"b",{"q2"}}}},
        {"q4", {{"a",{"q0"}}, {"b",{"T"}}}}
    };
    map<string,string> state_typeAFA = {{"&","AND"}};
    set<string> acceptingAFA = {"&","q0","q3", "p1"};
    AFA afa{statesAFA, "&", transitionsAFA, acceptingAFA, state_typeAFA};

    // ---------------- reg_extended -------------------
    set<string> states_reg_extended= {"&","p0","q0","q1","q2","q3","q4"};
    map<string,map<string,set<string>>> transitions_reg_extended = {
        {"&", {{ALL,{"q0"}}, {EPS,{"p0"}}}},
        {"p0", {{"a",{"p0"}}, {"b",{"p0"}}}},
        {"q0", {{"a",{"q1"}}}},
        {"q1", {{"a",{"q2"}}, {"b",{"q3"}}, {ALL,{"q0"}}}},
        {"q2", {{"b",{"q1"}}}},
        {"q3", {{"b",{"q4"}}}},
        {"q4", {{ALL,{"q0"}}, {"b",{"q3"}}}}
    };
    map<string,string> state_type_reg_extended= {{"&","AND"}};
    set<string> accepting_reg_extended = {"&","q1","q4", "p0"};
    AFA reg_extended {states_reg_extended, "&", transitions_reg_extended, accepting_reg_extended, state_type_reg_extended};
    // ---------------- reg_original -------------------
    string reg_original = "((aa|ba)(ab)*(bb)*)*";

    int n = 10;
    int max_len = 12;
    //условно все возможные символы
    vector<char> alphabet_all;
    for(char c='a';c<='z';c++) alphabet_all.push_back(c);
    vector<char> alphabet_ab = {'a','b'};

    for(int i=0;i<n;i++){
        string w_ab = random_word(alphabet_ab, max_len, rng);
        string w_all = random_word(alphabet_all, max_len, rng);

        for(string w : {w_ab,w_all}) {
            regex re0(reg_original);

            bool expected = regex_match(w, re0);
            bool result_dfa = dfa.accepts(w);
            bool result_nfa = nfa.accepts(w);
            bool result_afa = afa.accepts(w);
            bool result_reg_extended = reg_extended.accepts(w);

            if (!(result_dfa == result_nfa && result_nfa == result_afa 
                && result_afa == result_reg_extended && result_reg_extended == expected)) {
                cout << "Discrepancy on the word - " << w << ":\n";
                cout << "  reg0      : " << expected << "\n";
                cout << "  DFA       : " << result_dfa << "\n";
                cout << "  NFA       : " << result_nfa << "\n";
                cout << "  AFA       : " << result_afa << "\n";
                cout << "  reg1      : " << result_reg_extended << "\n";
            } else {
                cout << "The result matched. Word: " << w << " - " << expected << "\n";
            }
        }
    }

    return 0;
}
