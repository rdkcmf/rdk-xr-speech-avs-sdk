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

#include "CompatibleAudioFormat.h"
#include <rdkx_logger.h>

#include <Audio/MicrophoneInterface.h>

#include <AVS/SampleApp/InteractionManager.h>
#if defined(ENABLE_SMART_SCREEN_SUPPORT)
#include <SmartScreen/SampleApp/GUI/GUIManager.h>
#endif

#include <mutex>
#include <thread>

namespace WPEFramework {

    /// Responsible for making an interaction on audio data
    template <typename MANAGER>
    class InteractionHandler {
    public:
        static std::unique_ptr<InteractionHandler> Create()
        {
            std::unique_ptr<InteractionHandler<MANAGER>> interactionHandler(new InteractionHandler<MANAGER>());
            if (!interactionHandler) {
                XLOGD_ERROR("Failed to create a interaction handler!");
                return nullptr;
            }

            return interactionHandler;
        }

        InteractionHandler(const InteractionHandler&) = delete;
        InteractionHandler& operator=(const InteractionHandler&) = delete;
        ~InteractionHandler() = default;

    private:
        InteractionHandler()
            : m_interactionManager{ nullptr }
        {
        }

    public:
        bool Initialize(std::shared_ptr<MANAGER> interactionManager)
        {
            bool status = false;
            if (interactionManager) {
                m_interactionManager = interactionManager;
                status = true;
            }
            return status;
        }

        bool Deinitialize()
        {
            bool status = false;
            if (m_interactionManager) {
                m_interactionManager.reset();
                status = true;
            }
            return status;
        }

        void HoldToTalk();

    private:
        std::shared_ptr<MANAGER> m_interactionManager;
    };

    template <>
    inline void InteractionHandler<alexaClientSDK::sampleApp::InteractionManager>::HoldToTalk()
    {
        if (m_interactionManager) {
            m_interactionManager->holdToggled();
        }
    }

#if defined(ENABLE_SMART_SCREEN_SUPPORT)
    template <>
    inline void InteractionHandler<alexaSmartScreenSDK::sampleApp::gui::GUIManager>::HoldToTalk()
    {
        if (m_interactionManager) {
            m_interactionManager->handleHoldToTalk();
        }
    }
#endif

    // This class provides the audio input from Thunder
    template <typename MANAGER>
    class ThunderVoiceHandler : public alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface {
    public:
        static std::unique_ptr<ThunderVoiceHandler> create(std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream, alexaClientSDK::avsCommon::utils::AudioFormat audioFormat)
        {
            if (!stream) {
                XLOGD_ERROR("Invalid stream");
                return nullptr;
            }

            if (!AudioFormatCompatibility::IsCompatible(audioFormat)) {
                XLOGD_ERROR("Audio Format is not compatible");
                return nullptr;
            }

            std::unique_ptr<ThunderVoiceHandler> thunderVoiceHandler(new ThunderVoiceHandler(stream));
            if (!thunderVoiceHandler) {
                XLOGD_ERROR("Failed to create a ThunderVoiceHandler!");
                return nullptr;
            }

            if (!thunderVoiceHandler->Initialize()) {
                XLOGD_ERROR("ThunderVoiceHandler is not initialized.");
            }

            return thunderVoiceHandler;
        }

        bool stopStreamingMicrophoneData() override
        {
            XLOGD_DEBUG("stopStreamingMicrophoneData()");

            return true;
        }

        bool startStreamingMicrophoneData() override
        {
            XLOGD_DEBUG("startStreamingMicrophoneData()");

            return true;
        }

	bool isStreaming() override {
		return true;
	}

		std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream::Writer> GetWriteHandler()
		{
			return m_writer;
		}

		
        ~ThunderVoiceHandler()
        {

        }

    private:
        ThunderVoiceHandler(std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream)
            : m_audioInputStream{ stream }
	    , m_writer{nullptr}
            , m_isInitialized{ false }
        {

        }

        /// Initializes ThunderVoiceHandler.
        bool Initialize()
        {
            const std::lock_guard<std::mutex> lock{ m_mutex };
            bool error = false;

            if (m_isInitialized) {
                error = true;
            }

            if (error != true) {
                m_writer = m_audioInputStream->createWriter(alexaClientSDK::avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);
                if (m_writer == nullptr) {
                    XLOGD_ERROR("Failed to create stream writer");
                    error = true;
                }
            }			
			
            if (error != true) {
                m_isInitialized = true;
            }

            return m_isInitialized;
        }

        bool Deinitialize()
        {
            const std::lock_guard<std::mutex> lock{ m_mutex };

            if (m_writer) {
                m_writer.reset();
            }

            m_isInitialized = false;
            return true;
        }

    private:
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> m_audioInputStream;
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream::Writer> m_writer;
        std::shared_ptr<InteractionHandler<MANAGER>> m_interactionHandler;

        bool m_isInitialized;
        std::mutex m_mutex;
    };


} // namespace WPEFramework
