
//#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include <cmath>
#include <vector>
#include <algorithm>

using namespace emscripten;
using std::vector;
using std::make_pair;
using std::pair;
using std::max;
using std::string;

float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}
int int_sqrt(int x) {
  return sqrt(x);
}

int square(int x) {
  return x * x;
}

int allocate() {
  int* ptr = new int;
  return reinterpret_cast<int>(ptr);
}

void deallocate(int address) {
  int* ptr = reinterpret_cast<int*>(address);
  delete ptr;
}

void doStuffWithVector() {
  vector<int> foo;
  foo.push_back(4);
  foo.push_back(6);
  foo.push_back(2);
  foo.pop_back();
  foo.push_back(15);
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("lerp", &lerp);
    function("int_sqrt", &int_sqrt);
    function("square", &square);
    function("allocate", &allocate);
    function("deallocate", &deallocate);
    function("doStuffWithVector", &doStuffWithVector);
}

//
//extern "C" {
//
//int EMSCRIPTEN_KEEPALIVE int_sqrt(int x) {
//  return sqrt(x);
//}
//
//int EMSCRIPTEN_KEEPALIVE square(int x) {
//  return x * x;
//}
//
//int EMSCRIPTEN_KEEPALIVE identity(int x) {
//  return x;
//}
//
///*int allocate() {
//  int* ptr = new int;
//  return reinterpret_cast<int>(ptr);
//}
//
//void deallocate(int address) {
//  int* ptr = reinterpret_cast<int*>(address);
//  delete ptr;
//}
//
//void doStuffWithVector() {
//  vector<int> foo;
//  foo.push_back(4);
//  foo.push_back(6);
//  foo.push_back(2);
//  foo.pop_back();
//  foo.push_back(15);
//}*/
//
//int EMSCRIPTEN_KEEPALIVE meaning() {
//  return 42;
//}
//
//}


template<typename T>
class Grid {
public:
    Grid(int width, int height): width_(width), height_(height), data(width * height) {}
    void set(int row, int col, T val) {
        data[row * width_ + col] = val;
    }
    const T& get(int row, int col) const {
        return data[row * width_ + col];
    }

    int width_;
    int height_;
    vector<T> data;
};


using GridChar = Grid<char>;

static int idx(const GridChar& grid, int r1, int c1, int r2, int c2) {
    return c2 + grid.width_ * (r2 + grid.height_ * (c1 + grid.width_ * r1));
}

template<typename T>
class CircularQueue {
private:
    vector<T> data_;
    int head_;
    int tail_;
    int size_;

public:
    CircularQueue(int max_size) : data_(max_size), head_(0), tail_(0), size_(0) { }

    void push(T element) {
        data_[tail_++] = std::move(element);
        tail_ %= data_.size();
        size_++;
    }

    T pop() {
        const T result(std::move(data_[head_++]));
        head_ %= data_.size();
        size_--;
        return result;
    }

    bool empty() {
        return size_ == 0;
    }
};

class MyRobot {
public:
  MyRobot(int x, string y)
    : x(x)
    , y(y)
  {}

  void incrementX() {
    ++x;
  }

  int getX() const { return x; }
  void setX(int x_) { x = x_; }

  static string getStringFromInstance(const MyRobot& instance) {
    return instance.y;
  }

  int all_pairs_shortest_path(const GridChar& is_traversable) {
    const int rows = is_traversable.height_;
    const int cols = is_traversable.width_;

    vector<pair<int, int>> dirs = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
    vector<int> distances(rows*cols*rows*cols, -1);
    for (int start_row=0; start_row < rows; ++start_row) {
        for(int start_col=0; start_col < cols; ++start_col) {
            CircularQueue<pair<int, int>> queue(2 * (rows + cols + 5));
            queue.push(make_pair(start_row, start_col));
            distances[idx(is_traversable, start_row, start_col,start_row, start_col)] = 0;
            while(!queue.empty()) {
                const auto cur = queue.pop();
                const int cur_r = cur.first;
                const int cur_c = cur.second;
                const int next_dist = distances[idx(is_traversable, start_row, start_col,cur_r, cur_c)] + 1;
                for (int i=0; i < 4; ++i) {
                    const int next_r = cur_r + dirs[i].first;
                    const int next_c = cur_c + dirs[i].second;
                    if (next_r >= 0
                            && next_c >= 0
                            && next_r < rows
                            && next_c < cols
                            && is_traversable.get(next_r, next_c)) {
                        auto& ref = distances[idx(is_traversable, start_row, start_col,next_r, next_c)];
                        if ( ref == -1) {
                            ref = next_dist;
                            queue.push(make_pair(next_r, next_c));
                        }
                    }
                }
            }
        }
    }
    int longest_dist = 0;
    for (int start_row=0; start_row < rows; ++start_row) {
        for(int start_col=0; start_col < cols; ++start_col) {
            for (int r=0; r < rows; ++r) {
                for (int c=0; c < cols; ++c) {
                    longest_dist = max(longest_dist, distances[idx(is_traversable, start_row, start_col, r, c)]);
                }
            }
        }
    }
    return longest_dist;
  }

private:
  int x;
  string y;
};


// Binding code
EMSCRIPTEN_BINDINGS(my_robot_example) {
  class_<GridChar>("GridChar")
    .constructor<int, int>()
    .function("get", &GridChar::get)
    .function("set", &GridChar::set)
    ;
  class_<MyRobot>("MyRobot")
    .constructor<int, string>()
    .function("incrementX", &MyRobot::incrementX)
    .function("all_pairs_shortest_path", &MyRobot::all_pairs_shortest_path)
    .property("x", &MyRobot::getX, &MyRobot::setX)
    .class_function("getStringFromInstance", &MyRobot::getStringFromInstance)
    ;
}