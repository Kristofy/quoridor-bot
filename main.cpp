#include <bits/stdc++.h>

using namespace std;

struct Vec2 {
  int x;
  int y;

  inline auto operator+(const Vec2 &other) const -> Vec2 { return {x + other.x, y + other.y}; }

  inline auto operator==(const Vec2 &other) const -> bool { return x == other.x && y == other.y; }

  inline auto operator!=(const Vec2 &other) const -> bool { return !(*this == other); }
};

using PlayerId = int;
const PlayerId NoPlayer = 0;

class Board {
public:
  struct Sides {
    enum Side { Top, Right, Bottom, Left };

    Sides(Side side) : side(side) {}

    Sides() : side(Top) {}
    explicit operator Side() const { return side; }

    static auto opposite(Side side) -> Side {
      switch (side) {
      case Top:
        return Bottom;
      case Right:
        return Left;
      case Bottom:
        return Top;
      case Left:
        return Right;
      }
    }

  private:
    Side side;
  };

  struct Wall {
    bool isVertical;
    PlayerId player;
  };

  struct Player {
    PlayerId id = NoPlayer;
    Sides goal = Sides(Sides::Side::Top);
    Vec2 position = {0,0};
  };

  class PathFinder {
  public:
    explicit PathFinder(const Board &board) : board(board), visited(board.width * board.height, false) {}

    auto canReach(const Vec2 &start, const Sides &side) const -> bool {
      auto isGoal = isGoalFactory(side);
      auto getScore = getScoreFactory(side);

      fill(visited.begin(), visited.end(), false);
      priority_queue<Vec2, vector<Vec2>, function<bool(Vec2, Vec2)>> heap(
          [&getScore](const auto &a, const auto &b) { return getScore(a) < getScore(b); });

      // Greedy Best First Search
      heap.push(start);
      while (!heap.empty()) {
        const Vec2 pos = heap.top();
        heap.pop();

        if (isVisited(pos)) { continue; }
        setVisited(pos);

        if (isGoal(pos)) { return true; }
        
        for(const Vec2 next : {pos + Vec2{0, 1}, pos + Vec2{0, -1}, pos + Vec2{-1, 0}, pos + Vec2{1, 0}}){
            if (isInBounds(next) && !isWallBetween(pos, next) && !isVisited(next)){
                heap.push(next);
            }
        }
      }

      return false;
    }

    auto getPath(const Vec2 &start, const Sides &side) const -> pair<bool, vector<Vec2>> {
      pair<bool, vector<Vec2>> res = {false, {}};
      auto &[canReach, path] = res;

      auto isGoal = isGoalFactory(side);
      auto getScore = getScoreFactory(side);

      fill(visited.begin(), visited.end(), false);
      vector<vector<Vec2>> p(board.height, vector<Vec2>(board.width));

      using Record = pair<Vec2, Vec2>;
      priority_queue<Record, vector<Record>, function<bool(Record, Record)>> heap(
          [&getScore](const auto &a, const auto &b) { return getScore(a.first) < getScore(b.first); });

      // Greedy Best First Search
      heap.push({start, {-1, -1}});
      Vec2 finish;

      while (!heap.empty()) {
        const auto [pos, parent] = heap.top();
        heap.pop();
        
        if (isVisited(pos)) { continue; }
        setVisited(pos);
        p[pos.y][pos.x] = parent;

        if (isGoal(pos)) {
          canReach = true;
          finish = pos;
          break;
        }



        for (const Vec2 &next : getAvailblePositions(pos)) {
          if (!isVisited(next)) {
            heap.push({next, pos});
          }
        }
      }

      if (canReach) {
        while (finish != start) {
          path.push_back(finish);
          finish = p[finish.y][finish.x];
        }
        reverse(path.begin(), path.end());
      }

      return res;
    }

    // Note that there is a state, where it retures a certain positions twice
    // This is because the path is not unique to the diagonal element if two
    // paths are blocked next to each other All algorithms should be designed to
    // handle this
    // TODO(kristofy): Think through again
    auto getAvailblePositions(const Vec2 &start) const -> vector<Vec2> {
      vector<Vec2> positions;
      auto isInBounds = [&](const Vec2 &pos) {
        return pos.x >= 0 && pos.x < board.width && pos.y >= 0 && pos.y < board.height;
      };

      auto isEmptyTile = [&](const Vec2 &pos) { return board.playersGrid[pos.y][pos.x] == NoPlayer; };

      for (const Vec2 &dir : {Vec2{0, 1}, Vec2{0, -1}, Vec2{1, 0}, Vec2{-1, 0}}) {
        const Vec2 pos = start + dir;
        if (!isInBounds(pos) || isWallBetween(start, pos)) {
          continue;
        }
        if (isEmptyTile(pos)) {
          positions.push_back(pos);
          continue;
        }
        const Vec2 nextAfter = pos + dir;
        if (isInBounds(nextAfter) && !isWallBetween(pos, nextAfter) && isEmptyTile(nextAfter)) {
          positions.push_back(nextAfter);
          continue;
        }
        const Vec2 nextAfterPerp0 = pos + Vec2{dir.y, dir.x};
        if (isInBounds(nextAfterPerp0) && !isWallBetween(pos, nextAfterPerp0) && isEmptyTile(nextAfterPerp0)) {
          positions.push_back(nextAfterPerp0);
        }
        const Vec2 nextAfterPerp1 = pos + Vec2{-dir.y, -dir.x};
        if (isInBounds(nextAfterPerp1) && !isWallBetween(pos, nextAfterPerp1) && isEmptyTile(nextAfterPerp1)) {
          positions.push_back(nextAfterPerp1);
        }
      }
      return positions;
    }

