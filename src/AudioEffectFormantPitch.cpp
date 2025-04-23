#include "AudioEffectFormantPitch.hpp"

#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    void FormantPitchEffectInstance::_process(const void *p_src, AudioFrame *p_dst, int32_t count) {
        const AudioFrame *in = static_cast<const AudioFrame*>(p_src);
    
        double ratio = Math::pow(2.0, base->get_pitch_semitones() / 12.0);
        stretcher->setPitchScale(ratio);
        stretcher->setFormantOption(
            base->get_preserve_formants()
                ? RubberBand::RubberBandStretcher::OptionFormantPreserved
                : RubberBand::RubberBandStretcher::OptionFormantShifted
        );
    
        for (int i = 0; i < count; ++i) {
            accumulated_left.push_back(in[i].left);
            accumulated_right.push_back(in[i].right);
        }
    
        constexpr int process_chunk_size = 512;
    
        while (accumulated_left.size() >= process_chunk_size) {
            const float *in_channels[2] = {
                accumulated_left.data(),
                accumulated_right.data()
            };
            stretcher->process(in_channels, process_chunk_size, false);
    

            accumulated_left.erase(accumulated_left.begin(), accumulated_left.begin() + process_chunk_size);
            accumulated_right.erase(accumulated_right.begin(), accumulated_right.begin() + process_chunk_size);
        }
    
        // Retrieve output
        std::vector<float> out_left(count);
        std::vector<float> out_right(count);
        float *out_channels[2] = { out_left.data(), out_right.data() };
    
        int ret = stretcher->retrieve(out_channels, count);
    
        // Write to output buffer
        for (int i = 0; i < count; ++i) {
            if (i < ret) {
                p_dst[i].left = out_left[i];
                p_dst[i].right = out_right[i];
            } else {
                p_dst[i].left = p_dst[i].right = 0.0f;
            }
        }
    }

    void FormantPitchEffectInstance::_bind_methods()
    {
        // No methods or properties to bind here, but the stub must exist.
    }

    FormantPitchEffectInstance::FormantPitchEffectInstance() {
        // Initialize the vectors with a size of 0
        accumulated_left.reserve(0);
        accumulated_right.reserve(0);
    }

    FormantPitchEffectInstance::~FormantPitchEffectInstance() {
    }

    void FormantPitchEffect::_bind_methods() {
        // 1. Checkbox for formant preservation
        ClassDB::bind_method(D_METHOD("set_preserve_formants", "enable"), &FormantPitchEffect::set_preserve_formants);
        ClassDB::bind_method(D_METHOD("get_preserve_formants"), &FormantPitchEffect::get_preserve_formants);
        ADD_PROPERTY(
            PropertyInfo(Variant::BOOL, "preserve_formants"),
            "set_preserve_formants", "get_preserve_formants"
        );  // Shows a checkbox :contentReference[oaicite:6]{index=6}
    
        // 2. Slider for semitone pitch shift
        ClassDB::bind_method(D_METHOD("set_pitch_semitones", "st"), &FormantPitchEffect::set_pitch_semitones);
        ClassDB::bind_method(D_METHOD("get_pitch_semitones"), &FormantPitchEffect::get_pitch_semitones);
        ADD_PROPERTY(
            PropertyInfo(Variant::FLOAT, "pitch_semitones", PROPERTY_HINT_RANGE, "-24,24,0.1,suffix:st"),
            "set_pitch_semitones", "get_pitch_semitones"
        );  // Semitone slider from –24 to +24 :contentReference[oaicite:7]{index=7}
    }

    void FormantPitchEffect::set_pitch_semitones(float p_st) {
        pitch_semitones = p_st;
    }

    float FormantPitchEffect::get_pitch_semitones() const {
        return pitch_semitones;
    }

    void FormantPitchEffect::set_preserve_formants(bool p_enable) {
        preserve_formants = p_enable;
    }

    bool FormantPitchEffect::get_preserve_formants() const {
        return preserve_formants;
    }

    Ref<AudioEffectInstance> FormantPitchEffect::_instantiate() {
        Ref<FormantPitchEffectInstance> instance;
        instance.instantiate();  // Allocates the object behind the scenes

        instance->base = Ref<FormantPitchEffect>(this);

        int sampleRate = AudioServer::get_singleton()->get_mix_rate();
        
        instance->stretcher = std::make_unique<RubberBand::RubberBandStretcher>(
            sampleRate, 2,
            RubberBand::RubberBandStretcher::OptionProcessRealTime |
            (preserve_formants ? RubberBand::RubberBandStretcher::OptionFormantPreserved : RubberBand::RubberBandStretcher::OptionFormantShifted) |
            RubberBand::RubberBandStretcher::OptionPitchHighConsistency | RubberBand::RubberBandStretcher::OptionEngineFiner,
            1.0, 1.0
        );
        print_line("RubberBandStretcher created with sample rate: " + String::num(sampleRate));

        current_instance = instance;  // Now safe: both sides are Ref<>
        return instance;
    }

}