#include <bits/stdc++.h>

using namespace std;

class Grid {
    public:

        using PlayerId = int;
        const static PlayerId NoPlayer = 0;

        struct Sides {
            enum Side {
                Top,
                Right,
                Bottom,
                Left
            };

            Side side;
            Sides(Side side) : side(side) {}
            Sides() : side(Top) {}
            operator Side() const { return side; }

            static Side opposite(Side side) {
                switch(side) {
                    case Top: return Bottom;
                    case Right: return Left;
                    case Bottom: return Top;
                    case Left: return Right;
                }
            }

        };

        

        struct Wall {
            PlayerId player;
            bool isVertical;
        };

        struct Vec2 {
            int x;
            int y;

            inline Vec2 operator+(const Vec2& other) const {
                return {x + other.x, y + other.y};
            }
        };
    
        struct Player {
            PlayerId id;
            Sides goal;
            Vec2 position;
        };
        

        class PathFinder {
            public:
                PathFinder(const Grid& grid) : grid(grid), visited(grid.width * grid.height, false) {}

                bool canReach(const Vec2& start, const Sides& side) const {
                    auto isGoal = [&](Sides::Side side) -> function<bool(const Vec2&)> {
                        switch(side) {
                            case Sides::Side::Top:      return [&](const Vec2& pos) { return pos.y == 0; };
                            case Sides::Side::Right:    return [&](const Vec2& pos) { return pos.x == grid.width - 1; };
                            case Sides::Side::Bottom:   return [&](const Vec2& pos) { return pos.y == grid.height - 1; };
                            case Sides::Side::Left:     return [&](const Vec2& pos) { return pos.x == 0; };
                        }
                    }(side);

                    fill(visited.begin(), visited.end(), false);
                    using Record = pair<int, Vec2>;
                    priority_queue<Record, vector<Record>, function<bool(Record, Record)>> heap([](const auto& a, const auto& b){return a.first > b.first;});
                    // add a compare function to a new  priority queue


                    // Greedy Best First Search
                    heap.push({0, start});
                    while(!heap.empty()) {
                        const auto[score, pos] = heap.top();   
                        heap.pop();

                        if(isGoal(pos)) { return true; }

                        if(isVisited(pos)) { continue; }
                        setVisited(pos);

                        for(const Vec2& next : getAvailblePositions(pos)) {
                            heap.push({next.x, next});
                        }

                    }

                    return false;
                
                }

                // Note that there is a state, where it retures a certain positions twice
                // This is because the path is not unique to the diagonal element if two paths are blocked next to each other
                // All algorithms should be designed to handle this
                vector<Vec2> getAvailblePositions(const Vec2& start) const {
                    vector<Vec2> positions;
                    for(const Vec2& dir : {Vec2{0, 1}, Vec2{0, -1}, Vec2{1, 0}, Vec2{-1, 0}}) {
                        const Vec2 pos = start + dir;
                        if(pos.x < 0 || pos.x >= grid.width || pos.y < 0 || pos.y >= grid.height) { continue; }
                        if(isWallBetween(start, pos)) { continue; }

                        if(grid.playersGrid[pos.y][pos.x] == NoPlayer) { positions.push_back(pos); continue; }

                        const Vec2 nextAfter = pos + dir;
                        const bool isNextOutOfBounds = nextAfter.x < 0 || nextAfter.x >= grid.width || nextAfter.y < 0 || nextAfter.y >= grid.height;
                        if(isNextOutOfBounds) { continue; }

                        const bool isNextWalled = isWallBetween(pos, nextAfter) || grid.playersGrid[nextAfter.y][nextAfter.x] != NoPlayer;
                        if(!isNextWalled) {
                            positions.push_back(nextAfter);
                            continue;

                        } 
                        
                        const Vec2 Perpendicular0 = {dir.y, dir.x};
                        const Vec2 Perpendicular1 = {-dir.y, -dir.x};

                        const Vec2 nextAfterPerp0 = pos + Perpendicular0;
                        const Vec2 nextAfterPerp1 = pos + Perpendicular1;

                        const bool isNextPerp0OutOfBounds = nextAfterPerp0.x < 0 || nextAfterPerp0.x >= grid.width || nextAfterPerp0.y < 0 || nextAfterPerp0.y >= grid.height;
                        const bool isNextPerp1OutOfBounds = nextAfterPerp1.x < 0 || nextAfterPerp1.x >= grid.width || nextAfterPerp1.y < 0 || nextAfterPerp1.y >= grid.height;

                        const bool isNextPerp0Walled = isNextPerp0OutOfBounds || isWallBetween(pos, nextAfterPerp0);
                        const bool isNextPerp1Walled = isNextPerp1OutOfBounds || isWallBetween(pos, nextAfterPerp1);

                        if(!isNextPerp0Walled && grid.playersGrid[nextAfterPerp0.y][nextAfterPerp0.x] == NoPlayer) {
                            positions.push_back(nextAfterPerp0);
                        }

                        if(!isNextPerp1Walled && grid.playersGrid[nextAfterPerp1.y][nextAfterPerp1.x] == NoPlayer) {
                            positions.push_back(nextAfterPerp1);
                        }
                    }

                    return positions;
                }

