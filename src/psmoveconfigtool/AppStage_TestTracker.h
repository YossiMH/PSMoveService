#ifndef APP_STAGE_TEST_TRACKER_H
#define APP_STAGE_TEST_TRACKER_H

//-- includes -----
#include "AppStage.h"
#include "ClientPSMoveAPI.h"

#include <vector>

//-- definitions -----
class AppStage_TestTracker : public AppStage
{
public:
    AppStage_TestTracker(class App *app);

    virtual void enter() override;
    virtual void exit() override;
    virtual void update() override;
    virtual void render() override;

    virtual void renderUI() override;

    static const char *APP_STAGE_NAME;

    void request_tracker_start_stream(int trackerID);
    void request_tracker_stop_stream(int trackerID);

protected:
    static void handle_tracker_start_stream_response(
        ClientPSMoveAPI::eClientPSMoveResultCode ResultCode,
        const ClientPSMoveAPI::t_request_id request_id,
        ClientPSMoveAPI::t_response_handle response_handle,
        void *userdata);
    void open_shared_memory_stream();

    static void handle_tracker_stop_stream_response(
        ClientPSMoveAPI::eClientPSMoveResultCode ResultCode,
        const ClientPSMoveAPI::t_request_id request_id,
        ClientPSMoveAPI::t_response_handle response_handle,
        void *userdata);
    void close_shared_memory_stream();

private:
    enum eTrackerMenuState
    {
        inactive,
        idle,

        pendingTrackerStartStreamRequest,
        failedTrackerStartStreamRequest,

        pendingTrackerStopStreamRequest,
        failedTrackerStopStreamRequest,
    };

    eTrackerMenuState m_menuState;
    int m_trackerID;

    bool m_bStreamIsActive;
    class SharedVideoFrameReadOnlyAccessor *m_shared_memory_accesor;
};

#endif // APP_STAGE_TEST_TRACKER_H