    // TODO(kristofy) test wall
    auto isWallBetween(const Vec2 &a, const Vec2 &b) const -> bool {
      auto [x, y] = a;
      auto [x2, y2] = b;

      if (x > x2) {
        swap(x, x2);
      }
      if (y > y2) {
        swap(y, y2);
      }

      const bool isVertical = y2 - y == 1 && x == x2;
      const bool isHorizontal = x2 - x == 1 && y == y2;

      assert(isVertical ^ isHorizontal);

      int const U = (isVertical ? x : x2);
      int const V = (isVertical ? y2 : y);

      const bool isWall0 = board.wallsGrid[V][U - 0].isVertical && board.wallsGrid[V][U - 0].player != 0;
      const bool isWall1 = (isVertical ? x > 1 : y > 1) &&
                           board.wallsGrid[V - isHorizontal][U - isVertical].isVertical &&
                           board.wallsGrid[V - isHorizontal][U - isVertical].player != 0;

      return isWall0 || isWall1;
    }

  private:
    const Board &board;
    mutable vector<bool> visited;

    inline auto isVisited(const Vec2 &pos) const -> bool { return visited[pos.y * board.width + pos.x]; }

    inline void setVisited(const Vec2 &pos) const { visited[pos.y * board.width + pos.x] = true; }

    public: inline auto isGoalFactory(const Sides& side) const -> function<bool(const Vec2 &)> {
        switch ((Sides::Side)side) {
            case Sides::Side::Top:
                return [&](const Vec2 &pos) { return pos.y == board.height - 1; };
            case Sides::Side::Right:
                return [&](const Vec2 &pos) { return pos.x == board.width - 1; };
            case Sides::Side::Bottom:
                return [&](const Vec2 &pos) { return pos.y == 0; };
            case Sides::Side::Left:
                return [&](const Vec2 &pos) { return pos.x == 0; };
        }
        return [&](const Vec2& pos __attribute__((unused))) { cerr<<"Error occured" << endl; return false; };   
    }

    public: inline auto getScoreFactory(const Sides& side) const -> function<int(const Vec2 &)> {
        switch ((Sides::Side)side) {
            case Sides::Side::Top:
              return [&](const Vec2 &pos) { return pos.y; };
            case Sides::Side::Right:
                return [&](const Vec2 &pos) { return pos.x; };
            case Sides::Side::Bottom:
                return [&](const Vec2 &pos) { return board.height - 1 - pos.y; };
            case Sides::Side::Left:
                return [&](const Vec2 &pos) { return board.width - 1 - pos.x; };
        }
        return [&](const Vec2 &pos __attribute__((unused))) { return false; };
    }

    inline auto isInBounds (const Vec2 &pos) const -> bool {
        return pos.x >= 0 && pos.x < board.width && pos.y >= 0 && pos.y < board.height;
    };

  };

  Board(int width, int height)
      : width(width), height(height), wallsGrid(height, vector<Wall>(width, {0, false})),
        playersGrid(height, vector<PlayerId>(width, NoPlayer)), pathFinder(*this) {}

  void setWall(Vec2 pos, bool isVertical, PlayerId player) { wallsGrid[pos.y][pos.x].isVertical = isVertical; }

  void removePlayers() {
    for (auto &p : players) {
      playersGrid[p.position.y][p.position.x] = NoPlayer;
    }

    players.clear();
  }

  void addPlayer(Player p) {
    assert(p.id != NoPlayer);
    playersGrid[p.position.y][p.position.x] = p.id;
    players.push_back(p);
  }

  auto canMove(Player player, Vec2 pos) const -> bool {
    const auto &availablePositions = pathFinder.getAvailblePositions(player.position);
    return find(availablePositions.begin(), availablePositions.end(), pos) != availablePositions.end();
  }

  auto isValidWall(int x, int y, Wall wall) -> bool {
    if (wallsGrid[y][x].player != NoPlayer) {
      return false;
    }
    wallsGrid[y][x] = wall;

    const bool isValid = all_of(players.begin(), players.end(), [&](const Player &player) {
      return pathFinder.canReach(player.position, player.goal);
    });

    wallsGrid[y][x] = {false, NoPlayer};
    return isValid;
  }

private:
  vector<vector<Wall>> wallsGrid;
  vector<vector<PlayerId>> playersGrid;
  vector<Player> players;

public:
  int width;
  int height;
  PathFinder pathFinder;
};

