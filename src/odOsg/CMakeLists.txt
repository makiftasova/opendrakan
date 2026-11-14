
option(USE_FLUIDSYNTH "Whether to use FluidSynth subsystem to render music. (disabled with a message if FluidSynth is not found)" ON)

add_executable(odOsg "")

target_sources(odOsg PRIVATE
    "audio/Buffer.cpp"
    "audio/OpenAlContext.cpp"
    "audio/SoundSystem.cpp"
    "audio/Source.cpp"
    "audio/StreamingSource.cpp"
    "audio/music/DummySynth.cpp"
    "audio/music/StupidSineSynth.cpp"
    "render/Camera.cpp"
    "render/Geometry.cpp"
    "render/Group.cpp"
    "render/Handle.cpp"
    "render/Image.cpp"
    "render/LightState.cpp"
    "render/Model.cpp"
    "render/ModelBuilder.cpp"
    "render/PhysicsDebugDrawer.cpp"
    "render/Renderer.cpp"
    "render/Rig.cpp"
    "render/ShaderFactory.cpp"
    "render/Texture.cpp"
    "InputListener.cpp"
    "render/PointerGrab.cpp"
    "Settings.cpp"
    "Main.cpp")

# the RFL registers its classes statically, so the whole archive has to be linked
target_link_libraries(odOsg PRIVATE odCore "$<LINK_LIBRARY:WHOLE_ARCHIVE,dragonRfl>")

find_package(OpenSceneGraph 3.4.0 REQUIRED osgDB osgViewer osgGA osgUtil osgText)
target_link_libraries(odOsg PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
target_include_directories(odOsg PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})

# used to keep the mouse pointer inside the window. optional
find_package(X11)
if(X11_FOUND)
    target_link_libraries(odOsg PRIVATE X11::X11)
    target_compile_definitions(odOsg PRIVATE OD_HAVE_X11)
endif()

find_package(OpenAL REQUIRED)
target_link_libraries(odOsg PRIVATE OpenAL::OpenAL)

if(USE_FLUIDSYNTH)
    # FluidSynth is only available as a module via PkgConfig as of 2.0.6
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(FLUID IMPORTED_TARGET fluidsynth)
        if(FLUID_FOUND)
            target_link_libraries(odOsg PRIVATE PkgConfig::FLUID)
            target_compile_definitions(odOsg PRIVATE USE_FLUIDSYNTH)
            target_sources(odOsg PRIVATE "audio/music/FluidSynth.cpp" "audio/music/DlsTranscoder.cpp")
        else()
            message("No FluidSynth found. Disabling music.")
        endif()
    else()
        message("No pkgconfig found. pkgconfig is required to find FluidSynth. Disabling music.")
    endif()
endif()
