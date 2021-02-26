#include <bits/stdc++.h>
using namespace std;

#define sqr(a) ((a) * (a))
#define debug(...) fprintf(stderr, __VA_ARGS__)
// #define debugl(...) fprintf(stderr, __VA_ARGS__)
// #define debugl(...) fprintf(flog, __VA_ARGS__)
#define debugl(...)

typedef long long ll;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;

#define MAXN 100100

int D, I, S, V, F;

map<string,int> streetIds;
vector<string> streetNames;

struct Street {
  int B, E, L;
  string name;
  int id;
  void read() {
    static char buffer[101];
    scanf("%d%d%s%d", &B, &E, buffer, &L);
    name = buffer;
    id = streetIds.size();
    streetIds[name] = id;
    streetNames[id] = name;
  }
};

struct Car {
  vector<int> path;
  void read() {
    int n;
    scanf("%d", &n);
    path.resize(n);
    for (int i = 0; i < n; ++i) {
      static char buffer[101];
      scanf("%s", buffer);
      path[i] = streetIds[buffer];
    }
  }
};

vector<pair<int,int>> adj[MAXN];
vector<pair<int,int>> adji[MAXN];

vector<Street> streets;
vector<Car> cars;

bool used[MAXN];

vector<vector<int>> simulate() {
  vector<vector<int>> X(I);
  for (int i = 0; i < I; ++i) {
    int cnt = 0;
    for (pii v : adji[i]) {
      if (used[v.second]) {
        cnt++;
      }
    }
    X[i].resize(cnt, -1);
  }
  vector<int> Xi(S, -1);

  // queue for each intersections
  vector<queue<int>> q;
  q.resize(S);

  // events is "releasing a car from an intersection queue"
  // events[T] = queue<street id to process>
  vector<queue<int>> events;
  events.resize(D + 1);

  // qevents is "a car entering an intersection queue"
  // qevents[T] = queue<street id, car id>
  vector<queue<pair<int, int>>> qevents;
  qevents.resize(D + 1);

  // events are triggered by:
  // - a car entering a queue

  // at T = 0, initialize all cars at the end of the streets
  vector<int> p; // index of car path
  p.resize(V);
  for (int i = 0; i < V; ++i) {
    int streetId = cars[i].path[p[i]];
    qevents[0].emplace(streetId, i);
  }

  vector<int> arrival;
  arrival.resize(V, -1);

  for (int t = 0; t <= D; ++t) {
    debugl("T = %d\\n", t);
    debugl("qevents.size() = %d\\n", (int)qevents[t].size());

    // insert cars into queue
    while (!qevents[t].empty()) {
      auto front = qevents[t].front();
      qevents[t].pop();

      int streetId = front.first;
      int carId = front.second;
      if (q[streetId].empty()) {
        events[t].push(streetId);
      }
      q[streetId].push(carId);

      debugl("\\t<streetId = %s, carId = %d>\\n", streetNames[streetId].c_str(), carId);
    }

    debugl("events.size() = %d\\n", (int)events[t].size());

    while (!events[t].empty()) {
      int id = events[t].front();
      events[t].pop();

      debugl("\\t<streetId = %s>\\n", streetNames[id].c_str());

      queue<int>& qs = q[id];
      assert(!qs.empty());

      int iid = streets[id].E;
      int _t = t;
      int cycle = X[iid].size();
      int mod = t % cycle;
      // check if id is assigned
      if (Xi[id] != -1) {
        debugl("\\talready assigned: %d\\n", Xi[id]);
        int cycle = X[iid].size();
        _t = cycle * (t / cycle) + Xi[id];
        if (_t < t) {
          _t += cycle;
        }
      } else if (X[iid][mod] == -1) {
        debugl("\\tempty!\\n");
        // assign it right away
        X[iid][mod] = id;
        Xi[id] = mod;
        debugl("\\tAssigning <streetId = %s> = %d in <intersectionId = %d>\\n", streetNames[id].c_str(), mod, iid);
      } else {
        // now what?
        // just defer to t+1 I guess
        _t = t + 1;
      }

      if (t != _t) {
        if (_t <= D) {
          events[_t].push(id);
        }
      }
      else {
        // take single car in the queue
        int c = qs.front(); qs.pop();
        p[c]++;
        assert(p[c] < cars[c].path.size());
        int snext = cars[c].path[p[c]];
        int inext = streets[snext].E;
        int tnext = t + streets[snext].L;

        debugl("\\tprocessing <car = %d>: snext = %s, inext = %d, tnext = %d\\n", c, streetNames[snext].c_str(), inext, tnext);

        if (tnext <= D) {
          // arrive!
          if (p[c] == cars[c].path.size() - 1) {
            arrival[c] = tnext;
          }
          else {
            qevents[tnext].emplace(snext, c);
          }
        }

        // reinsert self into events if queue is still not empty
        if (!qs.empty() && t < D) {
          events[t + 1].push(id);
        }

        debugl("\\tdone processing <car = %d>\\n", c);
      }
    }
  }

  // assign what's left randomly
  int maxslots = 0;
  for (int i = 0; i < I; ++i) {
    vector<int> slots;
    for (int j = 0; j < X[i].size(); ++j) {
      if (X[i][j] == -1) {
        slots.push_back(j);
      }
    }
    maxslots = max(maxslots, (int)slots.size());

    for (pii v : adji[i]) {
      if (used[v.second] && Xi[v.second] == -1) {
        Xi[v.second] = slots.back();
        X[i][slots.back()] = v.second;
        slots.pop_back();
      }
    }
  }
  debugl("maxslots = %d\\n", maxslots);

  debugl("arrival:\\n");
  int score = 0;
  for (int c = 0; c < V; ++c) {
    if (arrival[c] == -1) continue;
    assert(arrival[c] <= D);
    score += F + D - arrival[c];
    debugl("<car = %d> arrived at %d\\n", c, arrival[c]);
  }
  debug("score = %d\\n", score);

  for (int i = 0; i < I; ++i) {
    for (int j = 0; j < X[i].size(); ++j) {
      assert(X[i][j] != -1);
    }
  }

  return X;
}

