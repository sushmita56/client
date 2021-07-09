#include <iostream>
#include <gtest/gtest.h>
#include "include/consumer.h"
#include "../../src/gui/owncloudgui.h"

using namespace pact_consumer;
ownCloudGui *Application::gui() const
{
    return _gui;
}

TEST(ClientSharing, Public) {
  auto provider = Pact("client", "oc-server");
  provider.pact_directory = "test/pact/provider";

  provider
    .given("i have a list of projects")
    .uponReceiving("a request for projects")
    .withRequest("GET", "/ocs/v2.php/cloud/users/admin?format=json")
    .willRespondWith(404);


  auto result = provider.run_test([] (auto mock_server) {
      gui = new ownCloudGui(this);
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
