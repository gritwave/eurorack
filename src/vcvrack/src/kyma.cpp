#include "plugin.hpp"

#include "gw/eurorack/hades.hpp"

struct Kyma final : Module
{
    enum ParamId
    {
        PITCH_PARAM,
        MORPH_PARAM,
        ATTACK_PARAM,
        RELEASE_PARAM,

        SELECT_PARAM,

        PARAMS_LEN
    };

    enum InputId
    {
        FM_MODULATOR_INPUT,
        FM_AMOUNT_INPUT,

        PITCH_INPUT,
        MORPH_INPUT,
        SUB_GAIN_INPUT,
        SUB_MORPH_INPUT,

        ENV_GATE_INPUT,
        CLOCK_GATE_INPUT,

        INPUTS_LEN
    };

    enum OutputId
    {
        SUB_OUTPUT,
        MAIN_OUTPUT,

        ENV_OUTPUT,

        CLOCK_HALF_OUTPUT,
        CLOCK_QUARTER_OUTPUT,

        OUTPUTS_LEN
    };

    enum LightId
    {
        ENV_LIGHT,
        SELECT_LIGHT,
        LIGHTS_LEN
    };

    Kyma()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(PITCH_PARAM, 0.0F, 10.0F, 0.0F, "Pitch", " V");
        configParam(MORPH_PARAM, 0.0F, 1.0F, 0.0F, "Wavetable position", "%", 0.0F, 100.0F);
        configParam(ATTACK_PARAM, 0.0F, 1.0F, 0.1F, "Attack", "ms", 0.0F, 500.0F);
        configParam(RELEASE_PARAM, 0.0F, 1.0F, 0.1F, "Release", "ms", 0.0F, 500.0F);

        configButton(SELECT_PARAM, "Select");

        configInput(FM_MODULATOR_INPUT, "FM modulator");
        configInput(FM_AMOUNT_INPUT, "FM amount");
        configInput(PITCH_INPUT, "1V/octave pitch");
        configInput(MORPH_INPUT, "Wavetable position");
        configInput(SUB_GAIN_INPUT, "Sub gain");
        configInput(SUB_MORPH_INPUT, "Sub wavetable position");

        configOutput(SUB_OUTPUT, "Sub");
        configOutput(MAIN_OUTPUT, "Main");
        configOutput(ENV_OUTPUT, "Envelope");
        configOutput(CLOCK_HALF_OUTPUT, "1/2 Clock");
        configOutput(CLOCK_QUARTER_OUTPUT, "1/4 Clock");
    }

    void process(ProcessArgs const& args) override
    {
        auto const select = params[SELECT_PARAM].getValue();
        lights[SELECT_LIGHT].setBrightness(select);
    }
};

struct KymaWidget final : ModuleWidget
{
    KymaWidget(Kyma* module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/kyma.svg")));

        addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.0, 22.0)), module, Kyma::PITCH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.0, 42.0)), module, Kyma::MORPH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(40.0, 22.0)), module, Kyma::ATTACK_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(40.0, 42.0)), module, Kyma::RELEASE_PARAM));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.0, 60.0)), module, Kyma::ENV_OUTPUT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(8.0, 60.0)), module, Kyma::SELECT_PARAM,
                                                                     Kyma::SELECT_LIGHT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0, 84.0)), module, Kyma::ENV_GATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.0, 84.0)), module, Kyma::CLOCK_GATE_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.0, 84.0)), module, Kyma::CLOCK_HALF_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.0, 84.0)), module, Kyma::CLOCK_QUARTER_OUTPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0, 98.0)), module, Kyma::PITCH_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.0, 98.0)), module, Kyma::MORPH_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.0, 98.0)), module, Kyma::SUB_GAIN_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.0, 98.0)), module, Kyma::SUB_MORPH_INPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0, 112.0)), module, Kyma::FM_MODULATOR_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.0, 112.0)), module, Kyma::FM_AMOUNT_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.0, 112.0)), module, Kyma::SUB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.0, 112.0)), module, Kyma::MAIN_OUTPUT));
    }
};

Model* modelKyma = createModel<Kyma, KymaWidget>("Kyma");
