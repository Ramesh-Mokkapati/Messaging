// Publisher.cpp
// Creates a DomainParticipant, Topic, Publisher and DataWriter, then
// writes 10 samples of Messenger::Message using the C++11 IDL mapping.

#include "MessengerTypeSupportImpl.h"

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include <ace/OS_NS_unistd.h>

#include <iostream>
#include <stdexcept>

int main(int argc, ACE_TCHAR* argv[])
{
    try
    {
        // Initialize DDS, consuming any OpenDDS-specific command line
        // options (e.g. -DCPSConfigFile rtps.ini).
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

        DDS::Publisher_var publisher = participant->create_publisher(
            PUBLISHER_QOS_DEFAULT,
            0,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!publisher)
        {
            throw std::runtime_error("create_publisher() failed");
        }

        DDS::DataWriterQos dw_qos;
        publisher->get_default_datawriter_qos(dw_qos);
        dw_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

        DDS::DataWriter_var writer = publisher->create_datawriter(
            topic,
            dw_qos,
            0,
            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!writer)
        {
            throw std::runtime_error("create_datawriter() failed");
        }

        Messenger::MessageDataWriter_var message_writer =
            Messenger::MessageDataWriter::_narrow(writer);
        if (!message_writer)
        {
            throw std::runtime_error("MessageDataWriter::_narrow() failed");
        }

        // Wait for at least one matching subscriber so the first samples
        // aren't sent into the void.
        DDS::StatusCondition_var condition = writer->get_statuscondition();
        condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

        DDS::WaitSet_var ws = new DDS::WaitSet;
        ws->attach_condition(condition);

        std::cout << "Publisher: waiting for a subscriber to match..." << std::endl;

        while (true)
        {
            DDS::PublicationMatchedStatus matches;
            if (writer->get_publication_matched_status(matches) != DDS::RETCODE_OK)
            {
                throw std::runtime_error("get_publication_matched_status() failed");
            }

            if (matches.current_count >= 1)
            {
                break;
            }

            DDS::ConditionSeq conditions;
            const DDS::Duration_t timeout = { 30, 0 };
            if (ws->wait(conditions, timeout) != DDS::RETCODE_OK)
            {
                throw std::runtime_error("Timed out waiting for a subscriber");
            }
        }

        ws->detach_condition(condition);
        std::cout << "Publisher: matched, sending samples." << std::endl;

        // --- C++11 IDL mapping: struct fields are accessed via
        //     get/set-style member functions, e.g. message.from("x")
        //     to set, message.from() to get. ---
        Messenger::Message message;
        message.subject_id(99);
        message.from("OpenDDS Publisher");
        message.subject("Hello, World!");
        message.text("This message uses the C++11 IDL mapping.");

        for (int i = 0; i < 10; ++i)
        {
            message.count(i);
            const DDS::ReturnCode_t error = message_writer->write(message, DDS::HANDLE_NIL);
            if (error != DDS::RETCODE_OK)
            {
                std::cerr << "write() returned error " << error << std::endl;
            }
            else
            {
                std::cout << "Sent sample " << i << std::endl;
            }

            ACE_OS::sleep(1);
        }

        // Give the sample(s) a chance to be delivered/acknowledged before
        // tearing everything down.
        DDS::Duration_t drain_timeout = { 10, 0 };
        writer->wait_for_acknowledgments(drain_timeout);

        participant->delete_contained_entities();
        dpf->delete_participant(participant);
        TheServiceParticipant->shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught in Publisher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
