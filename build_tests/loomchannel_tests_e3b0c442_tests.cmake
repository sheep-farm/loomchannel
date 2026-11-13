add_test([=[BusTest.OpenAndCloseChannel]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OpenAndCloseChannel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OpenAndCloseChannel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:80]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.OpenDoesNotOverwriteExistingChannel]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OpenDoesNotOverwriteExistingChannel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OpenDoesNotOverwriteExistingChannel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:93]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.SubscribersReceivePublishedContent]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.SubscribersReceivePublishedContent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.SubscribersReceivePublishedContent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:104]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.BusSubscribeDoesNotCrash]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.BusSubscribeDoesNotCrash]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.BusSubscribeDoesNotCrash]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:128]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.OrphanSubscriberDoesNotCrash]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OrphanSubscriberDoesNotCrash]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OrphanSubscriberDoesNotCrash]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:146]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.UnsubscribeStopsDelivery]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.UnsubscribeStopsDelivery]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.UnsubscribeStopsDelivery]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:176]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.ConnectionRAIIUnsubscribesOnScopeExit]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:206]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.ConstraintFiltersMessages]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.ConstraintFiltersMessages]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.ConstraintFiltersMessages]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:224]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DefaultChannelTest.SubscriberReceivesStopOnClose]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=DefaultChannelTest.SubscriberReceivesStopOnClose]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DefaultChannelTest.SubscriberReceivesStopOnClose]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:248]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DefaultChannelTest.NewSubscriberGetsCurrentContent]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=DefaultChannelTest.NewSubscriberGetsCurrentContent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DefaultChannelTest.NewSubscriberGetsCurrentContent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:263]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderedChannelTest.MultipleThreadsPreserveAllMessages]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=OrderedChannelTest.MultipleThreadsPreserveAllMessages]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderedChannelTest.MultipleThreadsPreserveAllMessages]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:280]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(loomchannel_tests_TESTS [==[BusTest.OpenAndCloseChannel]==] [==[BusTest.OpenDoesNotOverwriteExistingChannel]==] [==[BusTest.SubscribersReceivePublishedContent]==] [==[BusTest.BusSubscribeDoesNotCrash]==] [==[BusTest.OrphanSubscriberDoesNotCrash]==] [==[BusTest.UnsubscribeStopsDelivery]==] [==[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]==] [==[BusTest.ConstraintFiltersMessages]==] [==[DefaultChannelTest.SubscriberReceivesStopOnClose]==] [==[DefaultChannelTest.NewSubscriberGetsCurrentContent]==] [==[OrderedChannelTest.MultipleThreadsPreserveAllMessages]==])
