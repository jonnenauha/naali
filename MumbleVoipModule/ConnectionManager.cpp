#include "StableHeaders.h"
#include "ConnectionManager.h"
#include "MumbleVoipModule.h"
//#include "SoundServiceInterface.h"
#include <QDesktopServices>

#define BUILDING_DLL
#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL
#include "Connection.h"
#include "PCMAudioFrame.h"

namespace MumbleVoip
{
    void LibMumbleMainloopThread::run()
    {
        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
        MumbleVoipModule::LogDebug("Mumble library mainloop started");
        try
        {
            mumble_lib->Run();
        }
        catch(...)
        {
            MumbleVoipModule::LogError("Mumble library mainloop stopped by exception.");
        }
        MumbleVoipModule::LogDebug("Mumble library mainloop stopped");
    }

    ConnectionManager::ConnectionManager(Foundation::Framework* framework) :
        mumble_lib(0),
        framework_(framework),
        audio_playback_channel_(0),
        sending_audio_(false),
        recording_device_("")
    {
        StartMumbleLibrary();
    }

    ConnectionManager::~ConnectionManager()
    {
        StopMumbleLibrary();
    }


    void ConnectionManager::OpenConnection(ServerInfo info)
    {
        Connection* connection = new Connection(info);
        connections_[info.server] = connection;
        connection->Join(info.channel);
        connection->SendAudio(true); // test here
        QObject::connect( connection, SIGNAL(AudioDataAvailable(short*, int)), this, SLOT(OnAudioDataFromConnection(short*, int)) );
        QObject::connect( connection, SIGNAL(AudioFramesAvailable(Connection*)), this, SLOT(OnAudioFramesAvailable(Connection*)) );
        StartMumbleLibrary();
    }

    void ConnectionManager::CloseConnection(ServerInfo info)
    {
        if (connections_.contains(info.server))
        {
            Connection* connection = connections_[info.server];
            connection->Close();
            connections_.remove(info.server);
        }
    }

    // static
    void ConnectionManager::StartMumbleClient(const QString& server_url)
    {
        QUrl url(server_url);
        if (!url.isValid())
        {
            QString error_message = QString("Url '%1' is invalid.").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
        
        if (! QDesktopServices::openUrl(server_url))
        {
            QString error_message = QString("Cannot find handler application for url: %1").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
    }

    //static
    void ConnectionManager::KillMumbleClient()
    {
        // Evil hack to ensure that voip connection is not remaining open when using native mumble client
        QProcess kill_mumble;
        MumbleVoipModule::LogDebug("Try to kill mumble.exe process.");
        kill_mumble.start("taskkill /F /FI \"IMAGENAME eq mumble.exe"); // Works only for Windows
    }

    void ConnectionManager::StartMumbleLibrary()
    {
        mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
                
        lib_thread_.start();
    }

    void ConnectionManager::StopMumbleLibrary()
    {
        mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }

        mumble_lib->Shutdown();
        lib_thread_.wait();
        MumbleVoipModule::LogDebug("Mumble library uninitialized.");
    }


    void ConnectionManager::OnAudioFramesAvailable(Connection* connection)
    {
        for(;;)
        {
            PCMAudioFrame* frame = connection->GetAudioFrame();
            if (!frame)
                break;
            PlaybackAudioFrame(frame);
        }
    }
#include <QFile>

    void ConnectionManager::PlaybackAudioFrame(PCMAudioFrame* frame)
    {
        if (!framework_)
            return;
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            return;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;     

        QFile f("audio_out.raw");
        f.open(QIODevice::WriteOnly | QIODevice::Append);
        f.write(frame->Data(), frame->GetLengthBytes());
        f.close();

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = frame->Data();
        sound_buffer.frequency_ = frame->SampleRate();

        if (frame->SampleWidth() == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = frame->GetLengthBytes();
        sound_buffer.stereo_ = false;

        audio_playback_channel_ = soundsystem->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channel_);

        delete frame;
    }

    void ConnectionManager::OnAudioDataFromConnection(short* data, int size)
    {
        if (!framework_)
            return;
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            return;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;     


        int sample_rate = 48000; // test
        int sample_width = 16; // test
        int channel_count = 1; // test
        bool stereo = false; // test
        int spatial_audio_playback_ = true; // test

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = data;
        sound_buffer.frequency_ = sample_rate;

        if (sample_width == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = 48000 / 100 * 2;
        sound_buffer.stereo_ = stereo;
        if (size > 0 && sample_rate != -1 && sample_width != -1 && (channel_count == 1 || channel_count == 2) )
        {

            //if (spatial_audio_playback_)
            //    audio_playback_channel_ = soundsystem->PlaySoundBuffer3D(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_position_, audio_playback_channel_);
            //else
                audio_playback_channel_ = soundsystem->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channel_);
        }
        delete [] data;

    }

    void ConnectionManager::SendAudio(bool send)
    {
        if (!framework_)
            throw std::exception("Framework cannot be found.");
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            throw std::exception("service_manager cannot be found.");
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            throw std::exception("SoundServiceInterface cannot be found.");

        //StringVector devices = soundsystem->GetRecordingDevices();
        //if (devices.size() == 0)
        //    throw std::exception("No recording devices.");

        if (!sending_audio_ && send)
        {
            sending_audio_ = true;
            int frequency = AUDIO_SAMPLE_RATE_;
            bool sixteenbit = true;
            bool stereo = true;
            int buffer_size_ms = AUDIO_RECORDING_BUFFER_MS;
            int buffer_size = buffer_size_ms*2*frequency/1000;
            soundsystem->StartRecording(recording_device_, frequency, sixteenbit, stereo, buffer_size);
        }

        if (sending_audio_ && !send)
        {
            sending_audio_ = false;
            soundsystem->StopRecording();
        }
    }

    bool ConnectionManager::SendingAudio()
    {
        return sending_audio_;
    }

    void ConnectionManager::Update(f64 frametime)
    {
        if (sending_audio_)
        {
            boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = ConnectionManager::SoundService();
            if (!sound_service)
            {
                MumbleVoipModule::LogDebug("Soundservice cannot be found.");
                return;
            }

            while (sound_service->GetRecordedSoundSize() > AUDIO_FRAME_SIZE_IN_SAMPLES*2)
            {
                int bytes = sound_service->GetRecordedSoundData(playback_buffer_, AUDIO_FRAME_SIZE_IN_SAMPLES*2);
                PCMAudioFrame* frame = new PCMAudioFrame(AUDIO_SAMPLE_RATE_, 16, 2, playback_buffer_, bytes);
                for (QMap<QString, Connection*>::iterator i = connections_.begin(); i != connections_.end(); ++i)
                {
                    Connection* connection = *i;
                    if (connection->SendingAudio())
                        connection->SendAudioFrame(frame);
                }
            }
        }
    }

    boost::shared_ptr<Foundation::SoundServiceInterface> ConnectionManager::SoundService()
    {
        if (!framework_)
            return boost::shared_ptr<Foundation::SoundServiceInterface>();
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            return boost::shared_ptr<Foundation::SoundServiceInterface>();
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return boost::shared_ptr<Foundation::SoundServiceInterface>();

        return soundsystem;
    }

    // GetRecordedSoundData(void* buffer, uint size)

} // end of namespace: MumbleVoip

