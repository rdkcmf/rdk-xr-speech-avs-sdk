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
 * Credit to Amazon SDK. Some code herein is obtained and modified from alexa/avs-device-sdk
 * which is under Copyright 2016-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0"
 *
 * Copyright 2020 Metrological
 * Licensed under the Apache License, Version 2.0
 *
 */

#include "ThunderInputManager.h"

namespace WPEFramework {


    using namespace alexaClientSDK::avsCommon::sdkInterfaces;

 #if defined(ENABLE_SMART_SCREEN_SUPPORT)
 
     std::unique_ptr<ThunderInputManager>
 ThunderInputManager::create(std::shared_ptr<alexaSmartScreenSDK::sampleApp::gui::GUIManager> guiManager)
     {
             if (!guiManager) {
                     XLOGD_ERROR("Invalid guiManager passed to ThunderInputManager");
                     return nullptr;
                 }
                 return std::unique_ptr<ThunderInputManager>(new ThunderInputManager(guiManager));
             }
             
             ThunderInputManager::ThunderInputManager(std::shared_ptr<alexaSmartScreenSDK::sampleApp::gui::GUIManager>
         guiManager)
                 : m_limitedInteraction{ false }
                , m_playerState{alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE}
                 , m_interactionManager{ nullptr }
                 , m_guiManager{ nullptr }
             {
                     XLOGD_DEBUG("Parsing VoiceToApps LEDs...");
                     m_vtaFlag = vta.ioParse();
		      set_vsk_msg_handler(&avs_server_msg);
                 }
                 
 #endif


    std::unique_ptr<ThunderInputManager> ThunderInputManager::create(std::shared_ptr<alexaClientSDK::sampleApp::InteractionManager> interactionManager)
    {
        if (!interactionManager) {
            XLOGD_ERROR("Invalid InteractionManager passed to ThunderInputManager");
            return nullptr;
        }
        return std::unique_ptr<ThunderInputManager>(new ThunderInputManager(interactionManager));
    }

    ThunderInputManager::ThunderInputManager(std::shared_ptr<alexaClientSDK::sampleApp::InteractionManager> interactionManager)
        : m_limitedInteraction{ false }
        , m_interactionManager{ interactionManager }
        , m_playerState{alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE}
    {
        XLOGD_DEBUG("Parsing VoiceToApps LEDs...");
        m_vtaFlag = vta.ioParse();
    }

       // to check if audioPlayer is in playing/buffering/paused state
   bool ThunderInputManager::isAudioPlaying(void)
   {
       using namespace alexaClientSDK::avsCommon::avs;
       bool state=(m_playerState == PlayerActivity::PLAYING) || 
           (m_playerState == PlayerActivity::BUFFER_UNDERRUN) ||
           (m_playerState == PlayerActivity::PAUSED);
                std::cout << __func__ << "Current Aud State =" << state <<std::endl;
                return state;
   }
  
   void ThunderInputManager::onPlayerActivityChanged (alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context &context) 

       {
       m_playerState = state; 
       std::cout << " onPlayerActivityChanged:  State change called ";
      using namespace skillmapper;
           using namespace alexaClientSDK::avsCommon::avs;
       AudioPlayerState smState;
       switch (state) {
       case PlayerActivity::IDLE:
           std::cout << "idle ";
           smState = AudioPlayerState::IDLE; 
           break;
       case PlayerActivity::PLAYING:
           std::cout << "playing ";
           smState = AudioPlayerState::PLAYING; 
           break;
       case PlayerActivity::STOPPED:
          std::cout << "stopped" ;
           smState = AudioPlayerState::STOPPED; 
           break;
       case PlayerActivity::PAUSED:
           std::cout << "paused ";
           smState = AudioPlayerState::PAUSED; 
           break;
       case PlayerActivity::BUFFER_UNDERRUN:
           std::cout << "bufferring ";
           smState = AudioPlayerState::BUFFER_UNDERRUN; 
           break;
       case PlayerActivity::FINISHED:
           std::cout << "finished ";
           smState = AudioPlayerState::FINISHED; 
           break;
       default:
           std::cout << "unknown ";
           smState = AudioPlayerState::UNKNOWN; 
       }

       std::cout << std::endl;
       vta.handleAudioPlayerStateChangeNotification(smState);
   }


    void ThunderInputManager::onDialogUXStateChanged(DialogUXState newState)
    {

        NotifyDialogUXStateChanged(newState);

    }
	
