# why
use std.unix;

var task = func() {
    var res = -1;
    while (res != 0) {
        res = system("git push");
        if (res != 0) {
            unix.sleep(1);
            println("[", os.time(), "] push failed, retrying...");
        }
    }
    println("[", os.time(), "] git push successfully.");
}

task();