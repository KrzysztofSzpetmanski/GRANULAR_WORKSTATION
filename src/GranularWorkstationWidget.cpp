#include "GranularWorkstationModule.hpp"

#include <string>

using namespace rack;

namespace {

struct PanelLabel : TransparentWidget {
    std::string text;
    int fontSize = 8;
    int align = NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE;
    NVGcolor color = nvgRGB(0x0f, 0x17, 0x2a);

    void draw(const DrawArgs& args) override {
        auto font = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
        if (!font) {
            return;
        }
        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, static_cast<float>(fontSize));
        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, align);
        nvgText(args.vg, 0.f, 0.f, text.c_str(), nullptr);
    }
};

} // namespace

struct GranularWorkstationWidget : ModuleWidget {
    explicit GranularWorkstationWidget(GranularWorkstation* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GranularWorkstation.svg")));

        const float x10 = 10.0f;
        const float x25 = 25.0f;
        const float x40 = 40.0f;
        const float x55 = 55.0f;
        const float x70 = 70.0f;
        const float y23 = 23.0f;
        const float y38 = 38.0f;
        const float y53 = 53.0f;
        const float y83 = 83.0f;
        const float y98 = 98.0f;
        const float y113 = 113.0f;

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x10, y23)), module, GranularWorkstation::MIX_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x25, y23)), module, GranularWorkstation::SIZE_REV_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x40, y23)), module, GranularWorkstation::TEXTURE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x55, y23)), module, GranularWorkstation::DENSITY_PARAM));

        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x10, y38)), module, GranularWorkstation::OVERLAP_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x25, y38)), module, GranularWorkstation::POSITION_SPREAD_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x40, y38)), module, GranularWorkstation::PITCH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x55, y38)), module, GranularWorkstation::LOFIHI_PARAM));

        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x10, y53)), module, GranularWorkstation::SPACE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x25, y53)), module, GranularWorkstation::FEEDBACK_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x40, y53)), module, GranularWorkstation::DAMP_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x55, y53)), module, GranularWorkstation::REVERB_PARAM));

        addParam(createParamCentered<LEDButton>(mm2px(Vec(x70, y53)), module, GranularWorkstation::FREEZE_PARAM));
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(x70, y53)), module, GranularWorkstation::FREEZE_LIGHT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x10, y83)), module, GranularWorkstation::IN_L_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x25, y83)), module, GranularWorkstation::IN_R_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x40, y83)), module, GranularWorkstation::OUT_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x55, y83)), module, GranularWorkstation::OUT_R_OUTPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x70, y83)), module, GranularWorkstation::CLOCK_INPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x10, y98)), module, GranularWorkstation::SIZE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x25, y98)), module, GranularWorkstation::DENSITY_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x40, y98)), module, GranularWorkstation::POSITION_SPREAD_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x55, y98)), module, GranularWorkstation::PITCH_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x70, y98)), module, GranularWorkstation::FEEDBACK_CV_INPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x10, y113)), module, GranularWorkstation::MIX_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x25, y113)), module, GranularWorkstation::FREEZE_GATE_INPUT));

        auto addPanelLabel = [this](float xMm, float yMm, const std::string& txt, int size = 8, NVGcolor color = nvgRGB(0x0f, 0x17, 0x2a)) {
            auto* label = createWidget<PanelLabel>(mm2px(Vec(xMm, yMm)));
            label->text = txt;
            label->fontSize = size;
            label->color = color;
            addChild(label);
        };

        addPanelLabel(40.5f, 8.2f, "GRANULAR WORKSTATION", 10, nvgRGB(0x0b, 0x12, 0x20));
        addPanelLabel(40.5f, 14.2f, "LIVE GRANULAR REVERB LAB", 7, nvgRGB(0x1f, 0x29, 0x37));

        addPanelLabel(x10, y23 - 8.0f, "MIX", 7);
        addPanelLabel(x25, y23 - 8.0f, "SIZE/REV", 7);
        addPanelLabel(x40, y23 - 8.0f, "TEXTURE", 7);
        addPanelLabel(x55, y23 - 8.0f, "DENSITY", 7);

        addPanelLabel(x10, y38 - 8.0f, "OVERLAP", 7);
        addPanelLabel(x25, y38 - 8.0f, "POS/SPREAD", 7);
        addPanelLabel(x40, y38 - 8.0f, "PITCH", 7);
        addPanelLabel(x55, y38 - 8.0f, "LOFIHI", 7);

        addPanelLabel(x10, y53 - 8.0f, "SPACE", 7);
        addPanelLabel(x25, y53 - 8.0f, "FEEDBACK", 7);
        addPanelLabel(x40, y53 - 8.0f, "DAMP", 7);
        addPanelLabel(x55, y53 - 8.0f, "REVERB", 7);
        addPanelLabel(x70, y53 - 8.0f, "FREEZE", 7);

        addPanelLabel(x10, y83 - 8.0f, "IN L", 7);
        addPanelLabel(x25, y83 - 8.0f, "IN R", 7);
        addPanelLabel(x40, y83 - 8.0f, "OUT L", 7);
        addPanelLabel(x55, y83 - 8.0f, "OUT R", 7);
        addPanelLabel(x70, y83 - 8.0f, "CLOCK", 7);

        addPanelLabel(x10, y98 - 8.0f, "SIZE CV", 7);
        addPanelLabel(x25, y98 - 8.0f, "DENS CV", 7);
        addPanelLabel(x40, y98 - 8.0f, "POS CV", 7);
        addPanelLabel(x55, y98 - 8.0f, "PITCH CV", 7);
        addPanelLabel(x70, y98 - 8.0f, "FB CV", 7);

        addPanelLabel(x10, y113 - 8.0f, "MIX CV", 7);
        addPanelLabel(x25, y113 - 8.0f, "FRZ GATE", 7);
    }

    void appendContextMenu(Menu* menu) override {
        ModuleWidget::appendContextMenu(menu);

        auto* module = dynamic_cast<GranularWorkstation*>(this->module);
        menu->addChild(new MenuSeparator());

        struct ModeItem : MenuItem {
            GranularWorkstation* module = nullptr;
            granular::GranMode mode = granular::GranMode::CloudRev;

            void onAction(const event::Action& e) override {
                if (module) {
                    module->setMode(mode);
                }
                MenuItem::onAction(e);
            }

            void step() override {
                rightText = CHECKMARK(module && module->getMode() == mode);
                MenuItem::step();
            }
        };

        auto addMode = [&](const std::string& label, granular::GranMode mode) {
            ModeItem* item = createMenuItem<ModeItem>(label);
            item->module = module;
            item->mode = mode;
            menu->addChild(item);
        };

        menu->addChild(createMenuLabel("Mode"));
        addMode("Cloud Rev", granular::GranMode::CloudRev);
        addMode("Ghost Delay", granular::GranMode::GhostDelay);
        addMode("Freeze Space", granular::GranMode::FreezeSpace);
        addMode("Time Stretch", granular::GranMode::TimeStretch);
        addMode("Pitch Shift", granular::GranMode::PitchShift);
        addMode("Micro Loop", granular::GranMode::MicroLoop);
        addMode("Pulsar", granular::GranMode::Pulsar);
    }
};

Model* modelGRANULAR_WORKSTATION = createModel<GranularWorkstation, GranularWorkstationWidget>("GRANULAR_WORKSTATION");
