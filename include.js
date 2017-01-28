const
    magic = require('bindings')('memwatch'),
    events = require('events');

module.exports = new events.EventEmitter();

module.exports.gc = magic.gc;
module.exports.HeapDiff = magic.HeapDiff;
module.exports.set_consecutive_growth_limit = magic.set_consecutive_growth_limit;
module.exports.set_recent_period = magic.set_recent_period;
module.exports.set_ancient_period = magic.set_ancient_period;
module.exports.writeSnapshot = function () {
    return magic.writeSnapshot.apply(null, arguments);
};

magic.upon_gc(function (has_listeners, event, data) {
    if (has_listeners) {
        return (module.exports.listeners('stats').length > 0);
    } else {
        return module.exports.emit(event, data);
    }
});

var kForkFlag = magic.kForkFlag;
var kSignalFlag = magic.kSignalFlag;

var flags = kSignalFlag;
var options = (process.env.NODE_HEAPDUMP_OPTIONS || '').split(/\s*,\s*/);
for (var i = 0, n = options.length; i < n; i += 1) {
    var option = options[i];
    if (option === '') continue;
    else if (option === 'fork') flags |= kForkFlag;
    else if (option === 'signal') flags |= kSignalFlag;
    else if (option === 'nofork') flags &= ~kForkFlag;
    else if (option === 'nosignal') flags &= ~kSignalFlag;
    else console.error('memwatch-ng: unrecognized option:', option);
}
magic.configure(flags);
