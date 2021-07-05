#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <consumer.h>
#include "todo.h"

using namespace testing;
using namespace pact_consumer;
using namespace pact_consumer::matchers;

TEST(PactConsumerTest, GetJsonProjects) {
  auto provider = Pact("client", "oc-server");
  provider.pact_directory = "tests/pacts";

  provider
    .given("i have a list of projects")
    .uponReceiving("a request for projects")
    .withRequest("GET", "/projects")
    .withQuery(query)
    .withHeaders(headers)
    .willRespondWith(200)
    .withResponseHeaders(res_headers)
    .withResponseJsonBody(Object({
      { "projects", EachLike(Object({
        { "id", Integer(3) },
        { "name", Like("Project 1") },
        { "due", DateTime("yyyy-MM-dd'T'HH:mm:ss.SSSX") },
        { "tasks", AtLeastOneLike(4, Object({
          { "id", Integer() },
          { "name", Matching("Task \\d+", "Task 1") },
          { "done", Like(true) }
        }))}
      }))}
    }));

      auto result = provider.run_test([] ('http://localhost/core') {
        TodoClient todo;
        todo.serverUrl = mock_server->get_url();
        std::cout << "URL: " << todo.serverUrl << "\n";

        std::vector<Project> projects = todo.getProjects();

        EXPECT_THAT(todo.getProjects(), SizeIs(1));
        Project p = projects[0];
        EXPECT_EQ(p.name, "Project 1");
        EXPECT_GE(p.id, 0);
        EXPECT_THAT(p.tasks, SizeIs(4));
        EXPECT_GE(p.tasks[0].id, 0);
        EXPECT_EQ(p.tasks[0].name, "Task 1");

        return ::testing::UnitTest::GetInstance()->current_test_suite()->Passed();
      });
      EXPECT_TRUE(result.is_ok()) << "Test failed";
}
