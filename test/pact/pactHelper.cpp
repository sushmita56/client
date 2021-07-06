#include <iostream>
#include "consumer/consumer.h"
#include "consumer/pact_mock_server_ffi.h"

using namespace pact_consumer;
using namespace pact_consumer::matchers;

TEST(PactConsumerTest, GetJsonProjects) {
  auto provider = Pact("client", "oc-server");
  provider.pact_directory = "tests/pact";

  provider
    .given("i have a list of projects")
    .uponReceiving("a request for projects")
    .withRequest("GET", "/ocs/v2.php/cloud/users/Alice?format=json")
    .willRespondWith(200);

  auto result = provider.run_test([] (auto mock_server) {
    return ::testing::UnitTest::GetInstance()->current_test_suite()->Passed();
  });
  EXPECT_TRUE(result.is_ok()) << "Test failed";
}
