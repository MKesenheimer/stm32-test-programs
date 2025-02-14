define measurestep
    python import time
    python start_time = time.time()
    step
    python print("Call took {:.4f} ms".format(time.time() - start_time))
end
define ms
    measurestep
end
document measurestep
    Time execution of next step
    Usage: measurestep (or ms)
end

define measurebreak
    python import time
    python start_time = time.time()
    continue
    python print("Call took {:.4f} ms".format(time.time() - start_time))
end
define mb
    measurebreak
end
document measurebreak
    Time execution until next breakpoint
    Usage: measurebreak (or mb)
end


target extended-remote localhost:2331
b main
b Reset_Handler
mon reset
