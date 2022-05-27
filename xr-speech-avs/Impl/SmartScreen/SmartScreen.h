 /*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright 2020 Metrological
 * Licensed under the Apache License, Version 2.0
 *
 */

#pragma once

#ifndef MODULE_NAME
#define MODULE_NAME Core
#endif
#include <SmartScreen/SampleApp/SampleApplication.h>
#include "ThunderVoiceHandler.h"
#include "ThunderInputManager.h"
#include <WPEFramework/core/core.h>

#include <VoiceToApps/VoiceToApps.h>
#include <VoiceToApps/VideoSkillInterface.h>

#include <vector>

namespace WPEFramework {


    class SmartScreen
        : public Core::Thread,
          private alexaSmartScreenSDK::sampleApp::SampleApplication {
    public:
        SmartScreen()
            :v_writer(nullptr)
			,aspInputInteractionHandler(nullptr),m_isStarted(false)
            , m_thunderInputManager(nullptr)
            , m_thunderVoiceHandler(nullptr)
        {
           Run();
        }

        SmartScreen(const SmartScreen&) = delete;
        SmartScreen& operator=(const SmartScreen&) = delete;
        ~SmartScreen()
        {
            Stop();
            Wait(Thread::STOPPED | Thread::BLOCKED, Core::infinite);
        }
        void CreateSQSWorker(void)
        {
            std::cout << "Starting SQS Thread.." << std::endl;
            while((IsRunning() == true)){ handleReceiveSQSMessage(); }
            return;
        }

    private:
        virtual uint32_t Worker()
        {
            if ((IsRunning() == true)) {
                CreateSQSWorker();
            }
            Block();
            return (Core::infinite);
        }

    public:
        bool Initialize();
        bool Deinitialize();
        skillmapper::voiceToApps vta;

    private:
        bool Init(const std::string alexaClientConfig, const std::string smartScreenConfig);
        bool InitSDKLogs(const string& logLevel);
        bool JsonConfigToStream(std::vector<std::shared_ptr<std::istream>>& streams, const std::string& configFile);

    public:
		void Start();
		void Stop();
		void Data(const uint32_t sequenceNo, const uint8_t data[], const uint16_t length);

    private:
        std::shared_ptr<ThunderInputManager> m_thunderInputManager;
        std::shared_ptr<ThunderVoiceHandler<alexaSmartScreenSDK::sampleApp::gui::GUIManager>> m_thunderVoiceHandler;
	    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream::Writer> v_writer;
        std::shared_ptr<InteractionHandler<alexaSmartScreenSDK::sampleApp::gui::GUIManager>> aspInputInteractionHandler;		
        bool m_isStarted;
    };


}