string toString(Board::Sides sides) {
  string s;
  Board::Sides::Side side = (Board::Sides::Side)sides;
  switch (side) {
  case Board::Sides::Side::Top:
    s = "Top";
    break;
  case Board::Sides::Side::Bottom:
    s = "Bottom";
    break;
  case Board::Sides::Side::Left:
    s = "Left";
    break;
  case Board::Sides::Side::Right:
    s = "Right";
    break;
  }
  return s;
}

struct GameApi {
  GameApi() {
    string s;
    cin >> s;
    cout << "OK" << endl;
  }

  Board init() {
    int width = 9;
    int height = 9;
    Board board(width, height);
    cin >> number_of_players >> myID;
    players.resize(number_of_players);
    number_of_walls = 20 / number_of_players;
    return board;
  }

  void readTick(Board &board) {
    board.removePlayers();
    cin >> current_tick;
    cerr << "Tick: " << current_tick << endl;

    for (int i = 0; i < number_of_players; i++) {
      int id = players[i].id;
      cin >> players[i].position.x >> players[i].position.y >> players[i].id;
      players[i].id++;
      board.addPlayer(players[i]);
    }

    cerr << "Read players" << endl;


    if (current_tick <= number_of_players) {
        players[1].goal = Board::Sides::Side::Bottom;
        players[0].goal = Board::Sides::Side::Top;
        cerr << "Set goals" << endl;
        auto d = board.pathFinder.getScoreFactory(players[myID].goal);
        cerr << "dist : " << 9 - d(players[myID].position) << endl;

    }


    int walls;
    cin >> walls;

    Board::Wall wall;
    for (int i = 0; i < walls; i++) {
      Vec2 pos;
      cin >> pos.x >> pos.y >> wall.player;
      board.setWall(pos, wall.isVertical, wall.player + 1);
    }
    cerr << "Read walls" << endl;

  }

  void step(Board &board, Vec2 pos) {
    cerr << "[" << myID << "] "
         << "move: " << pos.x << " " << pos.y << endl;
    cout << pos.x << " " << pos.y << endl;
  }

  void placeWall(Board &board, Vec2 pos, bool isVertical) {
    cerr << "[" << myID << "] "
         << "wall: " << pos.x << " " << pos.y << " " << isVertical << endl;
    cout << pos.x << " " << pos.y << " " << isVertical << endl;
    number_of_walls--;
  }

  PlayerId myID;
  int number_of_players;
  int current_tick;
  int number_of_walls;
  vector<Board::Player> players;
};

int main() {
  GameApi api;
  Board board = api.init();

  while (true) {
    api.readTick(board);

    vector<pair<bool, vector<Vec2>>> paths(api.number_of_players);
    cerr << "in tick: " << api.current_tick << " the paths lenght are: ";
    for (int i = 0; i < api.number_of_players; i++) {
      paths[i] = board.pathFinder.getPath(api.players[i].position, api.players[i].goal);
      cerr << paths[i].second.size() << " ";
    }
    cerr << endl;

    cerr << "get paths" << endl;
    bool canIReach = paths[api.myID].first;
    int myPath = paths[api.myID].second.size();

    int mini = 0;
    for (int i = 0; i < api.number_of_players; i++) {
      if (paths[i].first && paths[i].second.size() < paths[mini].second.size() && i != api.myID) {
        mini = i;
      }
    }

    cerr << "Found  mins, " << paths[mini].second.size() << endl;

    if(canIReach) {
        cerr << "I can reach the goal in " << myPath << endl;
    }else{
        cerr << "I can't reach to goal" << endl;
    }

    if (paths[mini].second.size() > myPath) {
        api.step(board, paths[api.myID].second.front());
    } else {
        Vec2 a = paths[mini].second.front();
        if(board.isValidWall(a.x, a.y, Board::Wall{false, api.myID})) {
            api.placeWall(board, a, false);
        } else if(board.isValidWall(a.x, a.y, Board::Wall{true, api.myID})) {
            api.placeWall(board, a, true);
        } else {
            api.step(board, paths[api.myID].second.front());
        }
    }
    cerr << "Answered  mins" << endl;

  }

  return 0;
}

// Az első üzenetnek tartalmazhatná a kedzdő poziciót
// result += `${state.tick.pawnPos[i].x} ${state.tick.pawnPos[i].y} ${state.tick.ownedWalls[i]}\n`;
// to
// result += `${state.tick.pawnPos[i].x} ${state.tick.pawnPos[i].y} ${i}\n`;
// a sorrend nem az ami a dokumentációban van
// illegális lépéseket is meglép