#include "animation.h"
#include <map>
#include <iostream>
#include <vector>
#include <functional>
#include <glm/common.hpp>

typedef std::function<float(float)> INTERP_FN;

// returns x
float INTERP_FN_LINEAR(float x)
{
    return x;
}

// returns 1.
float INTERP_FN_CONSTANT(float x)
{
    return 1.0f;
}

// Fast in the beginning, slow at the end.
// returns x's progress along the surface of a circle starting from PI radians to PI/2 radians.
float INTERP_FN_FAST_SLOW(float x)
{
    return sqrt(1 - (x - 1) * (x - 1)); // Plug into desmos and look at range [0,1]
}

// Positive acceleration in the beginning, negative acceleration at the end.
// returns glm::smooth_step(x,0.0f,1.0f)
float INTERP_FN_SMOOTH_LINEAR(float x)
{
    return glm::smoothstep(0.0f, 1.0f, x);
}

std::vector<TLAnimID> finished_ids;

// Since map needs a default constructor and TLAnimation has a member float&
// Used to initialize a TLAnimation struct with an empty constructor.
// Need to research if this has any undesirable effects.
float dummy_object = 0;

struct TLAnimation
{
    float& object;
    float init_val = 0, targ_val = 1, curr_dur = 0, targ_dur = 1;
    INTERP_FN inter_fn;

    TLAnimation() :object(dummy_object) {}

    TLAnimation(float& object, float target, float duration, INTERP_FN fn) :object(object)
    {
        init_val = object;
        targ_val = target;
        curr_dur = 0.0f;
        targ_dur = duration;
        inter_fn = fn;
    }
    // Returns true if we should continue to step.

    bool step(float delta)
    {
        curr_dur += delta;

        if (curr_dur >= targ_dur)
        {
            object = targ_val;
            return false;
        }

        float percent = curr_dur / targ_dur;
        percent = inter_fn(percent);
        object = percent * (targ_val - init_val) + init_val;

        return true;
    }
    void reverse()
    {
        curr_dur = targ_dur - curr_dur;
        float temp = init_val;
        init_val = targ_val;
        targ_val = temp;
    }
    void revert()
    {
        reverse();
        object = targ_val;
    }
    void conclude()
    {
        object = targ_val;
    }
    void extend_target(float new_target)
    {
        float rate = (targ_val - init_val) / targ_dur;
        targ_dur = rate * (new_target - targ_val);
        targ_val = new_target;
    }

    TLAnimation& operator=(const TLAnimation& rhs)
    {
        object = rhs.object;
        init_val = rhs.init_val;
        targ_val = rhs.targ_val;
        curr_dur = rhs.curr_dur;
        targ_dur = rhs.targ_dur;

        return *this;
    }
};

// Provides animations a way to sort its keys.
struct ptr_cmpr {
    bool operator()(void* a, void* b) const { return (intptr_t)a < (intptr_t)b; }
};
std::map<TLAnimID, TLAnimation, ptr_cmpr> animations;

// Used internally by this file.
// Returns the function pointer associated with a TLInterp enum.
INTERP_FN get_interp_fn(TLInterp interp)
{
    switch (interp)
    {
    case TLInterp::LINEAR: return INTERP_FN_LINEAR;
    case TLInterp::CONSTANT: return INTERP_FN_CONSTANT;
    case TLInterp::FAST_SLOW: return INTERP_FN_FAST_SLOW;
    case TLInterp::SMOOTH_LINEAR: return INTERP_FN_SMOOTH_LINEAR;
    default: throw std::exception("Invalid TLInterp passed");
    }
}

// If an object calls animate while it already has an animation, its current animation in the map will be concluded
// and the new animation will start.
void TL::animate(TLAnimID id, float& object, float target, float duration, TLInterp interp)
{
    if (is_animating(id))
    {
        std::cout << "Warning: id currently has an animation. Each id is allowed only one animation at a time. The existing animation will be replaced by the new." << "\n";
        animations[id].conclude();
    }
    animations.insert(std::pair(id, TLAnimation(object, target, duration, get_interp_fn(interp))));
}

void TL::animations_step(float delta)
{
    for (auto& pair : animations)
    {
        // curr_anim_finished is set to true in TLAnimation::step if the animation finishes, otherwise false
        if (!pair.second.step(delta))
        {
            finished_ids.push_back(pair.first);
        }
    }

    for (TLAnimID id : finished_ids)
    {
        animations.erase(id);
    }
    finished_ids.clear();
}

bool TL::is_animating(TLAnimID id)
{
    return animations.find(id) != animations.end();
}

// Can lead to undefined behavior if id has no animation.
// Check if animating prior to calling reverse.
void TL::reverse(TLAnimID id)
{
    animations[id].reverse();
}

void TL::extend_target(TLAnimID id, float new_target)
{
    animations[id].extend_target(new_target);
}
