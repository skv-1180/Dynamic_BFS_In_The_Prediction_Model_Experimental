/**
 *    author:  Yash Joshi
**/

#include<bits/stdc++.h>
using namespace std;

#include<ext/pb_ds/assoc_container.hpp>
#include<ext/pb_ds/tree_policy.hpp>
using namespace __gnu_pbds;
template<class T> using oset = tree<T, null_type, less<T>,rb_tree_tag,tree_order_statistics_node_update>;
template<class T> using omultiset = tree<T, null_type, less_equal<T>,rb_tree_tag,tree_order_statistics_node_update>;

#define cl() cerr<<endl;
#define debug(x...)if (#x == "TEST"){cerr << '\n';} cerr << "(Line " << __LINE__ << "):[" << #x << "]=>";_print_(x);cerr << endl;
#define graph(x, y)cerr << "(Line " << __LINE__ << "):[" << #x << "]=>" << endl;__print(x, y);cerr << endl;
#define debugptr(x, y)cerr << "(Line " << __LINE__ << "):[" << #x << "] ";__print_(x, y);cerr << endl;
template <class T>void _print_(T x);
template <class T1, class T2>void _print_(pair<T1, T2> x);
// template <class T>void _print_(T *x, int n);
// template <class T>void _print_(vector<T> x);
// template <class T>void _print_(vector<T> x, int l);
template <class T>void _print_(vector<vector<T>> x);
template <class T1, class T2>void _print_(map<T1, T2> x);
template <class T>void _print_(queue<T> x);
template <class T>void _print_(deque<T> x);
template <class T>void _print_(set<T> x);
template <class T>void _print_(multiset<T> x);
template <class T>void _print_(unordered_set<T> x);
template <class T>void _print_(priority_queue<T>x);
template <class T>void _print_(priority_queue<T,vector<T>,greater<T>>x);

template <class T>void _print_(stack<T> x);
void _print_(long long x){
    if(sizeof(x)==sizeof(long long)){
        if(abs(x)>=1e18+10){
            if(x<0)cerr<<'-';
            cerr<<'I';
        }
        else cerr<<x;
    }
    else{
        cerr<<x;
    }
}
template <class T>void _print_(T x){
        cerr<<x;
}
template <class T1, class T2>void _print_(pair<T1, T2> x){
    cerr << "{";
    _print_(x.first);
    cerr << ",";
    _print_(x.second);
    cerr << "}";
}

