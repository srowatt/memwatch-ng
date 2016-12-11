/*
 * 2012|lloyd|http://wtfpl.org
 */

#ifndef __MEMWATCH_HH
#define __MEMWATCH_HH

#include <node.h>
#include <nan.h>

namespace memwatch
{
    NAN_METHOD(upon_gc);
    NAN_METHOD(trigger_gc);
    NAN_METHOD(set_consecutive_growth_limit);
    NAN_METHOD(set_recent_period);
    NAN_METHOD(set_ancient_period);
    void after_gc(v8::GCType type, v8::GCCallbackFlags flags);
};

#endif
