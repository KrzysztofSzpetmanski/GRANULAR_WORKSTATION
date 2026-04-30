#include "GranularWorkstationModule.hpp"

#include <algorithm>

#include "dsp/MathUtils.hpp"

GranularWorkstation::GranularWorkstation() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(MIX_PARAM, 0.0f, 1.0f, 0.5f, "Mix");
    configParam(SIZE_REV_PARAM, -1.0f, 1.0f, 0.2f, "Size / Reverse");
    configParam(TEXTURE_PARAM, 0.0f, 1.0f, 0.5f, "Texture");
    configParam(DENSITY_PARAM, 0.0f, 1.0f, 0.4f, "Density");
    configParam(OVERLAP_PARAM, 0.0f, 1.0f, 0.5f, "Overlap");
    configParam(POSITION_SPREAD_PARAM, 0.0f, 1.0f, 0.5f, "Position / Spread");
    configParam(PITCH_PARAM, -1.0f, 1.0f, 0.0f, "Pitch", " st", 0.0f, 24.0f);
    configParam(LOFIHI_PARAM, 0.0f, 1.0f, 1.0f, "Lofi / Hifi");
    configParam(SPACE_PARAM, 0.0f, 1.0f, 0.5f, "Space");
    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 0.3f, "Feedback");
    configParam(DAMP_PARAM, 0.0f, 1.0f, 0.6f, "Damp");
    configParam(REVERB_PARAM, 0.0f, 1.0f, 0.5f, "Reverb");
    configButton(FREEZE_PARAM, "Freeze");

    configInput(IN_L_INPUT, "Left audio");
    configInput(IN_R_INPUT, "Right audio");
    configInput(SIZE_CV_INPUT, "Size / Reverse CV");
    configInput(DENSITY_CV_INPUT, "Density CV");
    configInput(POSITION_SPREAD_CV_INPUT, "Position / Spread CV");
    configInput(PITCH_CV_INPUT, "Pitch CV");
    configInput(FEEDBACK_CV_INPUT, "Feedback CV");
    configInput(MIX_CV_INPUT, "Mix CV");
    configInput(FREEZE_GATE_INPUT, "Freeze gate");
    configInput(CLOCK_INPUT, "Clock");

    configOutput(OUT_L_OUTPUT, "Left audio");
    configOutput(OUT_R_OUTPUT, "Right audio");

    configLight(FREEZE_LIGHT, "Freeze");

    onSampleRateChange();
}

void GranularWorkstation::onSampleRateChange() {
    cachedSampleRate_ = APP->engine->getSampleRate();
    engine_.setSampleRate(cachedSampleRate_);
    engine_.reset();
}

float GranularWorkstation::param01WithCv(int paramId, int inputId) {
    float value = params[paramId].getValue();
    if (inputs[inputId].isConnected()) {
        const float cv01 = clamp(inputs[inputId].getVoltage() / 10.0f, 0.0f, 1.0f);
        value += (cv01 - 0.5f);
    }
    return clamp(value, 0.0f, 1.0f);
}

float GranularWorkstation::paramSignedWithCv(int paramId, int inputId, float cvScale) {
    float value = params[paramId].getValue();
    if (inputs[inputId].isConnected()) {
        value += inputs[inputId].getVoltage() * cvScale;
    }
    return clamp(value, -1.0f, 1.0f);
}

void GranularWorkstation::process(const ProcessArgs& args) {
    if (cachedSampleRate_ != args.sampleRate) {
        cachedSampleRate_ = args.sampleRate;
        engine_.setSampleRate(cachedSampleRate_);
    }

    const bool buttonPressed = params[FREEZE_PARAM].getValue() > 0.5f;
    if (freezeButtonTrigger_.process(buttonPressed)) {
        freezeLatched_ = !freezeLatched_;
    }

    const bool gateFreeze = inputs[FREEZE_GATE_INPUT].isConnected() && inputs[FREEZE_GATE_INPUT].getVoltage() > 1.0f;
    const bool finalFreeze = freezeLatched_ || gateFreeze;

    bool clockTick = false;
    if (inputs[CLOCK_INPUT].isConnected()) {
        clockTick = clockTrigger_.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.0f);
    }

    float inL = 0.0f;
    float inR = 0.0f;
    if (inputs[IN_L_INPUT].isConnected()) {
        inL = inputs[IN_L_INPUT].getVoltage() / 5.0f;
        inR = inputs[IN_R_INPUT].isConnected() ? inputs[IN_R_INPUT].getVoltage() / 5.0f : inL;
    }

    granular::GranularWorkstationParams p;
    p.mode = mode_;
    p.mix = param01WithCv(MIX_PARAM, MIX_CV_INPUT);
    p.sizeRev = paramSignedWithCv(SIZE_REV_PARAM, SIZE_CV_INPUT, 0.1f);
    p.texture = clamp(params[TEXTURE_PARAM].getValue(), 0.0f, 1.0f);
    p.density = param01WithCv(DENSITY_PARAM, DENSITY_CV_INPUT);
    p.overlap = clamp(params[OVERLAP_PARAM].getValue(), 0.0f, 1.0f);
    p.positionSpread = param01WithCv(POSITION_SPREAD_PARAM, POSITION_SPREAD_CV_INPUT);
    p.pitch = paramSignedWithCv(PITCH_PARAM, PITCH_CV_INPUT, 0.2f);
    p.lofiHi = clamp(params[LOFIHI_PARAM].getValue(), 0.0f, 1.0f);
    p.space = clamp(params[SPACE_PARAM].getValue(), 0.0f, 1.0f);
    p.feedback = param01WithCv(FEEDBACK_PARAM, FEEDBACK_CV_INPUT);
    p.damp = clamp(params[DAMP_PARAM].getValue(), 0.0f, 1.0f);
    p.reverb = clamp(params[REVERB_PARAM].getValue(), 0.0f, 1.0f);
    p.freeze = finalFreeze;
    p.clockTick = clockTick;

    float outL = 0.0f;
    float outR = 0.0f;
    engine_.process(inL, inR, outL, outR, p);

    outputs[OUT_L_OUTPUT].setVoltage(outL * 5.0f);
    outputs[OUT_R_OUTPUT].setVoltage(outR * 5.0f);

    lights[FREEZE_LIGHT].setBrightness(finalFreeze ? 1.0f : 0.0f);
}

json_t* GranularWorkstation::dataToJson() {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "mode", json_integer(static_cast<int>(mode_)));
    json_object_set_new(rootJ, "freezeLatched", json_boolean(freezeLatched_));
    return rootJ;
}

void GranularWorkstation::dataFromJson(json_t* rootJ) {
    json_t* modeJ = json_object_get(rootJ, "mode");
    if (modeJ) {
        const int modeInt = json_integer_value(modeJ);
        if (modeInt >= static_cast<int>(granular::GranMode::CloudRev) &&
            modeInt <= static_cast<int>(granular::GranMode::Pulsar)) {
            mode_ = static_cast<granular::GranMode>(modeInt);
        }
    }

    json_t* freezeJ = json_object_get(rootJ, "freezeLatched");
    if (freezeJ) {
        freezeLatched_ = json_boolean_value(freezeJ);
    }
}

void GranularWorkstation::setMode(granular::GranMode newMode) {
    mode_ = newMode;
}

granular::GranMode GranularWorkstation::getMode() const {
    return mode_;
}