template <class T>void __print_(T x[], int l){
    int j = 0;
    cerr << "{ ";
    for(int i=0;;i++){
        cerr << j++ << ":";
        _print_(x[i]);
        cerr << ' ';
        if(j==l)break;
    }
    cerr << "}";
}
template <class T>void __print_(vector<T> x,int l){
    if(l==-1){
        cerr<<endl;
        for(int i = 1; i < x.size(); i++) {
            _print_(x[i]);
            if(__builtin_popcount(i + 1) == 1) cerr << endl;
        }
        cerr<<endl;
        return;
    }
    cerr << "{ ";
    int j = 0;
    for (T i : x){
        cerr << j++ << ":";
        _print_(i);
        cerr << ' ';
        if(j==l)break;
    }
    cerr << "}";
}
template <class T>void _print_(vector<T> x){
    cerr << "{ ";
    int j = 0;
    for (T i : x){
        cerr << j++ << ":";
        _print_(i);
        cerr << ' ';
    }
    cerr << "}";
}
template<class T>void _print_(vector<vector<T>>v){
    cerr<<'{'<<endl;int j=0;
    for(vector<T> &arr:v){
        cerr<<'\t'<<j++<<':';
        _print_(arr);
        cl();
    }
    cerr<<'}';
}
template<class T>void _print_(oset<T>v){
    cerr<<'{';int j=0;
    for(int i = 0;i<v.size();i++){
        cerr<<j++<<':';
        _print_(*v.find_by_order(i));
        cerr << ' ';
    }
    cerr<<'}';
}
template<class T>void _print_(omultiset<T>v){
    cerr<<'{';int j=0;
    for(int i = 0;i<v.size();i++){
        cerr<<j++<<':';
        _print_(*v.find_by_order(i));
        cerr << ' ';
    }
    cerr<<'}';
}
template <class T1, class T2>void _print_(map<T1, T2> x){
    cerr << "[ ";
    for (auto i : x){
        cerr << "{";
        _print_(i.first);
        cerr << ":";
        _print_(i.second);
        cerr << "} ";
    }
    cerr << "]";
}
template <class T>void _print_(queue<T> x){
    if(x.empty()){
        cerr<<"THIS QUEUE IS EMPTY.";return;
    }
    cerr<<"{ FRONT:";_print_(x.front());cerr<<' ';x.pop();
    int j=1,y=x.size();
    while(x.size()>1){
        cerr<<j++<<':';
        _print_(x.front());x.pop();
        cerr<<' ';
    }
    if(y){cerr<<"BACK:";_print_(x.front());cerr<<' ';}
    cerr<<'}';
}
template <class T>void _print_(deque<T> x){
    if(x.empty()){
        cerr<<"THIS DEQUE IS EMPTY.";return;
    }
    cerr<<"{ FRONT:";_print_(x.front());cerr<<' ';x.pop_front();
    int j=1,y=x.size();
    while(x.size()>1){
        cerr<<j++<<':';
        _print_(x.front());x.pop_front();
        cerr<<' ';
    }
    if(y){cerr<<"BACK:";_print_(x.front());cerr<<' ';}
    cerr<<'}';
}
template <class T>void _print_(set<T> x){
    cerr << "{ ";
    int j = 0;
    for (T i : x){
        cerr << j++ << ":";
        _print_(i);
        cerr << " " ;
    }
    cerr << "}";
}
template <class T>void _print_(multiset<T> x){
    cerr << "{";
    int j = 0;
    for (T i : x){
        cerr << j++ << ":";
        _print_(i);
        cerr << " ";
    }
    cerr << "}";
}
template <class T>void _print_(unordered_set<T> x){
    cerr << "{";
    int j = 0;
    for (T i : x){
        cerr << j++ << ":";
        _print_(i);
        cerr << " ";
    }
    cerr << "}";
}
template <class T>void _print_(priority_queue<T> x){
    if(x.empty()){
        cerr<<"THIS HEAP IS EMPTY.";return;
    }
    cerr<<"{ VALUE:";_print_(x.top());cerr<<' ';x.pop();
    int j=1,y=x.size();
    while(!x.empty()){
        cerr<<j++<<':';
        _print_(x.top());x.pop();
        cerr<<' ';
    }
    cerr<<'}';
}
template <class T>void _print_(priority_queue<T,vector<T>,greater<T>> x){
    if(x.empty()){
        cerr<<"THIS HEAP IS EMPTY.";return;
    }
    cerr<<"{ VALUE:";_print_(x.top());cerr<<' ';x.pop();
    int j=1,y=x.size();
    while(!x.empty()){
        cerr<<j++<<':';
        _print_(x.top());x.pop();
        cerr<<' ';
    }
    cerr<<'}';
}

template <class T>void _print_(stack<T> x){
    int y=x.size(),j=1;
    if(!y)cerr<<"THIS STACK IS EMPTY!";
    else{
        cerr<<"{ TOP:";_print_(x.top());cerr<<' ';x.pop();
        y--;
        while(y--){
            cerr<<++j<<':';_print_(x.top());cerr<<' ';x.pop();
        }
        cerr<<'}';
    }
}
template <class T1, class... T2>void _print_(T1 x, T2... y){
    _print_(x);
    if (sizeof...(y))cerr << ",";
    _print_(y...);
}
template <class T>void __print(vector<vector<T>> adj, T source) {
    int N = 10000010;
    vector<bool> vis(N);
    vector<int> dis(N);
    queue<T> q;
    q.push(source);
    vis[source] = true;
    while (!q.empty()){
        T v = q.front();
        string s(4 * dis[v], ' ');
        cerr << s;
        _print_(v);
        cerr << "-->";
        q.pop();
        for (auto child : adj[v]){
            if (vis[child])continue;
            _print_(child);
            cerr << " ";
            vis[child] = true;
            q.push(child);
            dis[child] = dis[v] + 1;
        }
        cerr << endl;
    }
}