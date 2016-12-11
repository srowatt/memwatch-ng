`memwatch-ng`: Leak Detection and Heap Diffing for Node.JS
============================================================

[![Build Status](https://travis-ci.org/srowatt/memwatch-ng.svg?branch=master)](https://travis-ci.org/srowatt/memwatch-ng)

`memwatch-ng` is here to help you detect and find memory leaks in Node.JS code. `memwatch-ng` is based on the original `node-memwatch` project but adds some additional features to give you more control over how a leak is detected. 

It provides:

- A `leak` event, emitted when it appears your code is leaking memory.

- A `stats` event, emitted occasionally, giving you
  data describing your heap usage and trends over time.

- A `HeapDiff` class that lets you compare the state of your heap between
  two points in time, telling you what has been allocated, and what
  has been released.

`memwatch-ng` has been tested on Node.js versions: `0.10`, `0.12`, `1.x`, `2.x`, `3.x`, `4.x`, `5.x`, `6.x` and `7.x`.  


Installation
------------

- `npm install memwatch-ng`

or

- `git clone git://github.com/srowatt/memwatch-ng.git`


Description
-----------

There are a growing number of tools for debugging and profiling memory
usage in Node.JS applications, but there is still a need for a
platform-independent native module that requires no special
instrumentation.  This module attempts to satisfy that need.

To get started, import `memwatch-ng` like so:

```javascript
var memwatch = require('memwatch-ng');
```

### Leak Detection

You can then subscribe to `leak` events.  A `leak` event will be
emitted when your heap usage has increased for five consecutive
garbage collections:

```javascript
memwatch.on('leak', function(info) { ... });
```

The `info` object will look something like:

```javascript
{ start: Fri, 29 Jun 2012 14:12:13 GMT,
  end: Fri, 29 Jun 2012 14:12:33 GMT,
  growth: 67984,
  reason: 'heap growth over 5 consecutive GCs (20s) - 11.67 mb/hr' }
```

### Leak Detection Settings

By default, leaks are defined as 5 consecutive calls to the garbage collector 
that result in an increase of memory. To change the number of consecutive 
increases of memory limit, call 

```javascript
memwatch.set_consecutive_growth_limit(10);
```

Two different algorithms are used to determine leak detections. One is used for initial startup when memory is being allocated for loading modules and the other is for when Node.js is in a standard running state where memory allocations are less frequent. The `set_recent_period` method is used to set how many garbage compactions are triggered on startup before switching from the first leak detection algorithm to the second. 

```javascript
memwatch.set_recent_period(10);
```

The algorithm to detect leaks uses a moving window over a period of garbage compactions with a decaying weight applied to older garbage compactions. To set the moving window period, call the `set_ancient_period` method to set the number of garbage compactions used in the moving window:

```javascript
memwatch.set_ancient_period(120);
```

### Heap Usage

The best way to evaluate your memory footprint is to look at heap
usage right aver V8 performs garbage collection.  `memwatch` does
exactly this - it checks heap usage only after GC to give you a stable
baseline of your actual memory usage.

When V8 performs a garbage collection (technically, we're talking
about a full GC with heap compaction), `memwatch` will emit a `stats`
event.

```javascript
memwatch.on('stats', function(stats) { ... });
```

The `stats` data will look something like this:

```javascript
{
  "num_full_gc": 17,
  "num_inc_gc": 8,
  "heap_compactions": 8,
  "estimated_base": 2592568,
  "current_base": 2592568,
  "min": 2499912,
  "max": 2592568,
  "usage_trend": 0
}
```

`estimated_base` and `usage_trend` are tracked over time.  If usage
trend is consistently positive, it indicates that your base heap size
is continuously growing and you might have a leak.

V8 has its own idea of when it's best to perform a GC, and under a
heavy load, it may defer this action for some time.  To aid in
speedier debugging, `memwatch` provides a `gc()` method to force V8 to
do a full GC and heap compaction.


### Heap Diffing

So far we have seen how `memwatch` can aid in leak detection.  For
leak isolation, it provides a `HeapDiff` class that takes two snapshots
and computes a diff between them.  For example:

```javascript
// Take first snapshot
var hd = new memwatch.HeapDiff();

// do some things ...

// Take the second snapshot and compute the diff
var diff = hd.end();
```

The contents of `diff` will look something like:

```javascript
{
  "before": { "nodes": 11625, "size_bytes": 1869904, "size": "1.78 mb" },
  "after":  { "nodes": 21435, "size_bytes": 2119136, "size": "2.02 mb" },
  "change": { "size_bytes": 249232, "size": "243.39 kb", "freed_nodes": 197,
    "allocated_nodes": 10007,
    "details": [
      { "what": "String",
        "size_bytes": -2120,  "size": "-2.07 kb",  "+": 3,    "-": 62
      },
      { "what": "Array",
        "size_bytes": 66687,  "size": "65.13 kb",  "+": 4,    "-": 78
      },
      { "what": "LeakingClass",
        "size_bytes": 239952, "size": "234.33 kb", "+": 9998, "-": 0
      }
    ]
  }
}
```

The diff shows that during the sample period, the total number of
allocated `String` and `Array` classes decreased, but `Leaking Class`
grew by 9998 allocations.  Hmmm.

You can use `HeapDiff` in your `on('stats')` callback; even though it
takes a memory snapshot, which triggers a V8 GC, it will not trigger
the `stats` event itself.  Because that would be silly.


Future Work
-----------

Please see the Issues to share suggestions and contribute!


License
-------

http://wtfpl.net
