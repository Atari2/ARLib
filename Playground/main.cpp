#include "../CharConv.h"
#include "../Printer.h"
#include "../PriorityQueue.h"
using namespace ARLib;

int main() {
    auto queue_priority = [](const auto& left, const auto& right) {
        if (left.size() > right.size())
            return greater;
        else if (left.size() < right.size())
            return less;
        return equal;
    };
    PriorityQueue<String, String> queue{queue_priority};
    PriorityQueue<String> queue2{};
    queue.push("123"_s);
    queue.push("1234"_s);
    queue2.push("hello"_s, 10);
    queue2.push("world"_s, 20);
    Printer::print("{} {}", queue2, queue);
    Printer::print("{} {}", queue.pop(), queue2.pop());
    Printer::print("{} {}", queue.size(), queue2.size());
    return 0;
}