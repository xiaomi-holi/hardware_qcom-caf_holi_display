/*
Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
Changes from Qualcomm Innovation Center are provided under the following license:
Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Qualcomm Innovation Center, Inc. nor the
      names of its contributors may be used to endorse or promote
      products derived from this software without specific prior
      written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __HW_PERIPHERAL_DRM_H__
#define __HW_PERIPHERAL_DRM_H__

#include <map>
#include <vector>
#include <string>
#include "hw_device_drm.h"

namespace sdm {

struct CWBConfig {
  bool enabled = false;
  sde_drm::DRMDisplayToken token = {};
};

enum SelfRefreshState {
  kSelfRefreshNone,
  kSelfRefreshEnable,
  kSelfRefreshDisable,
};

class HWPeripheralDRM : public HWDeviceDRM, public PanelFeaturePropertyIntf {
 public:
  explicit HWPeripheralDRM(int32_t display_id, BufferAllocator *buffer_allocator,
                           HWInfoInterface *hw_info_intf);
  virtual ~HWPeripheralDRM() {}
  virtual PanelFeaturePropertyIntf *GetPanelFeaturePropertyIntf() { return this; }
  virtual int GetPanelFeature(PanelFeaturePropertyInfo *feature_info);
  virtual int SetPanelFeature(const PanelFeaturePropertyInfo &feature_info);
  virtual DisplayError GetFeatureSupportStatus(const HWFeature feature, uint32_t *status);
  virtual DisplayError GetPanelBrightnessBasePath(std::string *base_path) const;

 protected:
  virtual DisplayError Init();
  virtual DisplayError Validate(HWLayers *hw_layers);
  virtual DisplayError Commit(HWLayers *hw_layers);
  virtual DisplayError Flush(HWLayers *hw_layers);
  virtual DisplayError SetDppsFeature(void *payload, size_t size);
  virtual DisplayError GetDppsFeatureInfo(void *payload, size_t size);
  virtual DisplayError HandleSecureEvent(SecureEvent secure_event, const HWQosData &qos_data);
  virtual DisplayError ControlIdlePowerCollapse(bool enable, bool synchronous);
  virtual DisplayError PowerOn(const HWQosData &qos_data, shared_ptr<Fence> *release_fence);
  virtual DisplayError PowerOff(bool teardown);
  virtual DisplayError Doze(const HWQosData &qos_data, shared_ptr<Fence> *release_fence);
  virtual DisplayError DozeSuspend(const HWQosData &qos_data, shared_ptr<Fence> *release_fence);
  virtual DisplayError SetDisplayDppsAdROI(void *payload);
  virtual DisplayError SetDynamicDSIClock(uint64_t bit_clk_rate);
  virtual DisplayError GetDynamicDSIClock(uint64_t *bit_clk_rate);
  virtual DisplayError SetDisplayAttributes(uint32_t index);
  virtual DisplayError SetDisplayMode(const HWDisplayMode hw_display_mode);
  virtual DisplayError SetRefreshRate(uint32_t refresh_rate);
  virtual DisplayError SetFrameTrigger(FrameTriggerMode mode);
  virtual DisplayError SetPanelBrightness(int level);
  virtual DisplayError GetPanelBrightness(int *level);
  virtual void GetHWPanelMaxBrightness();
  virtual DisplayError SetBLScale(uint32_t level);
  virtual DisplayError EnableSelfRefresh();
  virtual  DisplayError TeardownConcurrentWriteback(void);

 private:
  void InitDestScaler();
  void SetDestScalarData(const HWLayersInfo &hw_layer_info);
  void ResetDestScalarCache();
  DisplayError SetupConcurrentWritebackModes();
  bool SetupConcurrentWriteback(const HWLayersInfo &hw_layer_info, bool validate,
                                int64_t *release_fence_fd);
  void ConfigureConcurrentWriteback(const HWLayersInfo &hw_layer_info);
  void PostCommitConcurrentWriteback(LayerBuffer *output_buffer);
  void CreatePanelFeaturePropertyMap();
  void SetIdlePCState() {
    drm_atomic_intf_->Perform(sde_drm::DRMOps::CRTC_SET_IDLE_PC_STATE, token_.crtc_id,
                              idle_pc_state_);
  }
  void CacheDestScalarData();
  void SetSelfRefreshState();
  void SetVMReqState();
  void ResetPropertyCache();

  struct DestScalarCache {
    SDEScaler scalar_data = {};
    uint32_t flags = {};
  };

  sde_drm_dest_scaler_data sde_dest_scalar_data_ = {};
  std::vector<SDEScaler> scalar_data_ = {};
  CWBConfig cwb_config_ = {};
  bool has_cwb_crop_ = false;
  bool has_dedicated_cwb_ = false;
  sde_drm::DRMIdlePCState idle_pc_state_ = sde_drm::DRMIdlePCState::NONE;
  bool idle_pc_enabled_ = true;
  std::vector<DestScalarCache> dest_scalar_cache_ = {};
  drm_msm_ad4_roi_cfg ad4_roi_cfg_ = {};
  bool needs_ds_update_ = false;
  void PopulateBitClkRates();
  std::vector<uint64_t> bitclk_rates_;
  std::string brightness_base_path_ = "";
  SelfRefreshState self_refresh_state_ = kSelfRefreshNone;
  bool ltm_hist_en_ = false;
  uint32_t cwb_cached_conn_id_ = 0;
  std::map<PanelFeaturePropertyID, sde_drm::DRMPanelFeatureID> panel_feature_property_map_ {};
};

}  // namespace sdm

#endif  // __HW_PERIPHERAL_DRM_H__
