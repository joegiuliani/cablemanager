#pragma once

// The step function could be staticized, but I like that classes can own an Animation
// object if they really want to.

// Fire and forget strategy for animations.
// Call animations_step() in your main loop.
// 
// Only floats are supported for animation.
// 
// To animate a float, just call animate(...). Visit it's implentation for details.
// 
// Animations work using a passed delta time for each step.
// 
// Each animation step checks if an animation has concluded, and then removes it from the animations store.
// 
// 
// 
// Interpolation types
// The actual functions used for each type can be found in the source file.

// ID used to query whether a given animation exists.
// ID is placed in a map along with an animation struct as its value.
// When an animation concludes, it and it's corresponding key are removed from the map.
typedef void* TLAnimID;
typedef unsigned int TLAtlasID;

// Interpolation types corresponding to functions implemented in the source file.
enum class TLInterp
{
	LINEAR, CONSTANT, FAST_SLOW, SMOOTH_LINEAR
};

namespace TL
{
	void animate(TLAnimID id, float& object, float target, float duration, TLInterp interp = TLInterp::LINEAR);
	void animations_step(float delta);
	bool is_animating(TLAnimID id);
	void reverse(TLAnimID id);
	void extend_target(TLAnimID id, float new_target);
}


