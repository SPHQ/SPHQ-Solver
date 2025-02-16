/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *|
 * Part of BlueTit Solver, licensed under Apache 2.0 with Commons Clause.
 * Commercial use, including SaaS, requires a separate license, see /LICENSE.md
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <chrono>
#include <thread>

#include "tit/core/cmd.hpp"
#include "tit/core/io.hpp"

namespace tit {
namespace {

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[[gnu::noinline]] void func_3() {
  eprintln("func_3");
  eprintln("Creating a joinable thread...");
  std::thread(
      [] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
}

[[gnu::noinline]] void func_2() {
  eprintln("func_2");
  func_3();
}

[[gnu::noinline]] void func_1() {
  eprintln("func_1");
  func_2();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

auto run_test(CmdArgs /*args*/) -> int {
  func_1();
  eprintln("This line should not be executed.");
  return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // namespace
} // namespace tit

TIT_IMPLEMENT_MAIN(run_test)
