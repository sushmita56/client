#include <iostream>
#include <gtest/gtest.h>
#include "include/consumer.h"

using namespace pact_consumer;

TEST(ClientSharing, Public) {
  auto provider = Pact("client", "oc-server");
  provider.pact_directory = "test/pact/provider";

  provider
    .given("i have a list of projects")
    .uponReceiving("a request for projects")
    .withRequest("GET", "/ocs/v2.php/cloud/users/admin?format=json")
    .willRespondWith(204);


  auto result = provider.run_test([] (auto mock_server) {
    return ::testing::UnitTest::GetInstance()->current_test_suite()->Passed();
  });
  EXPECT_TRUE(result.is_ok()) << "Test failed";
}

int main(int argc, char **argv) {
  std::cout<<"hello";
  ::testing::InitGoogleTest(&argc, argv);
  init();
  return RUN_ALL_TESTS();
}
