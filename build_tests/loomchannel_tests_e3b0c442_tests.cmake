add_test([=[BusTest.OpenAndCloseChannel]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OpenAndCloseChannel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OpenAndCloseChannel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:127]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.OpenDoesNotOverwriteExistingChannel]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OpenDoesNotOverwriteExistingChannel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OpenDoesNotOverwriteExistingChannel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:140]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.SubscribersReceivePublishedContent]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.SubscribersReceivePublishedContent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.SubscribersReceivePublishedContent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:151]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.BusSubscribeDoesNotCrash]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.BusSubscribeDoesNotCrash]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.BusSubscribeDoesNotCrash]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:175]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.OrphanSubscriberReceivesAfterChannelOpens]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.OrphanSubscriberReceivesAfterChannelOpens]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.OrphanSubscriberReceivesAfterChannelOpens]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:193]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.UnsubscribeStopsDelivery]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.UnsubscribeStopsDelivery]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.UnsubscribeStopsDelivery]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:225]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.ConnectionRAIIUnsubscribesOnScopeExit]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:255]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.ConstraintFiltersMessages]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.ConstraintFiltersMessages]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.ConstraintFiltersMessages]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:273]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.SubscriberCallbackCanReenterBus]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.SubscriberCallbackCanReenterBus]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.SubscriberCallbackCanReenterBus]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:294]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BusTest.SubscriberCallbackCanLookupChannelDuringBroadcast]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=BusTest.SubscriberCallbackCanLookupChannelDuringBroadcast]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BusTest.SubscriberCallbackCanLookupChannelDuringBroadcast]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:319]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DefaultChannelTest.SubscriberReceivesStopOnClose]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=DefaultChannelTest.SubscriberReceivesStopOnClose]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DefaultChannelTest.SubscriberReceivesStopOnClose]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:340]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DefaultChannelTest.NewSubscriberGetsCurrentContent]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=DefaultChannelTest.NewSubscriberGetsCurrentContent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DefaultChannelTest.NewSubscriberGetsCurrentContent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:355]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderedChannelTest.MultipleThreadsPreserveAllMessages]=]  /home/flavio/Work/loomchannel/build_tests/loomchannel_tests [==[--gtest_filter=OrderedChannelTest.MultipleThreadsPreserveAllMessages]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderedChannelTest.MultipleThreadsPreserveAllMessages]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/home/flavio/Work/loomchannel/tests/test_bus.cpp:372]==]
    WORKING_DIRECTORY [==[/home/flavio/Work/loomchannel/build_tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(loomchannel_tests_TESTS [==[BusTest.OpenAndCloseChannel]==] [==[BusTest.OpenDoesNotOverwriteExistingChannel]==] [==[BusTest.SubscribersReceivePublishedContent]==] [==[BusTest.BusSubscribeDoesNotCrash]==] [==[BusTest.OrphanSubscriberReceivesAfterChannelOpens]==] [==[BusTest.UnsubscribeStopsDelivery]==] [==[BusTest.ConnectionRAIIUnsubscribesOnScopeExit]==] [==[BusTest.ConstraintFiltersMessages]==] [==[BusTest.SubscriberCallbackCanReenterBus]==] [==[BusTest.SubscriberCallbackCanLookupChannelDuringBroadcast]==] [==[DefaultChannelTest.SubscriberReceivesStopOnClose]==] [==[DefaultChannelTest.NewSubscriberGetsCurrentContent]==] [==[OrderedChannelTest.MultipleThreadsPreserveAllMessages]==])
