// Subscriber.cpp
// Creates a DomainParticipant, Topic, Subscriber and DataReader, and
// prints every Messenger::Message sample it receives.

#include "MessengerTypeSupportImpl.h"
#include "DataReaderListenerImpl.h"

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>

#include <iostream>
#include <stdexcept>

int main(int argc, ACE_TCHAR* argv[])
{
    try
    {
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

        DDS::DomainParticipant_var participant = dpf->create_participant(
            42,
            PARTICIPANT_QOS_DEFAULT,
            0,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!participant)
        {
            throw std::runtime_error("create_participant() failed");
        }

        Messenger::MessageTypeSupport_var ts = new Messenger::MessageTypeSupportImpl;
        if (ts->register_type(participant, "") != DDS::RETCODE_OK)
        {
            throw std::runtime_error("register_type() failed");
        }

        CORBA::String_var type_name = ts->get_type_name();
        DDS::Topic_var topic = participant->create_topic(
            "Movie Discussion List",
            type_name,
            TOPIC_QOS_DEFAULT,
            0,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!topic)
        {
            throw std::runtime_error("create_topic() failed");
        }

        DDS::Subscriber_var subscriber = participant->create_subscriber(
            SUBSCRIBER_QOS_DEFAULT,
            0,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!subscriber)
        {
            throw std::runtime_error("create_subscriber() failed");
        }

        DDS::DataReaderListener_var listener(new DataReaderListenerImpl);

        DDS::DataReaderQos dr_qos;
        subscriber->get_default_datareader_qos(dr_qos);
        dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

        DDS::DataReader_var reader = subscriber->create_datareader(
            topic,
            dr_qos,
            listener,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!reader)
        {
            throw std::runtime_error("create_datareader() failed");
        }

        std::cout << "Subscriber: waiting for messages..." << std::endl;

        DataReaderListenerImpl* listener_servant =
            dynamic_cast<DataReaderListenerImpl*>(listener.in());
        if (!listener_servant)
        {
            throw std::runtime_error("Failed to get listener servant");
        }

        // Blocks until the publisher has connected and then disconnected.
        listener_servant->wait_for_completion();

        participant->delete_contained_entities();
        dpf->delete_participant(participant);
        TheServiceParticipant->shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught in Subscriber: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