int main(int argc, char** argv) {
  srand(0);

  scanf("%d%d%d%d%d", &D, &I, &S, &V, &F);

  streets.resize(S);
  streetNames.resize(S);
  for (int i = 0; i < S; ++i) {
    streets[i].read();
    adj[streets[i].B].emplace_back(streets[i].E, streets[i].id);
    adji[streets[i].E].emplace_back(streets[i].B, streets[i].id);
  }

  cars.resize(V);
  for (int i = 0; i < V; ++i) {
    cars[i].read();
    for (int s : cars[i].path) {
      used[s] = true;
    }
  }

  auto X = simulate();

  vector<vector<pair<int,int>>> ans;

  for (int v = 0; v < I; ++v) {
    vector<pair<int,int>> schedule;
    for (int x : X[v]) {
      schedule.emplace_back(x, 1);
    }
    ans.emplace_back(schedule);
  }
  assert(ans.size() == I);

  int nans = 0;
  for (int v = 0; v < I; ++v) {
    if (ans[v].size()) {
      nans++;
    }
  }
  printf("%d\\n", nans);
  for (int v = 0; v < I; ++v) {
    if (!ans[v].size()) continue;
    printf("%d\\n%d\\n", v, (int)ans[v].size());
    for (auto y : ans[v]) {
      printf("%s %d\\n", streetNames[y.first].c_str(), y.second);
    }
  }

  return 0;
}

"""

# %% [code]
import os

with open('program.cpp','w') as f:
    f.write(code)
    f.close()

!g++ program.cpp && ./a.out < ../input.txt > output.txt

# %% [code]
# cleanup
!rm -f a.out log