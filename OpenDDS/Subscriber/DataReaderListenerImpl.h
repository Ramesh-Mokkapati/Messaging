// DataReaderListenerImpl.h
#ifndef DATAREADERLISTENERIMPL_H
#define DATAREADERLISTENERIMPL_H

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/LocalObject.h>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

class DataReaderListenerImpl
    : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
{
public:
    DataReaderListenerImpl();
    ~DataReaderListenerImpl();

    virtual void on_data_available(DDS::DataReader_ptr reader);

    virtual void on_requested_deadline_missed(
        DDS::DataReader_ptr, const DDS::RequestedDeadlineMissedStatus&)
    {
    }
    virtual void on_requested_incompatible_qos(
        DDS::DataReader_ptr, const DDS::RequestedIncompatibleQosStatus&)
    {
    }
    virtual void on_liveliness_changed(
        DDS::DataReader_ptr, const DDS::LivelinessChangedStatus&)
    {
    }
    virtual void on_subscription_matched(
        DDS::DataReader_ptr reader, const DDS::SubscriptionMatchedStatus& status);
    virtual void on_sample_rejected(
        DDS::DataReader_ptr, const DDS::SampleRejectedStatus&)
    {
    }
    virtual void on_sample_lost(
        DDS::DataReader_ptr, const DDS::SampleLostStatus&) {
    }

    // Blocks until a subscriber has matched and then unmatched (i.e.
    // the publisher finished and went away), so main() knows when to
    // stop waiting and shut down cleanly.
    void wait_for_completion();

private:
    ACE_Thread_Mutex mutex_;
    ACE_Condition_Thread_Mutex condition_;
    bool matched_;
    bool done_;
};

#endif // DATAREADERLISTENERIMPL_H
