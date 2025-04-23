#include <godot_cpp/godot.hpp>               // pulls in gdnative/GDExtension API
#include <godot_cpp/core/class_db.hpp>

#include "RubberbandShift.hpp"

using namespace godot;

void initialize_formant_pitch_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    ClassDB::register_class<RubberbandShift>();
    ClassDB::register_class<RubberbandShiftInstance>();
}

extern "C" {

    // Initialization.
    
    GDExtensionBool GDE_EXPORT
    formant_lib_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);
        init_obj.register_initializer(initialize_formant_pitch_extension);
    
        return init_obj.init();
    }
}