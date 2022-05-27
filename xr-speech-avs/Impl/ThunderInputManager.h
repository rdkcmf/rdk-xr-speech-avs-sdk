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
#include <VoiceToApps/VoiceToApps.h>
#include <rdkx_logger.h>

#include <AVS/SampleApp/InteractionManager.h>
#include <AVSCommon/SDKInterfaces/MessageObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/TemplateRuntimeObserverInterface.h>
#include <acsdkAudioPlayerInterfaces/AudioPlayerObserverInterface.h>
#if defined(ENABLE_SMART_SCREEN_SUPPORT)
#include <SmartScreen/SampleApp/SampleApplication.h>
#endif

#include <atomic>

namespace WPEFramework {


    /// Observes user input from the console and notifies the interaction manager of the user's intentions.
    class ThunderInputManager
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface,
          public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateObserverInterface,
          public alexaClientSDK::registrationManager::RegistrationObserverInterface,
          public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface,
		  public alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface,
          public alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface,
		  public alexaClientSDK::avsCommon::sdkInterfaces::MessageObserverInterface {
    public:
        static std::unique_ptr<ThunderInputManager> create(std::shared_ptr<alexaClientSDK::sampleApp::InteractionManager> interactionManager);
        #if defined(ENABLE_SMART_SCREEN_SUPPORT)
        static std::unique_ptr<ThunderInputManager> create(std::shared_ptr<alexaSmartScreenSDK::sampleApp::gui::GUIManager> guiManager);
        #endif
	skillmapper::voiceToApps vta;
	int m_vtaFlag;
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerState;
	
        void onLogout() override;
        void onDialogUXStateChanged(DialogUXState newState) override;
		void receive(const std::string& contextId, const std::string& message) override;

        //audio Player 
        void onPlayerActivityChanged (alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context &context) override;
        bool isAudioPlaying(void); 
		//template card
        void renderTemplateCard(const std::string& jsonPayload, alexaClientSDK::avsCommon::avs::FocusState focusState) override;
        void    clearTemplateCard () override; 
        void    renderPlayerInfoCard (const std::string &jsonPayload, TemplateRuntimeObserverInterface::AudioPlayerInfo
        info, alexaClientSDK::avsCommon::avs::FocusState focusState) override;
        void    clearPlayerInfoCard () override;

		void NotifyDialogUXStateChanged(DialogUXState newState);

    private:
        ThunderInputManager(std::shared_ptr<alexaClientSDK::sampleApp::InteractionManager> interactionManager);
#if defined(ENABLE_SMART_SCREEN_SUPPORT)
         ThunderInputManager(std::shared_ptr<alexaSmartScreenSDK::sampleApp::gui::GUIManager> guiManager);
#endif
        void onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) override;
		void onCapabilitiesStateChange (CapabilitiesDelegateObserverInterface::State newState, CapabilitiesDelegateObserverInterface::Error newError, const std::vector< std::string > &addedOrUpdatedEndpointIds, const std::vector< std::string > &deletedEndpointIds) override;

        std::shared_ptr<alexaClientSDK::sampleApp::InteractionManager> m_interactionManager;
       #if defined(ENABLE_SMART_SCREEN_SUPPORT)
               std::shared_ptr<alexaSmartScreenSDK::sampleApp::gui::GUIManager> m_guiManager;
       #endif
        std::atomic_bool m_limitedInteraction;
    };


} // namespace WPEFramework