	void ThunderInputManager::renderTemplateCard(const std::string& jsonPayload, alexaClientSDK::avsCommon::avs::FocusState focusState)
    {
        if(! m_vtaFlag ) {
         XLOGD_DEBUG("VoiceToApps vtaFlag not initialized...");
          return;
        }

        XLOGD_DEBUG("VoiceToApps template card: %s ...\n", jsonPayload.c_str());
        vta.curlCmdSendOnRcvMsg(jsonPayload);
    }

    void  ThunderInputManager::renderPlayerInfoCard (const std::string &jsonPayload, 
                TemplateRuntimeObserverInterface::AudioPlayerInfo info, alexaClientSDK::avsCommon::avs::FocusState focusState) 
    {

    }

    void ThunderInputManager::clearTemplateCard() {
            
    }

    void ThunderInputManager::clearPlayerInfoCard() {

    }

    void ThunderInputManager::receive(const std::string& contextId, const std::string& message) {
        XLOGD_DEBUG( "Message received from observer..");

        if(! m_vtaFlag ) {
            XLOGD_ERROR("VoiceToApps vtaFlag not initialized...");
            return;
        }

        std::string key = "\"namespace\":\"SpeechSynthesizer\",\"name\":\"Speak\"";
        if(std::string::npos!=message.find(key,0) ){
            vta.curlCmdSendOnRcvMsg(message);
			avs_server_msg(message.c_str(), (unsigned long)message.length());
		}
    }

    void ThunderInputManager::NotifyDialogUXStateChanged(DialogUXState newState)
    {
        bool isStateHandled = true;
        using namespace skillmapper;
        bool smartScreenEnabled = 0;
       
#if defined(ENABLE_SMART_SCREEN_SUPPORT)
        smartScreenEnabled = 1;
#endif
        if (! m_vtaFlag ) {
            XLOGD_ERROR("VoiceToApps vtaFlag not initialized during state change...");
        }
    
   
        switch (newState) {
        case DialogUXState::IDLE:
            vta.handleSDKStateChangeNotification(VoiceSDKState::VTA_IDLE, smartScreenEnabled,isAudioPlaying() );
            std::cout << "Calling Smart screen notification with idle status " << std::endl;
            break;
        case DialogUXState::LISTENING:
            vta.handleSDKStateChangeNotification(VoiceSDKState::VTA_LISTENING, smartScreenEnabled,isAudioPlaying());
            break;
        case DialogUXState::EXPECTING:
            vta.handleSDKStateChangeNotification(VoiceSDKState::VTA_EXPECTING, smartScreenEnabled, isAudioPlaying());
#ifdef FILEAUDIO
            if ( vta.invocationMode ){
                 vta.fromExpecting=true;
                 vta.skipMerge=true;
            }
#endif
            break;
        case DialogUXState::THINKING:
            vta.handleSDKStateChangeNotification(VoiceSDKState::VTA_THINKING, smartScreenEnabled, isAudioPlaying() );
            break;
        case DialogUXState::SPEAKING:
            vta.handleSDKStateChangeNotification(VoiceSDKState::VTA_SPEAKING, smartScreenEnabled,isAudioPlaying() );
#ifdef FILEAUDIO
            if ( vta.fromExpecting ) {
                 vta.fromExpecting=false;
                 vta.skipMerge=false;
            }
#endif
            break;
        case DialogUXState::FINISHED:
            XLOGD_DEBUG("Unmapped Dialog state (%d)", newState);
            isStateHandled = false;
            break;
        default:
            XLOGD_DEBUG("Unknown State (%d)", newState);
            isStateHandled = false;
        }

    }

    void ThunderInputManager::onLogout()
    {
        m_limitedInteraction = true;
    }

    void ThunderInputManager::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError)
    {	
        m_limitedInteraction = m_limitedInteraction || (newState == AuthObserverInterface::State::UNRECOVERABLE_ERROR);
    }

    void ThunderInputManager::onCapabilitiesStateChange(
        CapabilitiesDelegateObserverInterface::State newState, 
		CapabilitiesDelegateObserverInterface::Error newError, 
		const std::vector< std::string > &addedOrUpdatedEndpointIds, 
		const std::vector< std::string > &deletedEndpointIds)
    {
        m_limitedInteraction = m_limitedInteraction || (newState == CapabilitiesDelegateObserverInterface::State::FATAL_ERROR);
    }


} // namespace WPEFramework
