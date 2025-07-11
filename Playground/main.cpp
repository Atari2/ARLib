#include "Vector.hpp"
#include "GenericView.hpp"
#include "String.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "JSONParser.hpp"
#include "Tuple.hpp"
#include "ArgParser.hpp"
#include "EventLoop.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    EventLoop loop{};
    loop.start();

    for (size_t i = 0; i < 100; ++i) {
        loop.subscribe_callback([](int id) {
            Printer::print("Hello from callback with id: {}\n", id); }, static_cast<int>(i)
        );
    }
    loop.join(EventLoop::JoinType::WaitUntilFinished);

    return 0;
}
