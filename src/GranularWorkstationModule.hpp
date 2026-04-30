#pragma once

#include "plugin.hpp"
#include "dsp/GranularWorkstationEngine.hpp"

struct GranularWorkstation : Module {
    enum ParamIds {
        MIX_PARAM,
        SIZE_PARAM,
        TEXTURE_PARAM,
        DENSITY_PARAM,
        OVERLAP_PARAM,
        POSITION_SPREAD_PARAM,
        PITCH_PARAM,
        REVERSE_PARAM,
        SPACE_PARAM,
        FEEDBACK_PARAM,
        DAMP_PARAM,
        REVERB_PARAM,
        FREEZE_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        IN_L_INPUT,
        IN_R_INPUT,
        SIZE_CV_INPUT,
        DENSITY_CV_INPUT,
        POSITION_SPREAD_CV_INPUT,
        PITCH_CV_INPUT,
        FEEDBACK_CV_INPUT,
        MIX_CV_INPUT,
        FREEZE_GATE_INPUT,
        CLOCK_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        OUT_L_OUTPUT,
        OUT_R_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        FREEZE_LIGHT,
        NUM_LIGHTS
    };

    GranularWorkstation();

    void onSampleRateChange() override;
    void process(const ProcessArgs& args) override;

    json_t* dataToJson() override;
    void dataFromJson(json_t* rootJ) override;

    void setMode(granular::GranMode newMode);
    granular::GranMode getMode() const;

private:
    float param01WithCv(int paramId, int inputId);
    float paramSignedWithCv(int paramId, int inputId, float cvScale);

    granular::GranularWorkstationEngine engine_;
    granular::GranMode mode_ = granular::GranMode::CloudRev;

    bool freezeLatched_ = false;
    dsp::BooleanTrigger freezeButtonTrigger_;
    dsp::SchmittTrigger clockTrigger_;

    float cachedSampleRate_ = 0.0f;
};
