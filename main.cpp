#include "bits/stdc++.h"
using namespace std;
/*
1. split into CC
2. solve each CC
3. merge CC
easier said than done;

step 1
    input in 2d array (problem push_back(causes memory copying many time -> reserve (for 2d?)))
    ...(future problem)

step 2
    bfs
*/
void solve() {
    //step 1
    int n,m;
    cin>>n>>m;
    vector<int> rating(n),degree(n),ans(n);
    for(int& i: rating) cin>>i;
    vector<vector<int>> cc(n+1);
    vector<pair<int,int>> edges;
    edges.reserve(m);
    for(int i=0;i<m;i++) {
        int u,v;
        cin>>u>>v;
        edges.push_back({u,v});
        degree[u]++;
        degree[v]++;
    }
    for(int i=1;i<=n;i++) {
        cc[i].reserve(degree[i]);
    }
    for(auto& e: edges) {
        int u=e.first,v=e.second;
        cc[u].push_back(v);
        cc[v].push_back(u);
    }
    //step 2
    int id =0;
    vector<bool> vis(n+1,false);
    while(id<n) {
        while(id<n && vis[id+1]) id++;
        if(id==n) break;
        id++;
        queue<int> q;
        q.push(id);
        vector<int> comp;
        while(!q.empty()) {
            int u=q.front();
            q.pop();
            if(vis[u]) continue;
            vis[u]=true;
            comp.push_back(u);
            for(int v: cc[u]) {
                if(!vis[v]) q.push(v);
            }
        }
        sort(comp.begin(),comp.end(),[&](int a,int b){
            return rating[a-1]>rating[b-1];
        });
        for(int i=0;i<comp.size();i++) {
            ans[comp[i]-1]=i+1;
        }
    }
}
int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);
    solve();
}