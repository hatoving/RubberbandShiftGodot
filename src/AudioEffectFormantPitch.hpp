#pragma once

#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>

#include <memory>
#include <rubberband/RubberBandStretcher.h>

namespace godot {
    class FormantPitchEffect;
    class FormantPitchEffectInstance;

    class FormantPitchEffectInstance : public AudioEffectInstance {
      GDCLASS(FormantPitchEffectInstance, AudioEffectInstance);
      friend class FormantPitchEffect;
      Ref<FormantPitchEffect> base;

      std::vector<float> accumulated_left;
      std::vector<float> accumulated_right;
    protected:
      static void _bind_methods();  // ← Add this line
      std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;

    public:
      void _process(const void *p_src_buffer, AudioFrame *p_dst_buffer, int32_t p_frame_count) override;

      FormantPitchEffectInstance();
      ~FormantPitchEffectInstance();
  };

    class FormantPitchEffect : public AudioEffect {
        GDCLASS(FormantPitchEffect, AudioEffect);
        friend class FormantPitchEffectInstance;
        bool preserve_formants = true;
        float pitch_semitones = 0.0f;
        Ref<FormantPitchEffectInstance> current_instance;
      protected:
        static void _bind_methods();
      public:
        Ref<AudioEffectInstance> _instantiate() override;

        void set_pitch_semitones(float p_st);
        float get_pitch_semitones() const;
        void set_preserve_formants(bool p_enable);
        bool get_preserve_formants() const;

    };

}