                bool isWallBetween(const Vec2& a, const Vec2& b) const {
                    auto [x, y] = a;
                    auto [x2, y2] = b;

                    if(x > x2) { swap(x, x2); }
                    if(y > y2) { swap(y, y2); }

                    const bool isVertical = y2 - y == 1 && x == x2;
                    const bool isHorizontal = x2 - x == 1 && y == y2;

                    assert(isVertical ^ isHorizontal);

                    if(isVertical) {
                        const bool isHorizontalWall0 = grid.wallsGrid[y2][x-0].isVertical && grid.wallsGrid[y2][x-0].player != 0;
                        const bool isHorizontalWall1 = x > 1 && grid.wallsGrid[y2][x-1].isVertical && grid.wallsGrid[y2][x-1].player != 0;
                        const bool isHorizontalWall = isHorizontalWall0 || isHorizontalWall1;

                        return isHorizontalWall;

                    } else {
                        const bool isVerticalWall0 = grid.wallsGrid[y-0][x2].isVertical && grid.wallsGrid[y-0][x2].player != 0;
                        const bool isVerticalWall1 = y > 1 && grid.wallsGrid[y-1][x2].isVertical && grid.wallsGrid[y-1][x2].player != 0;
                        const bool isVerticalWall = isVerticalWall0 || isVerticalWall1;

                        return isVerticalWall;
                    }

                }

            private:
                const Grid& grid;
                mutable vector<bool> visited;


                inline bool isVisited(const Vec2& pos) const {
                    return visited[pos.y * grid.width + pos.x];
                }

                inline void setVisited(const Vec2& pos) const {
                    visited[pos.y * grid.width + pos.x] = true;
                }
        };

    public:
        Grid(int width, int height)
            : width(width), height(height), wallsGrid(height, vector<Wall>(width, {0, false})), playersGrid(height, vector<PlayerId>(width, NoPlayer)), pathFinder(*this) {


        }

        void setWall(int x, int y) {
            wallsGrid[x][y].isVertical = true;
        }

        void removePlayers(){
            for(auto& p : players) {
                playersGrid[p.position.y][p.position.x] = NoPlayer;
            }

            playersGrid.clear();
        }

        void addPlayer(Player p) {
            playersGrid[p.position.y][p.position.x] = p.id;
            players.push_back(p);
        }

        bool canMove(Player player, Vec2 pos) {
            const auto& availablePositions = pathFinder.getAvailblePositions(player.position);
            return find(availablePositions.begin(), availablePositions.end(), pos) != availablePositions.end();
        }

        bool isValidWall(int x, int y, Wall wall) {
            if(wallsGrid[y][x].player != NoPlayer) { return false; }
            wallsGrid[y][x] = wall;

            const bool isValid = [&]() {
                for(const Player& player : players) {
                    if(!pathFinder.canReach(player.position, player.goal)) {
                        return false;
                    }
                }

                return true;
            }();

            wallsGrid[y][x] = {NoPlayer, false};
            return isValid;
        }

    private:
        int width;
        int height;
        vector<vector<Wall>> wallsGrid;
        vector<vector<PlayerId>> playersGrid;
        vector<Player> players;
        PathFinder pathFinder;

};


int main(){


    return 0;
}