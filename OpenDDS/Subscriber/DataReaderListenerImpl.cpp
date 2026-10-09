// DataReaderListenerImpl.cpp
#include "DataReaderListenerImpl.h"
#include "MessengerTypeSupportImpl.h"

#include <ace/Guard_T.h>

#include <iostream>

DataReaderListenerImpl::DataReaderListenerImpl()
    : condition_(mutex_)
    , matched_(false)
    , done_(false)
{
}

DataReaderListenerImpl::~DataReaderListenerImpl()
{
}

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
    Messenger::MessageDataReader_var reader_i =
        Messenger::MessageDataReader::_narrow(reader);
    if (!reader_i)
    {
        std::cerr << "on_data_available: MessageDataReader::_narrow() failed" << std::endl;
        return;
    }

    Messenger::Message message;
    DDS::SampleInfo info;

    const DDS::ReturnCode_t error = reader_i->take_next_sample(message, info);
    if (error == DDS::RETCODE_OK)
    {
        if (info.valid_data)
        {
            // C++11 IDL mapping: fields are read via accessor methods.
            std::cout << "Message received"
                << "\n  from:       " << message.from()
                << "\n  subject:    " << message.subject()
                << "\n  subject_id: " << message.subject_id()
                << "\n  text:       " << message.text()
                << "\n  count:      " << message.count()
                << std::endl;
        }
        else
        {
            std::cout << "Received a metadata-only sample (e.g. dispose/unregister)." << std::endl;
        }
    }
    else
    {
        std::cerr << "take_next_sample() returned error " << error << std::endl;
    }
}

void DataReaderListenerImpl::on_subscription_matched(
    DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus& status)
{
    ACE_Guard<ACE_Thread_Mutex> guard(mutex_);

    if (status.current_count >= 1)
    {
        matched_ = true;
        std::cout << "Subscriber: matched with a publisher." << std::endl;
    }
    else if (status.current_count == 0 && matched_)
    {
        // We were matched before and now we're not: the publisher is done.
        done_ = true;
        std::cout << "Subscriber: publisher went away, shutting down." << std::endl;
        condition_.signal();
    }
}

void DataReaderListenerImpl::wait_for_completion()
{
    ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
    while (!done_)
    {
        condition_.wait();
    }
}
